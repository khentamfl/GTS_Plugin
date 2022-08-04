#include "data/transient.h"
#include "data/runtime.h"
#include "node.h"
#include "util.h"
#include "scale/modscale.h"

using namespace SKSE;
using namespace RE;


namespace Gts {
	Transient& Transient::GetSingleton() noexcept {
		static Transient instance;
		return instance;
	}

	TempActorData* Transient::GetData(TESObjectREFR* object) {
		if (!object) {
			return nullptr;
		}
		auto key = object->formID;
		TempActorData* result = nullptr;
		try {
			result = &this->_actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			return nullptr;
		}
		return result;
	}

	TempActorData* Transient::GetActorData(Actor* actor) {
		std::unique_lock lock(this->_lock);
		if (!actor) {
			return nullptr;
		}
		auto key = actor->formID;
		try {
			auto no_discard = this->_actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			// Try to add
			if (!actor) {
				return nullptr;
			}
			if (!actor->Is3DLoaded()) {
				return nullptr;
			}
			TempActorData result;
			auto bound = get_bound(actor);
			if (!bound) {
				return nullptr;
			}
			auto scale = get_scale(actor);
			if (scale < 0.0) {
				return nullptr;
			}
			float base_height_unit = bound->extents[2] * scale;
			float base_height_meters = unit_to_meter(base_height_unit);

			// Volume scales cubically
			float base_volume = bound->extents[0] * bound->extents[1] * bound->extents[2] * scale * scale * scale;
			float base_volume_meters = unit_to_meter(base_volume);

			result.base_height = base_height_meters;
			result.base_volume = base_volume_meters;
			result.last_hh_adjustment = 0.0;
			result.total_hh_adjustment = 0.0;
			result.base_walkspeedmult = actor->GetActorValue(ActorValue::kSpeedMult);
			auto shoe = actor->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kFeet);
			float shoe_weight = 1.0;
			if (shoe) {
				shoe_weight = shoe->weight;
			}
			result.shoe_weight = shoe_weight;
			result.char_weight = actor->GetWeight();

			result.is_teammate = actor->formID != 0x14 && actor->IsPlayerTeammate();

			auto hhBonusPerk = Runtime::GetSingleton().hhBonus;
			if (hhBonusPerk) {
				result.has_hhBonus_perk = actor->HasPerk(hhBonusPerk);
				if (!result.has_hhBonus_perk && result.is_teammate) {
					auto player_data = this->GetData(PlayerCharacter::GetSingleton());
					if (player_data) {
						result.has_hhBonus_perk = player_data->has_hhBonus_perk;
					}
				}
			} else {
				result.has_hhBonus_perk = false;
			}

			this->_actor_data.try_emplace(key, result);
		}
		return &this->_actor_data[key];
	}

	void Transient::UpdateActorData(Actor* actor) {
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}

		auto key = actor->formID;
		std::unique_lock lock(this->_lock);
		try {
			auto data = this->_actor_data.at(key);
			auto shoe = actor->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kFeet);
			float shoe_weight = 1.0;
			if (shoe) {
				shoe_weight = shoe->weight;
			}
			data.shoe_weight = shoe_weight;

			data.char_weight = actor->GetWeight();

			data.is_teammate = actor->formID != 0x14 && actor->IsPlayerTeammate();

			auto hhBonusPerk = Runtime::GetSingleton().hhBonus;
			if (hhBonusPerk) {
				data.has_hhBonus_perk = actor->HasPerk(hhBonusPerk);
				if (!data.has_hhBonus_perk && data.is_teammate) {
					auto player_data = this->GetData(PlayerCharacter::GetSingleton());
					if (player_data) {
						data.has_hhBonus_perk = player_data->has_hhBonus_perk;
					}
				}
			} else {
				data.has_hhBonus_perk = false;
			}


		} catch (const std::out_of_range& oor) {
			return;
		}
	}

	void Transient::Clear() {
		std::unique_lock lock(this->_lock);
		this->_actor_data.clear();
	}
}
