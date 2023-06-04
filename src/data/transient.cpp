#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "node.hpp"
#include "scale/modscale.hpp"

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
			float fall_start = actor->GetPosition()[2];
			float last_set_fall_start = fall_start;
			float carryweight_boost = 0.0;
			float health_boost = 0.0;
			float basehp = 0.0;

			bool can_do_vore = true;
			bool can_be_crushed = true;
			bool dragon_was_eaten = false;
			bool can_be_vored = true;
			bool being_held = false;

			// Volume scales cubically
			float base_volume = bound->extents[0] * bound->extents[1] * bound->extents[2] * scale * scale * scale;
			float base_volume_meters = unit_to_meter(base_volume);

			result.base_height = base_height_meters;
			result.base_volume = base_volume_meters;

			result.base_walkspeedmult = actor->AsActorValueOwner()->GetBaseActorValue(ActorValue::kSpeedMult);
			auto shoe = actor->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kFeet);
			float shoe_weight = 1.0;
			if (shoe) {
				shoe_weight = shoe->weight;
			}
			result.shoe_weight = shoe_weight;
			result.char_weight = actor->GetWeight();
			result.fall_start = fall_start;
			result.last_set_fall_start = last_set_fall_start;
			result.carryweight_boost = carryweight_boost;
			result.health_boost = health_boost;
			result.basehp = basehp;
			result.can_do_vore = can_do_vore;
			result.can_be_crushed = can_be_crushed;
			result.being_held = being_held;
			result.dragon_was_eaten = dragon_was_eaten;
			result.can_be_vored = can_be_vored;

			result.is_teammate = actor->formID != 0x14 && actor->IsPlayerTeammate();

			this->_actor_data.try_emplace(key, result);
		}
		return &this->_actor_data[key];
	}


	std::string Transient::DebugName() {
		return "Transient";
	}

	void Transient::Update() {
		for (auto actor: find_actors()) {
			if (!actor) {
				continue;
			}
			if (!actor->Is3DLoaded()) {
				continue;
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


			} catch (const std::out_of_range& oor) {
				continue;
			}
		}
	}
	void Transient::Reset() {
		std::unique_lock lock(this->_lock);
		this->_actor_data.clear();
	}
	void Transient::ResetActor(Actor* actor) {
		std::unique_lock lock(this->_lock);
		if (actor) {
			auto key = actor->formID;
			this->_actor_data.erase(key);
		}
	}
}
