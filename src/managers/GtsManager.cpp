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

namespace {
	void FixActorFade(Actor* actor) {
		auto profiler = Profilers::Profile("Manager: Fade Fix");
		if (get_visual_scale(actor) < 1.5) {
			return;
		}
		if ((actor->formID == 0x14 || IsTeammate(actor))) {
			auto node = find_node(actor, "skeleton_female.nif");
			NiAVObject* skeleton = node;
			if (node) {
				BSFadeNode* fadenode = node->AsFadeNode();
				if (fadenode) {
					if (fadenode->GetRuntimeData().currentFade < 1.0f) {
						fadenode->GetRuntimeData().currentFade = 1.0f;
					}
				}
			}
		}
	}
	/*void WaterExperiments(Actor* giant) {
		auto shaders = BSShaderManager::State& GetSingleton();
		if (shaders) {
			float intersect = shaders->waterIntersect;
			float bound = shaders->cachedPlayerBound->radius;
			NiPoint3 center = shaders->cachedPlayerBound->center;
			log::info("Intersect: {}, bound: {}, center: {}", intersect, bound, Vector2Str(center));
		}
	}*/
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
    std::vector<std::pair<NiPoint3, NiPoint3>> rays = {
      {giantPos, NiPoint3(0.0, 0.0, 1.0)},
      {bumperPos, NiPoint3(0.0, 0.0, 1.0)},
      {bumperPos, bumperDirForwardUp},
      {bumperPos, bumperDirBackUp}
    };

    const float RAY_LENGTH = 720;
    bool debug = IsDebugEnabled();

    // Ceiling
    std::vector<float>  ceiling_heights = {};
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
		float max_scale = room_height_s * 0.82;
    max_scale /= stateScale; // Make avalibale space seem bigger when prone etc
    if (giant->formID == 0x14) {
      log::info("room_height_m: {}", room_height_m);
      log::info("max_scale: {}", max_scale);
    }

    return max_scale;
  }

	void update_height(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data) {
		auto profiler = Profilers::Profile("Manager: update_height");
		if (!actor) {
			return;
		}
		if (!trans_actor_data) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		float currentOtherScale = Get_Other_Scale(actor);
		trans_actor_data->otherScales = currentOtherScale;
		//log::info("Other Scale of {} is {}", actor->GetDisplayFullName(), currentOtherScale);

		float target_scale = persi_actor_data->target_scale;

		// Smooth target_scale towards max_scale if target_scale > max_scale
		float max_scale = persi_actor_data->max_scale;
		if (target_scale > max_scale) {
			float minimum_scale_delta = 0.000005; // 0.00005%
			if (fabs(target_scale - max_scale) < minimum_scale_delta) {
				persi_actor_data->target_scale = max_scale;
				persi_actor_data->target_scale_v = 0.0;
			} else {
				critically_damped(
					persi_actor_data->target_scale,
					persi_actor_data->target_scale_v,
					max_scale,
					persi_actor_data->half_life*1.5,
					Time::WorldTimeDelta()
					);
			}
		} else {
			persi_actor_data->target_scale_v = 0.0;
		}

    // Room Size adjustments
    // We only do this if they are bigger than 1.05x their natural scale (currentOtherScale)
    // and if enabled in the mcm
    if (SizeRaycastEnabled() && target_scale > currentOtherScale * 1.05) {
      float room_scale = GetMaxRoomScale(actor);
      if (room_scale > currentOtherScale) {
        // Only apply room scale if room_scale > natural_scale
        //   This stops it from working when room_scale < 1.0
        if (actor->formID == 0x14) {
          log::info("old target_scale: {}", target_scale);
          log::info("room_scale: {}", room_scale);
        }
        target_scale = min(target_scale, room_scale);
        if (actor->formID == 0x14) {
          log::info("new target_scale: {}", target_scale);
        }
  		}
		}

		if (fabs(target_scale - persi_actor_data->visual_scale) > 1e-5) {
			float minimum_scale_delta = 0.000005; // 0.00005%
			if (fabs(target_scale - persi_actor_data->visual_scale) < minimum_scale_delta) {
				persi_actor_data->visual_scale = target_scale;
				persi_actor_data->visual_scale_v = 0.0;
			} else {
				critically_damped(
					persi_actor_data->visual_scale,
					persi_actor_data->visual_scale_v,
					target_scale,
					persi_actor_data->half_life,
					Time::WorldTimeDelta()
				);
			}
		}
	}
	void apply_height(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data, bool force = false) {
		auto profiler = Profilers::Profile("Manager: apply_height");
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}
		if (!trans_actor_data) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		float scale = get_scale(actor);
		if (scale < 0.0) {
			return;
		}
		float visual_scale = persi_actor_data->visual_scale;
		if (actor->formID == 0x14) {
			float scaleOverride = get_fp_scale(actor);
			if (IsFirstPerson() && scaleOverride >= 1e-4) {
				if (scaleOverride > 1.0) {
					visual_scale *= GetProneAdjustment(); // In normal case we * it for compatibility with crawling/proning.
				} else {
					visual_scale = scaleOverride; // In Loot/Combat mode case, we override it with flat value (such as 0.6).
				}
			}
		}


		// Is scale correct already?
		if (fabs(visual_scale - scale) <= 1e-5 && !force) {
			return;
		}
		// Is scale too small
		if (visual_scale <= 1e-5) {
			return;
		}
		//log::info("Setting {} scale to {}, visual scale: {}", actor->GetDisplayFullName(), visual_scale, vs);
		set_scale(actor, visual_scale);
	}

	void apply_speed(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data, bool force = false) {
		auto profiler = Profilers::Profile("Manager: apply_speed");
		if (!Persistent::GetSingleton().is_speed_adjusted) {
			return;
		}
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}
		if (!trans_actor_data) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		float scale = get_visual_scale(actor);
		if (scale < 1e-5) {
			return;
		}
		float speedmultcalc = GetAnimationSlowdown(actor); // For all other movement types
		float bonus = Persistent::GetSingleton().GetActorData(actor)->smt_run_speed;
		float perkspeed = 1.0;
		persi_actor_data->anim_speed = speedmultcalc*perkspeed;//MS_mult;
	}

	void update_effective_multi(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data) {
		auto profiler = Profilers::Profile("Manager: update_effective_multi");
		if (!actor) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		if (HasSMT(actor)) {
			persi_actor_data->effective_multi = 2.0;
		} else {
			persi_actor_data->effective_multi = 1.0;
		}
	}

	void update_actor(Actor* actor) {
		auto profiler = Profilers::Profile("Manager: update_actor");
		auto temp_data = Transient::GetSingleton().GetActorData(actor);
		auto saved_data = Persistent::GetSingleton().GetActorData(actor);
		update_effective_multi(actor, saved_data, temp_data);
		update_height(actor, saved_data, temp_data);
	}

	void apply_actor(Actor* actor, bool force = false) {
		auto profiler = Profilers::Profile("Manager: apply_actor");
		//log::info("Apply_Actor name is {}", actor->GetDisplayFullName());
		auto temp_data = Transient::GetSingleton().GetData(actor);
		auto saved_data = Persistent::GetSingleton().GetData(actor);
		apply_height(actor, saved_data, temp_data, force);
		apply_speed(actor, saved_data, temp_data, force);
	}
}

