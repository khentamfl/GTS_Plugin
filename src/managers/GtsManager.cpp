#include "managers/animation/AnimationManager.hpp"
#include "managers/gamemode/GameModeManager.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/ai/headtracking.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "scale/scalespellmanager.hpp"
#include "managers/InputManager.hpp"
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
	void FixActorState(Actor* giant) { // Fixes Animations for GTS Grab Actions
		int StateID;
		int GTSStateID;

		giant->GetGraphVariableInt("currentDefaultState", StateID);
		giant->GetGraphVariableInt("GTS_Def_State", GTSStateID);
		// log::info("StateID: {}, GTSStateID:{}", StateID, GTSStateID);
		if (GTSStateID != StateID) {
			// log::info("Setting Grab Int to {}", StateID);
			giant->SetGraphVariableInt("GTS_Def_State", StateID);
		}
	}

	void FixActorFade(Actor* actor) {
		auto profiler = Profilers::Profile("Manager: Fade Fix");
		if (get_visual_scale(actor) < 1.5) {
			return;
		}
		if ((actor->formID == 0x14 ||actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction"))) {
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
    float currentOtherScale = GetOtherScale(actor);
    trans_actor_data->otherScales = currentOtherScale;

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
		SoftPotential getspeed {
			.k = 0.142, // 0.125
			.n = 0.82, // 0.86
			.s = 1.90, // 1.12
			.o = 1.0,
			.a = 0.0,  //Default is 0
		};

		float speedmultcalc = soft_core(scale, getspeed); // For all other movement types
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
		if (Runtime::HasMagicEffect(actor, "SmallMassiveThreat")) {
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
    static Timer testDelay = Time(5.0);
    if (testDelay.ShouldRun()) {
		    apply_height(actor, saved_data, temp_data, force);
    }
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
	for (auto actor: find_actors()) {
		if (!actor) {
			continue;
		}
		FixActorState(actor);
	}
  set_model_scale(PlayerCharacter::GetSingleton(), 0.5);
  log::info("Set player scale to 0.5");
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
			if (sizemanager.GetPreciseDamage()) {
				accuratedamage.DoAccurateCollision(actor, 1.0, 1.0, 1000, 1.0);
				ClothManager::GetSingleton().CheckRip();
			}
			GameModeManager::GetSingleton().GameMode(actor); // Handle Game Modes
		}
		if (Runtime::GetBool("PreciseDamageOthers")) {
			if (actor->formID != 0x14 && !actor->IsPlayerTeammate() && !Runtime::InFaction(actor, "FollowerFaction")) {
				accuratedamage.DoAccurateCollision(actor, 1.0, 1.0, 1000, 1.0);
			}
		}

		float current_health_percentage = GetHealthPercentage(actor);

		update_actor(actor);
		apply_actor(actor);

		SetHealthPercentage(actor, current_health_percentage);

		static Timer timer = Timer(3.00); // Add Size-related spell once per 3 sec
		if (!SizeManager::GetSingleton().GetPreciseDamage()) {
			if (timer.ShouldRunFrame()) {
				ScaleSpellManager::GetSingleton().CheckSize(actor);
			}
		}
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
