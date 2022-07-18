#include "transient.h"
#include "node.h"
#include "util.h"
#include "scale.h"

using namespace SKSE;
using namespace RE;


namespace Gts {
	Transient& Transient::GetSingleton() noexcept {
		static Transient instance;
		return instance;
	}

	TempActorData* Transient::GetActorData(Actor* actor) {
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
			result.base_walkspeedmult = actor->GetActorValue(ActorValue::kSpeedMult);
			this->_actor_data[key] = result;
		}
		return &this->_actor_data[key];
	}

	void Transient::Clear() {
		std::unique_lock lock(this->_lock);
		this->_actor_data.clear();
	}
}
