
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/gamemode/GameModeManager.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/cameras/camutil.hpp"
#include "managers/ai/headtracking.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/hitmanager.hpp"
#include "utils/DynamicScale.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "scale/modscale.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "utils/camera.hpp"
#include "utils/debug.hpp"
#include "UI/DebugAPI.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "rays/raycast.hpp"
#include "Config.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace Gts {
	float GetCeilingHeight(Actor* giant) {
		if (!giant) {
			return std::numeric_limits<float>::infinity();
		}

		auto charCont = giant->GetCharController();
		if (!charCont) {
			return std::numeric_limits<float>::infinity();
		}
		auto root_node = giant->GetCurrent3D();
		if (!root_node) {
			return std::numeric_limits<float>::infinity();
		}
		float scale = get_visual_scale(giant);
		// === Calculation of ray directions ===
		auto transform = root_node->world;
		transform.scale = 1.0;
   	// ray 1 center on giant + 70
		auto ray1_p = NiPoint3(0.0, 0.0, 70.0); // in local space
		ray1_p = transform * ray1_p; // in global space
		// straight up
		auto ray1_d = NiPoint3(0.0, 0.0, 1.0); // direction

    // ray 2 slightly in front of the player
		auto ray2_p = NiPoint3(0.0, 25.0 * scale, 70.0);
		ray2_p = transform * ray2_p;
		// straight up
		auto ray2_d = NiPoint3(0.0, 0.0, 1.0);


    // Math for tilting forwards
    //     o
    //  _______
    //  |    /
    // a|   /
    //  |  /
    //  |A/
    //  |/
    //
    //  tan(A)*a=o
    //
    //  If z = a = 1.0
    //  then y = o = tan(A)*z
    //
    //  Convert from degrees to radian with radians=degrees*3.141/180.0
    //
    // At ray2 but tilting forward a bit
    float tilt_degrees = 5.0;
    auto ray3_p = ray2_p;
		auto ray3_d = transform.rotate * NiPoint3(0.0, tan(tilt_degrees*3.141/180.0)*1.0, 1.0);
		ray3_d.Unitize();

    // At ray2 but tilting backwards a bit
    auto ray4_p = ray2_p;
		auto ray4_d = transform.rotate * NiPoint3(0.0, tan(-tilt_degrees*3.141/180.0)*1.0, 1.0);
		ray4_d.Unitize();

		// List of ray positions and directions for the ceiling
		// Don't add a down here, down is made automatically as -dir
		std::vector<std::pair<NiPoint3, NiPoint3> > rays = {
			{ray1_p, ray1_d},
			{ray2_p, ray2_d},
			{ray3_p, ray3_d},
			{ray4_p, ray4_d}
		};

    if (scale > 3.0) {
      int sides = 6;
      float degrees = 380.0 / sides;
      float rads = degrees * 3.141 / 180.0;
      for (int i=0; i<sides;  i++) {
        auto mat = NiMatrix3(0.0, 0.0, rads * i);
        auto vert = mat * NiPoint3(0.0, 12.0, 0.0);
        vert = transform.rotate * (vert * scale);
        vert = ray2_p + vert;
        rays.push_back(
          {
            vert,
            NiPoint3(0.0, 0.0, 1.0)
          }
        );

      }
    }

    if (scale > 9.0) {
      int sides = 6;
      float degrees = 380.0 / sides;
      float rads = degrees * 3.141 / 180.0;
      for (int i=0; i<sides;  i++) {
        auto mat = NiMatrix3(0.0, 0.0, rads * i);
        auto vert = mat * NiPoint3(0.0, 4.0, 0.0);
        vert = transform.rotate * (vert * scale);
        vert = ray2_p + vert;
        rays.push_back(
          {
            vert,
            NiPoint3(0.0, 0.0, 1.0)
          }
        );
      }
    }

		const float RAY_LENGTH = 720;
		bool debug = IsDebugEnabled();

		// Ceiling
		std::vector<float>  ceiling_heights = {};
		log::info("Casting ceiling rays");
		for (const auto& ray: rays) {
			NiPoint3 ray_start = ray.first;
			NiPoint3 ray_dir = ray.second;
			if (debug) {
				NiPoint3 ray_end = ray_start + ray_dir*RAY_LENGTH;
				DebugAPI::DrawSphere(glm::vec3(ray_start.x, ray_start.y, ray_start.z), 8.0, 10, {0.0, 1.0, 0.0, 1.0});
				DebugAPI::DrawLineForMS(glm::vec3(ray_start.x, ray_start.y, ray_start.z), glm::vec3(ray_end.x, ray_end.y, ray_end.z), 10, {1.0, 0.0, 0.0, 1.0});
			}
			bool success = false;
			NiPoint3 endpos_up = CastRayStatics(giant, ray_start, ray_dir, RAY_LENGTH, success);
			if (success) {
				if (debug) {
					DebugAPI::DrawSphere(glm::vec3(endpos_up.x, endpos_up.y, endpos_up.z), 5.0, 30, {1.0, 0.0, 0.0, 1.0});
				}
				ceiling_heights.push_back(endpos_up.z);
			}
		}

		if (ceiling_heights.empty()) {
			return std::numeric_limits<float>::infinity();
		}
		float ceiling = *std::min_element(ceiling_heights.begin(), ceiling_heights.end());

		// Floor
		log::info("Casting floor rays");
		std::vector<float>  floor_heights = {};
		for (const auto& ray: rays) {
			NiPoint3 ray_start = ray.first;
			NiPoint3 ray_dir = ray.second * -1.0;
			if (debug) {
				NiPoint3 ray_end = ray_start + ray_dir*RAY_LENGTH;
				DebugAPI::DrawSphere(glm::vec3(ray_start.x, ray_start.y, ray_start.z), 8.0, 10, {0.0, 1.0, 1.0, 1.0});
				DebugAPI::DrawLineForMS(glm::vec3(ray_start.x, ray_start.y, ray_start.z), glm::vec3(ray_end.x, ray_end.y, ray_end.z), 10, {1.0, 0.0, 1.0, 1.0});
			}
			bool success = false;
			NiPoint3 endpos_up = CastRayStatics(giant, ray_start, ray_dir, RAY_LENGTH, success);
			if (success) {
				if (debug) {
					DebugAPI::DrawSphere(glm::vec3(endpos_up.x, endpos_up.y, endpos_up.z), 5.0, 30, {1.0, 0.0, 1.0, 1.0});
				}
				floor_heights.push_back(endpos_up.z);
			}
		}

		if (floor_heights.empty()) {
			return std::numeric_limits<float>::infinity();
		}
		float floor = *std::max_element(floor_heights.begin(), floor_heights.end());

		// Room height
		float room_height = fabs(ceiling - floor);
		float room_height_m = unit_to_meter(room_height);

		return room_height_m;
	}

	float GetMaxRoomScale(Actor* giant) {
		float stateScale = GetRaycastStateScale(giant);

		float room_height_m = GetCeilingHeight(giant);
    if (giant->formID == 0x14) {
      log::info("room_height_m (pre spring): {}", room_height_m);
    }

    // Spring
    auto& dynamicData = DynamicScale::GetData(giant);
    if (giant->formID == 0x14) {
      log::info(
        "Spring State: taget: {}, value: {}, velocity: {:.16f}, hl: {}",
        dynamicData.roomHeight.target,
        dynamicData.roomHeight.value,
        dynamicData.roomHeight.velocity,
        dynamicData.roomHeight.halflife
      );
    }
    if (!std::isinf(room_height_m)) {
      // Under roof
      if (std::isinf(dynamicData.roomHeight.target)) {
        // Last check was infinity so we just went under a roof
        // Snap current value to new roof
        if (giant->formID == 0x14) {
          log::info("Entered roof");
        }
        dynamicData.roomHeight.value = room_height_m;
        dynamicData.roomHeight.velocity = 0.0;
      }

      dynamicData.roomHeight.target = room_height_m;
      room_height_m = dynamicData.roomHeight.value;
    } else {
      // No roof, set roomHeight to infinity so we know that we left the roof
      // then continue as normal
      if (!std::isinf(dynamicData.roomHeight.target)) {
        if (giant->formID == 0x14) {
          log::info("Left roof");
        }
        dynamicData.roomHeight.target = room_height_m;
        dynamicData.roomHeight.value = room_height_m;
        dynamicData.roomHeight.velocity = 0.0;
      }
    }

    if (giant->formID == 0x14) {
      log::info("room_height_m (post spring): {}", room_height_m);
    }

		float room_height_s = room_height_m/1.82; // / height by 1.82 (default character height)
		float max_scale = (room_height_s * 0.82) / stateScale; // Define max scale, make avalibale space seem bigger when prone etc
		if (giant->formID == 0x14) {
			log::info("State scale: {}", stateScale);
			log::info("room_height_m: {}", room_height_m);
			log::info("max_scale: {}", max_scale);
		}

		return max_scale;
	}

  DynamicScaleData::DynamicScaleData() : roomHeight(
    Spring(std::numeric_limits<float>::infinity(), 1.0)
  ) {}

	DynamicScale& DynamicScale::GetSingleton() {
    static DynamicScale instance;
		return instance;
  }
  DynamicScaleData& DynamicScale::GetData(Actor* actor) {
    if (!actor) {
      throw std::exception("DynamicScale::GetData: Actor must exist");
    }
    auto id = actor->formID;

    auto& manager = DynamicScale::GetSingleton();
    manager.data.try_emplace(id);

    try {
      return manager.data.at(id);
    } catch (const std::out_of_range& oor) {
      throw std::exception("DynamicScale::GetData: Unable to find actor data");
    }
  }
}
