#include "managers/animation/AnimationManager.hpp"
#include "managers/gamemode/GameModeManager.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "scale/scalespellmanager.hpp"
#include "managers/InputManager.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/hitmanager.hpp"
#include "managers/highheel.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "utils/debug.hpp"
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
		Profilers::Start("Manager: Fade Fix");
		if (get_visual_scale(actor) < 1.5) {
			return;
		}
		if ((actor->formID == 0x14 ||actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction"))) {
			auto node = find_node(actor, "skeleton_female.nif");
			NiAVObject* skeleton = node;
			if (node) {
				BSFadeNode* fadenode = node->AsFadeNode();
				if (fadenode) {
					fadenode->GetRuntimeData().currentFade = 1.0;
				}
			}
		}
		Profilers::Stop("Manager: Fade Fix");
	}

	void ProcessExperiment(Actor* actor) {
		const std::vector<std::string_view> SpineNodes = {
			"NPC Spine [Spn0]",
			"NPC Spine [Spn1]",
			"NPC Spine [Spn2]",
			"CME UBody [UBody]",
		};
		for (auto& node: SpineNodes) {
			auto spine = find_node(actor, node);
			if (spine) {
				log::info("Node {} of {} rotation is x{} y{} z{}", node, actor->GetDisplayFullName(), spine->local.rotate{0}, spine->local.rotate{1}, spine->local.rotate{2});
			}
		}
		auto Combat = actor->GetActorRuntimeData().combatController;
		auto aiProc = actor->GetActorRuntimeData().currentProcess;
		auto high = aiProc->high;
		log::info("Water offset of {} is: {}", actor->GetDisplayFullName(), Vector2Str(high->locationOffsetByWaterPoint));
		log::info("Animation Angle of {} is {}", actor->GetDisplayFullName(), Vector2Str(high->animationAngleMod));
		high->locationOffsetByWaterPoint.z = 5 * get_visual_scale(actor);
		if (Combat) {
			auto CombatTarget = Combat->targetHandle.get().get();
			if (CombatTarget) {
				NiPoint3 Location = CombatTarget->GetPosition();
				log::info("Original Location of {} is: {}", CombatTarget->GetDisplayFullName(), Vector2Str(Location));
				Location.z -= 2400 * get_visual_scale(actor);
				log::info("Altered Location of {} is: {}", CombatTarget->GetDisplayFullName(), Vector2Str(Location));
				aiProc->SetHeadtrackTarget(actor, Location);
				log::info("Combat target of {} is {}", actor->GetDisplayFullName(), CombatTarget->GetDisplayFullName());
			}
		}
	}

	void update_height(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data) {
		Profilers::Start("Manager: update_height");
		if (!actor) {
			return;
		} if (!trans_actor_data) {
			return;
		} if (!persi_actor_data) {
			return;
		}
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
		Profilers::Stop("Manager: update_height");
	}
	void apply_height(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data, bool force = false) {
		Profilers::Start("Manager: apply_height");
		if (!actor) {
			return;
		} if (!actor->Is3DLoaded()) {
			return;
		} if (!trans_actor_data) {
			return;
		} if (!persi_actor_data) {
			return;
		}
		float scale = get_natural_scale(actor);//get_scale(actor);
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
		Profilers::Stop("Manager: apply_height");
	}

	void apply_speed(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data, bool force = false) {
		Profilers::Start("Manager: apply_speed");
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
		Profilers::Stop("Manager: apply_speed");
	}

	void update_effective_multi(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data) {
		Profilers::Start("Manager: update_effective_multi");
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
		Profilers::Stop("Manager: update_effective_multi");
	}

	void update_actor(Actor* actor) {
		Profilers::Start("Manager: update_actor");
		auto temp_data = Transient::GetSingleton().GetActorData(actor);
		auto saved_data = Persistent::GetSingleton().GetActorData(actor);
		update_effective_multi(actor, saved_data, temp_data);
		update_height(actor, saved_data, temp_data);
		Profilers::Stop("Manager: update_actor");
	}

	void apply_actor(Actor* actor, bool force = false) {
		Profilers::Start("Manager: apply_actor");
		//log::info("Apply_Actor name is {}", actor->GetDisplayFullName());
		auto temp_data = Transient::GetSingleton().GetData(actor);
		auto saved_data = Persistent::GetSingleton().GetData(actor);
		apply_height(actor, saved_data, temp_data, force);
		apply_speed(actor, saved_data, temp_data, force);
		Profilers::Stop("Manager: apply_actor");
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

// Poll for updates
void GtsManager::Update() {
	Profilers::Start("Manager: Update()");
	for (auto actor: find_actors()) {
		if (!actor) {
			return;
		}

		FixActorFade(actor); 

		auto& accuratedamage = AccurateDamage::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();

		
		if (actor->formID == 0x14 || actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction")) {
			if (sizemanager.GetPreciseDamage()) {
				accuratedamage.DoAccurateCollision(actor, 1.0, 1.0, 1000, 1.0);
				ClothManager::GetSingleton().CheckRip();
			}
			ProcessExperiment(actor);
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
	Profilers::Stop("Manager: Update()");
}

void GtsManager::OnAddPerk(const AddPerkEvent& evt) {
	if (evt.actor->formID == 0x14) {
		if (evt.perk == Runtime::GetPerk("TrueGiantess")) {
			CallHelpMessage();
		} if (evt.perk == Runtime::GetPerk("FastShrink") && !Runtime::HasSpell(evt.actor, "ShrinkBolt")) {
			Runtime::AddSpell(evt.actor, "ShrinkBolt");
		} if (evt.perk == Runtime::GetPerk("LethalShrink") && !Runtime::HasSpell(evt.actor, "ShrinkStorm")) {
			Runtime::AddSpell(evt.actor, "ShrinkStorm");
		}
	}
}

void GtsManager::reapply(bool force) {
	// Get everyone in loaded AI data and reapply
	Profilers::Start("Manager: reapply");
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
	Profilers::Stop("Manager: reapply");
}
void GtsManager::reapply_actor(Actor* actor, bool force) {
	Profilers::Start("Manager: reapply_actor");
	// Reapply just this actor
	if (!actor) {
		return;
	}
	if (!actor->Is3DLoaded()) {
		return;
	}
	apply_actor(actor, force);
	Profilers::Stop("Manager: reapply_actor");
}
