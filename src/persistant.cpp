#include "persistant.h"
#include "scale.h"

using namespace SKSE;
using namespace RE;

namespace {
	inline const auto ActorDataRecord = _byteswap_ulong('ACTD');
}

namespace Gts {
	Persistant& Persistant::GetSingleton() {
		static Persistant instance;
		return instance;
	}

	void Persistant::OnRevert(SerializationInterface*) {
		std::unique_lock lock(GetSingleton()._lock);
		GetSingleton()._actor_data.clear();
	}

	void Persistant::OnGameLoaded(SerializationInterface* serde) {
		std::uint32_t type;
		std::uint32_t size;
		std::uint32_t version;

		while (serde->GetNextRecordInfo(type, version, size)) {
			if (type == ActorDataRecord) {
				std::size_t size;
				serde->ReadRecordData(&hitCountsSize, sizeof(hitCountsSize));
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
					float target_scale;
					serde->ReadRecordData(&target_scale, sizeof(target_scale));
					float max_scale;
					serde->ReadRecordData(&max_scale, sizeof(max_scale));
					ActorData data;
					data.native_scale = native_scale;
					data.visual_scale = visual_scale;
					data.target_scale = target_scale;
					data.max_scale = max_scale;
					auto* actor = TESForm::LookupByID<Actor>(newActorFormID);
					if (actor) {
						GetSingleton()._actor_data.try_emplace(actor, hitCount);
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

	void Persistant::OnGameSaved(SerializationInterface* serde) {
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
			serde->WriteRecordData(&data.target_scale, sizeof(data.target_scale));
			serde->WriteRecordData(&data.max_scale, sizeof(data.max_scale));
		}
	}

	ActorData* Persistant::GetActorData(Actor* actor) {
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
			new_data.target_scale = scale;
			new_data.max_scale = scale;
			this->_actor_data[key] = new_data;
			result = &this->_actor_data.at(key);
		}
		return result;
	}
}
