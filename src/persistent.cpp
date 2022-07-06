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
		std::unique_lock lock(GetSingleton()._lock);
		GetSingleton()._actor_data.clear();
	}

	void Persistent::OnGameLoaded(SerializationInterface* serde) {
		std::uint32_t type;
		std::uint32_t size;
		std::uint32_t version;

		while (serde->GetNextRecordInfo(type, version, size)) {
			if (type == ActorDataRecord) {
				std::size_t size;
				serde->ReadRecordData(&size, sizeof(size));
				for (; size > 0; --size) {
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
					data.native_scale = native_scale;
					data.visual_scale = visual_scale;
					data.visual_scale_v = visual_scale_v;
					data.target_scale = target_scale;
					data.max_scale = max_scale;
					auto* actor = TESForm::LookupByID<Actor>(newActorFormID);
					if (actor) {
						GetSingleton()._actor_data.try_emplace(actor, data);
					} else {
						log::warn("Actor ID {:X} could not be found after loading the save.", newActorFormID);
					}
				}
			} else {
				log::warn("Unknown record type in cosave.");
				__assume(false);
			}
		}
	}

	void Persistent::OnGameSaved(SerializationInterface* serde) {
		std::unique_lock lock(GetSingleton()._lock);

		if (!serde->OpenRecord(ActorDataRecord, 0)) {
			log::error("Unable to open record to write cosave data.");
			return;
		}

		auto size = GetSingleton()._actor_data.size();
		serde->WriteRecordData(&size, sizeof(size));
		for (auto const& [actor, data] : GetSingleton()._actor_data) {
			auto form_id = &actor->formID;
			serde->WriteRecordData(form_id, sizeof(form_id));
			serde->WriteRecordData(&data.native_scale, sizeof(data.native_scale));
			serde->WriteRecordData(&data.visual_scale, sizeof(data.visual_scale));
			serde->WriteRecordData(&data.visual_scale_v, sizeof(data.visual_scale_v));
			serde->WriteRecordData(&data.target_scale, sizeof(data.target_scale));
			serde->WriteRecordData(&data.max_scale, sizeof(data.max_scale));
		}
	}

	ActorData* Persistent::GetActorData(Actor* actor) {
		auto key = actor;
		ActorData* result = nullptr;
		try {
			result = &this->_actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			// Add new
			ActorData new_data;
			auto scale = get_scale(actor);
			new_data.native_scale = scale;
			new_data.visual_scale = scale;
			new_data.visual_scale_v = 0.0;
			new_data.target_scale = scale;
			new_data.max_scale = scale;
			this->_actor_data[key] = new_data;
			result = &this->_actor_data.at(key);
		}
		return result;
	}
}
