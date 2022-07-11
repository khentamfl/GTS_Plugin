#include "persistent.h"
#include "GtsManager.h"
#include "scale.h"

using namespace SKSE;
using namespace RE;

namespace {
	inline const auto ActorDataRecord = _byteswap_ulong('ACTD');
	inline const auto ScaleMethodRecord = _byteswap_ulong('SCMD');
	inline const auto HighHeelCorrectionRecord = _byteswap_ulong('HHCO');
}

namespace Gts {
	Persistent& Persistent::GetSingleton() noexcept {
		static Persistent instance;
		return instance;
	}

	void Persistent::OnRevert(SerializationInterface*) {
		std::unique_lock lock(GetSingleton()._lock);
		GetSingleton()._actor_data.clear();
	}

	void Persistent::OnGameLoaded(SerializationInterface* serde) {
		std::uint32_t type;
		std::uint32_t size;
		std::uint32_t version;

		while (serde->GetNextRecordInfo(type, version, size)) {
			if (type == ActorDataRecord) {
				if (version >= 1) {
					std::size_t count;
					serde->ReadRecordData(&count, sizeof(count));
					for (; count > 0; --count) {
						RE::FormID actorFormID;
						serde->ReadRecordData(&actorFormID, sizeof(actorFormID));
						RE::FormID newActorFormID;
						if (!serde->ResolveFormID(actorFormID, newActorFormID)) {
							log::warn("Actor ID {:X} could not be found after loading the save.", actorFormID);
							continue;
						}
						float native_scale;
						serde->ReadRecordData(&native_scale, sizeof(native_scale));
						float visual_scale;
						serde->ReadRecordData(&visual_scale, sizeof(visual_scale));
						float visual_scale_v;
						serde->ReadRecordData(&visual_scale_v, sizeof(visual_scale_v));
						float target_scale;
						serde->ReadRecordData(&target_scale, sizeof(target_scale));
						float max_scale;
						serde->ReadRecordData(&max_scale, sizeof(max_scale));
						float half_life;
						if (version >= 2) {
							serde->ReadRecordData(&half_life, sizeof(half_life));
						} else {
							half_life = 0.05;
						}
						float anim_speed;
						if (version >= 3) {
							serde->ReadRecordData(&anim_speed, sizeof(anim_speed));
						} else {
							anim_speed = 1.0;
						}
						ActorData data;
						log::info("Loading Actor {:X} with data, native_scale: {}, visual_scale: {}, visual_scale_v: {}, target_scale: {}, max_scale: {}, half_life: {}, anim_speed: {}", newActorFormID, native_scale, visual_scale, visual_scale_v, target_scale, max_scale, half_life, anim_speed);
						data.native_scale = native_scale;
						data.visual_scale = visual_scale;
						data.visual_scale_v = visual_scale_v;
						data.target_scale = target_scale;
						data.max_scale = max_scale;
						data.half_life = half_life;
						data.anim_speed = anim_speed;
						TESForm* actor_form = TESForm::LookupByID<Actor>(newActorFormID);
						if (actor_form) {
							Actor* actor = skyrim_cast<Actor*>(actor_form);
							if (actor) {
								GetSingleton()._actor_data.insert_or_assign(newActorFormID, data);
							} else {
								log::warn("Actor ID {:X} could not be found after loading the save.", newActorFormID);
							}
						} else {
							log::warn("Actor ID {:X} could not be found after loading the save.", newActorFormID);
						}
					}
				} else {
					log::info("Disregarding version 0 cosave info.");
				}
			} else if (type == ScaleMethodRecord) {
				int size_method;
				serde->ReadRecordData(&size_method, sizeof(size_method));
				switch (size_method) {
					case 0:
						GetSingleton().size_method = SizeMethod::ModelScale;
						break;
					case 1:
						GetSingleton().size_method = SizeMethod::RootScale;
						break;
					case 2:
						GetSingleton().size_method = SizeMethod::RefScale;
						break;
				}
			} else if (type == HighHeelCorrectionRecord) {
				bool highheel_correction;
				serde->ReadRecordData(&highheel_correction, sizeof(highheel_correction));
				GetSingleton().highheel_correction = highheel_correction;
			} else {
				log::warn("Unknown record type in cosave.");
				__assume(false);
			}
		}
	}

	void Persistent::OnGameSaved(SerializationInterface* serde) {
		std::unique_lock lock(GetSingleton()._lock);

		if (!serde->OpenRecord(ActorDataRecord, 3)) {
			log::error("Unable to open actor data record to write cosave data.");
			return;
		}

		auto count = GetSingleton()._actor_data.size();
		serde->WriteRecordData(&count, sizeof(count));
		for (auto const& [form_id_t, data] : GetSingleton()._actor_data) {
			FormID form_id = form_id_t;
			float native_scale = data.native_scale;
			float visual_scale = data.visual_scale;
			float visual_scale_v = data.visual_scale_v;
			float target_scale = data.target_scale;
			float max_scale = data.max_scale;
			float half_life = data.half_life;
			float anim_speed = data.anim_speed;
			log::info("Saving Actor {:X} with data, native_scale: {}, visual_scale: {}, visual_scale_v: {}, target_scale: {}, max_scale: {}, half_life: {}, anim_speed: {}", form_id, native_scale, visual_scale, visual_scale_v, target_scale, max_scale, half_life, anim_speed);
			serde->WriteRecordData(&form_id, sizeof(form_id));
			serde->WriteRecordData(&native_scale, sizeof(native_scale));
			serde->WriteRecordData(&visual_scale, sizeof(visual_scale));
			serde->WriteRecordData(&visual_scale_v, sizeof(visual_scale_v));
			serde->WriteRecordData(&target_scale, sizeof(target_scale));
			serde->WriteRecordData(&max_scale, sizeof(max_scale));
			serde->WriteRecordData(&half_life, sizeof(half_life));
			serde->WriteRecordData(&anim_speed, sizeof(anim_speed));
		}

		if (!serde->OpenRecord(ScaleMethodRecord, 0)) {
			log::error("Unable to open scale method record to write cosave data.");
			return;
		}

		int size_method = GetSingleton().size_method;
		serde->WriteRecordData(&size_method, sizeof(size_method));

		if (!serde->OpenRecord(HighHeelCorrectionRecord, 0)) {
			log::error("Unable to open high heel correction record to write cosave data.");
			return;
		}

		int highheel_correction = GetSingleton().highheel_correction;
		serde->WriteRecordData(&highheel_correction, sizeof(highheel_correction));

	}

	ActorData* Persistent::GetActorData(Actor* actor) {
		if (!actor) {
			return nullptr;
		}
		auto key = actor->formID;
		ActorData* result = nullptr;
		try {
			result = &this->_actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			// Add new
			if (!actor) {
				return nullptr;
			}
			if (!actor->Is3DLoaded()) {
				return nullptr;
			}
			ActorData new_data;
			auto scale = get_scale(actor);
			if (scale < 0.0) {
				return nullptr;
			}
			new_data.native_scale = scale;
			new_data.visual_scale = scale;
			new_data.visual_scale_v = 0.0;
			new_data.target_scale = scale;
			new_data.max_scale = 65535.0;
			new_data.half_life = 0.05;
			new_data.anim_speed = 1.0;
			this->_actor_data[key] = new_data;
			result = &this->_actor_data.at(key);
		}
		return result;
	}
}
