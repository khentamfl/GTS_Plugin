#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/gamemode/GameModeManager.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/AccurateDamage.hpp"
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
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "utils/debug.hpp"
#include "scale/modscale.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
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

	void ManageTalkPerk(Actor* giant) {
		static Timer TriggerDelay = Timer(0.5);
		if (giant->formID == 0x14) {
			if (!TriggerDelay.ShouldRunFrame()) {
				return;
			}
			if (Runtime::HasPerk(giant, "TalkToActor")) {
				log::info("TalkToActor Perk True");
				auto perk = Runtime::GetPerk("TalkToActor");
				if (perk) {
					log::info("Perk found");
					auto value = perk->perkConditions.head->data;
					if (data) {
						auto result = value.comparisonValue;
						if (result) {
							auto finalvalue = result.f;
							log::info("Perk Value: {}", finalvalue);
						}
					}
				}	
 			}
		}
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

		float scaleOverride = persi_actor_data->scaleOverride;
		if (scaleOverride >= 1e-4) {
			visual_scale = scaleOverride;
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

		ManageTalkPerk(actor);

		auto& accuratedamage = AccurateDamage::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();

		if (actor->formID == 0x14 || IsTeammate(actor)) {
			accuratedamage.DoAccurateCollisionLeft(actor, 0.4 * TimeScale(), 1.0, 2000, 0.05, 3.0, DamageSource::CrushedLeft);
			accuratedamage.DoAccurateCollisionRight(actor, 0.4 * TimeScale(), 1.0, 2000, 0.05, 3.0, DamageSource::CrushedRight);
			ClothManager::GetSingleton().CheckRip();

			if (IsCrawling(actor)) {
				ApplyAllCrawlingDamage(actor, 1.0, 1000, 0.25);
			}

			GameModeManager::GetSingleton().GameMode(actor); // Handle Game Modes
		}
		if (Runtime::GetBool("PreciseDamageOthers")) {
			if (actor->formID != 0x14 && !actor->IsPlayerTeammate() && !Runtime::InFaction(actor, "FollowerFaction")) {
				accuratedamage.DoAccurateCollisionLeft(actor, 0.4 * TimeScale(), 1.0, 1000, 0.25, 3.0, DamageSource::CrushedLeft);
				accuratedamage.DoAccurateCollisionRight(actor, 0.4 * TimeScale(), 1.0, 1000, 0.25, 3.0, DamageSource::CrushedRight);
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
		if (evt.perk == Runtime::GetPerk("TrueGiantess")) {
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
