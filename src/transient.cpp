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
		log::info("+ Transient::GetActorData");
		if (!actor) {
			log::info("- Transient::GetActorData == No Actor");
			return nullptr;
		}
		log::info("  - key = actor");
		auto key = actor;
		try {
			log::info("  - grab current key");
			auto no_discard = this->_actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			// Try to add
			log::info("  - is actor valid");
			if (!actor) {
				log::info("- Transient::GetActorData == No Actor");
				return nullptr;
			}
			log::info("  - is 3d loaded");
			if (!actor->Is3DLoaded()) {
				log::info("- Transient::GetActorData == 3D Not loaded");
				return nullptr;
			}
			log::info("  - Making result");
			TempActorData result;
			log::info("  - getting bound");
			auto bound = get_bound(actor);
            if (!bound) {
                return nullptr;
            }
			log::info("  - getting scale");
			auto scale = get_scale(actor);
			if (scale < 0.0) {
				log::info("- Transient::GetActorData == Scale not valid");
				return nullptr;
			}
			log::info("  - Calculating bound extent");
			float base_height_unit = bound->extents[2] * scale;
			log::info("  - Converting to meters");
			float base_height_meters = unit_to_meter(base_height_unit);
			log::info("  - Applying result");
			result.base_height = base_height_meters;
			log::info("  - Setting key to value");
			this->_actor_data[key] = result;
		}
		log::info("- Transient::GetActorData");
		return &this->_actor_data[key];
	}
}