GtsManager& GtsManager::GetSingleton() noexcept {
	static GtsManager instance;

	static std::atomic_bool initialized;
	static std::latch latch(1);
	if (!initialized.exchange(true)) {
		latch.count_down();
	}
	latch.wait();

	return instance;
}

std::string GtsManager::DebugName() {
	return "GtsManager";
}

void GtsManager::Start() {
}

// Poll for updates
void GtsManager::Update() {
	auto profiler = Profilers::Profile("Manager: Update()");
	for (auto actor: find_actors()) {
		if (!actor) {
			return;
		}

		FixActorFade(actor);

		auto& accuratedamage = AccurateDamage::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();

		if (actor->formID == 0x14 || IsTeammate(actor)) {
			accuratedamage.DoAccurateCollisionLeft(actor, 0.4 * TimeScale(), 1.0, 4000, 0.05, 3.0, DamageSource::CrushedLeft);
			accuratedamage.DoAccurateCollisionRight(actor, 0.4 * TimeScale(), 1.0, 4000, 0.05, 3.0, DamageSource::CrushedRight);

			ClothManager::GetSingleton().CheckRip();
			SpawnActionIcon(actor);

			if (IsCrawling(actor)) {
				ApplyAllCrawlingDamage(actor, 1.0, 1000, 0.25);
			}

			GameModeManager::GetSingleton().GameMode(actor); // Handle Game Modes
		}
		if (Runtime::GetBool("PreciseDamageOthers")) {
			if (actor->formID != 0x14 && !actor->IsPlayerTeammate() && !Runtime::InFaction(actor, "FollowerFaction")) {
				accuratedamage.DoAccurateCollisionLeft(actor, 0.4 * TimeScale(), 1.0, 4000, 0.05, 3.0, DamageSource::CrushedLeft);
				accuratedamage.DoAccurateCollisionRight(actor, 0.4 * TimeScale(), 1.0, 4000, 0.05, 3.0, DamageSource::CrushedRight);
			}
		}

		float current_health_percentage = GetHealthPercentage(actor);

		update_actor(actor);
		apply_actor(actor);

		SetHealthPercentage(actor, current_health_percentage);

	}
}

void GtsManager::OnAddPerk(const AddPerkEvent& evt) {
	if (evt.actor->formID == 0x14) {
		if (evt.perk == Runtime::GetPerk("TotalControl")) {
			CallHelpMessage();
		}
		if (evt.perk == Runtime::GetPerk("FastShrink") && !Runtime::HasSpell(evt.actor, "ShrinkBolt")) {
			Runtime::AddSpell(evt.actor, "ShrinkBolt");
		}
		if (evt.perk == Runtime::GetPerk("LethalShrink") && !Runtime::HasSpell(evt.actor, "ShrinkStorm")) {
			Runtime::AddSpell(evt.actor, "ShrinkStorm");
		}
		if (evt.perk == Runtime::GetPerk("CalamityPerk")) {
			AddCalamityPerk();
		}
	}
}

void GtsManager::reapply(bool force) {
	// Get everyone in loaded AI data and reapply
	auto profiler = Profilers::Profile("Manager: reapply");
	auto actors = find_actors();
	for (auto actor: actors) {
		if (!actor) {
			continue;
		}
		if (!actor->Is3DLoaded()) {
			continue;
		}
		reapply_actor(actor, force);
	}
}
void GtsManager::reapply_actor(Actor* actor, bool force) {
	auto profiler = Profilers::Profile("Manager: reapply_actor");
	// Reapply just this actor
	if (!actor) {
		return;
	}
	if (!actor->Is3DLoaded()) {
		return;
	}
	apply_actor(actor, force);
}
