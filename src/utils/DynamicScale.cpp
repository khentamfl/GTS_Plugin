
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
#include "raycast.hpp"
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

		// Calculation of ray directions
		auto transform = root_node->world;
		auto giantPos = giant->GetPosition() + NiPoint3(0.0, 0.0, 70.0);
		auto bumperPos = transform * charCont->bumperCollisionBound.center;
		bumperPos.z = 70.0;

		auto bumperDirForwardUp = (
			charCont->bumperCollisionBound.center
			+ NiPoint3(0.0, 0.25 * charCont->bumperCollisionBound.extents.y, 0.0)
			+ NiPoint3(0.0, 0.0, charCont->bumperCollisionBound.extents.z)
			);
		bumperDirForwardUp = transform * bumperDirForwardUp;
		bumperDirForwardUp = bumperDirForwardUp - bumperPos;
		bumperDirForwardUp.Unitize();

		auto bumperDirBackUp = (
			charCont->bumperCollisionBound.center
			+ NiPoint3(0.0, -0.25 * charCont->bumperCollisionBound.extents.y, 0.0)
			+ NiPoint3(0.0, 0.0, charCont->bumperCollisionBound.extents.z)
			);
		bumperDirBackUp = transform * bumperDirBackUp;
		bumperDirBackUp = bumperDirBackUp - bumperPos;
		bumperDirBackUp.Unitize();

		// List of ray positions and directions for the ceiling
		// Don't add a down here, down is made automatically as -dir
		std::vector<std::pair<NiPoint3, NiPoint3> > rays = {
			{giantPos, NiPoint3(0.0, 0.0, 1.0)},
			{bumperPos, NiPoint3(0.0, 0.0, 1.0)},
			{bumperPos, bumperDirForwardUp},
			{bumperPos, bumperDirBackUp}
		};

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
				DebugAPI::DrawSphere(glm::vec3(endpos_up.x, endpos_up.y, endpos_up.z), 5.0, 30, {1.0, 0.0, 0.0, 1.0});
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
				DebugAPI::DrawSphere(glm::vec3(endpos_up.x, endpos_up.y, endpos_up.z), 5.0, 30, {1.0, 0.0, 1.0, 1.0});
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
		float room_height_s = room_height_m/1.82; // / height by 1.82 (default character height)
		float max_scale = (room_height_s * 0.82) / stateScale; // Define max scale, make avalibale space seem bigger when prone etc
		if (giant->formID == 0x14) {
			log::info("State scale: {}", stateScale);
			log::info("room_height_m: {}", room_height_m);
			log::info("max_scale: {}", max_scale);
		}

		return max_scale;
	}
}
