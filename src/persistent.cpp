#include "persistent.h"
#include "scale.h"

using namespace SKSE;
using namespace RE;

namespace {
	inline const auto ActorDataRecord = _byteswap_ulong('ACTD');
}

namespace Gts {
	Persistent& Persistent::GetSingleton() noexcept {
		static Persistent instance;
		return instance;
	}

	void Persistent::OnRevert(SerializationInterface*) {
		log::info("+ Reverting cosave");
		std::unique_lock lock(GetSingleton()._lock);
		GetSingleton()._actor_data.clear();
		log::info("- Reverting cosave");
	}

	void Persistent::OnGameLoaded(SerializationInterface* serde) {
		log::info("+ Loading cosave");
		std::uint32_t type;
		std::uint32_t size;
		std::uint32_t version;

		while (serde->GetNextRecordInfo(type, version, size)) {
			if (type == ActorDataRecord) {
				if (version == 1) {
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
						ActorData data;
						log::info("Loading Actor {:X} with data, native_scale: {}, visual_scale: {}, visual_scale_v: {}, target_scale: {}, max_scale: {}", newActorFormID, native_scale, visual_scale, visual_scale_v, target_scale, max_scale);
						data.native_scale = native_scale;
						data.visual_scale = visual_scale;
						data.visual_scale_v = visual_scale_v;
						data.target_scale = target_scale;
						data.max_scale = max_scale;
						auto* actor = TESForm::LookupByID<Actor>(newActorFormID);
						if (actor) {
							GetSingleton()._actor_data.insert_or_assign(actor, data);
						} else {
							log::warn("Actor ID {:X} could not be found after loading the save.", newActorFormID);
						}
					}
				} else {
					log::info("Disregarding version 0 cosave info.")
				}
			} else {
				log::warn("Unknown record type in cosave.");
				__assume(false);
			}
		}
		log::info("- Loading cosave");
	}

	void Persistent::OnGameSaved(SerializationInterface* serde) {
		log::info("+ Saving cosave");
		std::unique_lock lock(GetSingleton()._lock);

		if (!serde->OpenRecord(ActorDataRecord, 1)) {
			log::error("Unable to open record to write cosave data.");
			return;
		}

		auto count = GetSingleton()._actor_data.size();
		serde->WriteRecordData(&count, sizeof(count));
		for (auto const& [actor, data] : GetSingleton()._actor_data) {
			FormID form_id = actor->formID;
			float native_scale = data.native_scale;
			float visual_scale = data.visual_scale;
			float visual_scale_v = data.visual_scale_v;
			float target_scale = data.target_scale;
			float max_scale = data.max_scale;
			log::info("Saving Actor {:X} with data, native_scale: {}, visual_scale: {}, visual_scale_v: {}, target_scale: {}, max_scale: {}", form_id, native_scale, visual_scale, visual_scale_v, target_scale, max_scale);
			serde->WriteRecordData(&form_id, sizeof(form_id));
			serde->WriteRecordData(&native_scale, sizeof(native_scale));
			serde->WriteRecordData(&visual_scale, sizeof(visual_scale));
			serde->WriteRecordData(&visual_scale_v, sizeof(visual_scale_v));
			serde->WriteRecordData(&target_scale, sizeof(target_scale));
			serde->WriteRecordData(&max_scale, sizeof(max_scale));
		}
		log::info("- Saving cosave");
	}

	ActorData* Persistent::GetActorData(Actor* actor) {
		log::info("+ Persistent::GetActorData");
		auto key = actor;
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
			new_data.native_scale = scale;
			new_data.visual_scale = scale;
			new_data.visual_scale_v = 0.0;
			new_data.target_scale = scale;
			new_data.max_scale = 65535.0;
			this->_actor_data[key] = new_data;
			result = &this->_actor_data.at(key);
		}
		log::info("- Persistent::GetActorData");
		return result;
	}
}
