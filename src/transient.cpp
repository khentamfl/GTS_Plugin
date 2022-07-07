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
		auto key = actor;
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
			float base_height_unit = bound->extents[2] * get_scale(actor);
			float base_height_meters = unit_to_meter(base_height_unit);
			result.base_height = base_height_meters;
			this->_actor_data[key] = result;
		}
		return &this->_actor_data[key];
	}
}
