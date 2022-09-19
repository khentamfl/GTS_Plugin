#include "Config.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GtsManager.hpp"
#include "managers/highheel.hpp"
#include "managers/Attributes.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "scale/scale.hpp"
#include "util.hpp"
#include "node.hpp"
#include "timer.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	void update_height(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data) {
		if (!actor) {
			return;
		}
		//if (actor->formID==0x14) {
		//log::info("Player's VS:{}, VS_V: {}", persi_actor_data->visual_scale, persi_actor_data->visual_scale_v);
		//}
		if (!actor->Is3DLoaded()) {
			return;
		}
		if (!trans_actor_data) {
			return;
		}
		if (!persi_actor_data) {
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
	}
	void apply_height(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data, bool force = false) {
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
		float change_requirement = Runtime::GetSingleton().sizeLimit->value + persi_actor_data->bonus_max_size;

		// Is scale correct already?
		if (fabs(visual_scale - scale) <= 1e-5 && !force) {
			return;
		}

		// Is scale too small
		if (visual_scale <= 1e-5) {
			return;
		}

		// log::trace("Scale changed from {} to {}. Updating",scale, visual_scale);
		set_scale(actor, visual_scale);
	}

	void apply_speed(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data, bool force = false) {
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

		float scale = persi_actor_data->visual_scale;
		if (scale < 1e-5) {
			//log::info("!SCALE IS < 1e-5! {}", actor->GetDisplayFullName());
			return;
		}
		SoftPotential& speed_adjustment = Persistent::GetSingleton().speed_adjustment;
		SoftPotential& MS_adjustment = Persistent::GetSingleton().MS_adjustment;
		float speed_mult = soft_core(scale, speed_adjustment);
		float MS_mult = soft_core(scale, MS_adjustment);


		static Timer timer = Timer(0.50); // Run every 0.5s or as soon as we can
		if (timer.ShouldRunFrame()) {


			float Bonus = Persistent::GetSingleton().GetActorData(actor)->smt_run_speed;
			float MovementSpeed = actor->GetActorValue(ActorValue::kSpeedMult);
			persi_actor_data->anim_speed = speed_mult;
			if (actor->IsWalking() == true) {
				actor->SetActorValue(ActorValue::kSpeedMult, ((trans_actor_data->base_walkspeedmult * (Bonus/3 + 1.0))) / MS_mult);
				//log::info("Slow Walk Adjusting MS of {}, BaseWS: {}, Ms_Mult: {}, kSpeedMult: {}", actor->GetDisplayFullName(), trans_actor_data->base_walkspeedmult, MS_mult, MovementSpeed);
			}
			if (actor->IsSprinting() == true) {
				actor->SetActorValue(ActorValue::kSpeedMult, ((trans_actor_data->base_walkspeedmult * (Bonus/3 + 1.0))) * 1.25 / MS_mult);
				//log::info("Sprint Adjusting MS of {}, BaseWS: {}, Ms_Mult: {}, kSpeedMult: {}", actor->GetDisplayFullName(), trans_actor_data->base_walkspeedmult, MS_mult, MovementSpeed);
			} else {
				actor->SetActorValue(ActorValue::kSpeedMult, (trans_actor_data->base_walkspeedmult + (Bonus/3 + 1.0))/ MS_mult);
				//log::info("Normal Adjusting MS of {}, BaseWS: {}, Ms_Mult: {}, kSpeedMult: {}", actor->GetDisplayFullName(), trans_actor_data->base_walkspeedmult, MS_mult, MovementSpeed);
			}
		}



		// Experiement
		if (false) {
			auto& rot_speed = actor->currentProcess->middleHigh->rotationSpeed;
			if (fabs(rot_speed.x) > 1e-5 || fabs(rot_speed.y) > 1e-5 || fabs(rot_speed.z) > 1e-5) {
				log::info("{} rotationSpeed: {},{},{}", actor_name(actor), rot_speed.x,rot_speed.y,rot_speed.z);
				actor->currentProcess->middleHigh->rotationSpeed.x *= speed_mult;
				actor->currentProcess->middleHigh->rotationSpeed.y *= speed_mult;
				actor->currentProcess->middleHigh->rotationSpeed.z *= speed_mult;
			}
			auto& animationDelta = actor->currentProcess->high->animationDelta;
			if (fabs(animationDelta.x) > 1e-5 || fabs(animationDelta.y) > 1e-5 || fabs(animationDelta.z) > 1e-5) {
				log::info("{} animationDelta: {},{},{}", actor_name(actor), animationDelta.x,animationDelta.y,animationDelta.z);
			}
			auto& animationAngleMod = actor->currentProcess->high->animationAngleMod;
			if (fabs(animationAngleMod.x) > 1e-5 || fabs(animationAngleMod.y) > 1e-5 || fabs(animationAngleMod.z) > 1e-5) {
				log::info("{} animationAngleMod: {},{},{}", actor_name(actor), animationAngleMod.x,animationAngleMod.y,animationAngleMod.z);
			}
			auto& pathingCurrentRotationSpeed = actor->currentProcess->high->pathingCurrentRotationSpeed;
			if (fabs(pathingCurrentRotationSpeed.x) > 1e-5 || fabs(pathingCurrentRotationSpeed.y) > 1e-5 || fabs(pathingCurrentRotationSpeed.z) > 1e-5) {
				log::info("{} pathingCurrentRotationSpeed: {},{},{}", actor_name(actor), pathingCurrentRotationSpeed.x,pathingCurrentRotationSpeed.y,pathingCurrentRotationSpeed.z);
			}
			auto& pathingDesiredRotationSpeed = actor->currentProcess->high->pathingDesiredRotationSpeed;
			if (fabs(pathingDesiredRotationSpeed.x) > 1e-5 || fabs(pathingDesiredRotationSpeed.y) > 1e-5 || fabs(pathingDesiredRotationSpeed.z) > 1e-5) {
				log::info("{} pathingDesiredRotationSpeed: {},{},{}", actor_name(actor), pathingDesiredRotationSpeed.x,pathingDesiredRotationSpeed.y,pathingDesiredRotationSpeed.z);
			}
		}
	}

	void update_effective_multi(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data) {
		if (!actor) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		auto small_massive_threat = Runtime::GetSingleton().SmallMassiveThreat;
		if (!small_massive_threat) {
			return;
		}
		if (actor->HasMagicEffect(small_massive_threat)) {
			persi_actor_data->effective_multi = 2.0;
		} else {
			persi_actor_data->effective_multi = 1.0;
		}
	}

	void update_actor(Actor* actor) {
		auto temp_data = Transient::GetSingleton().GetActorData(actor);
		auto saved_data = Persistent::GetSingleton().GetActorData(actor);
		update_effective_multi(actor, saved_data, temp_data);
		update_height(actor, saved_data, temp_data);
	}

	void apply_actor(Actor* actor, bool force = false) {
		//log::info("Apply_Actor name is {}", actor->GetDisplayFullName());
		auto temp_data = Transient::GetSingleton().GetData(actor);
		auto saved_data = Persistent::GetSingleton().GetData(actor);
		apply_height(actor, saved_data, temp_data, force);
		apply_speed(actor, saved_data, temp_data, force);
	}

	enum ChosenGameMode {
		None,
		Grow,
		Shrink,
		Standard,
		Quest,
	};


	void ApplyGameMode(Actor* actor, const ChosenGameMode& game_mode, const float& GrowthRate, const float& ShrinkRate )  {
		const float EPS = 1e-7;
		if (game_mode != ChosenGameMode::None) {
			float natural_scale = get_natural_scale(actor);
			float Scale = get_visual_scale(actor);
			float maxScale = get_max_scale(actor);
			float targetScale = get_target_scale(actor);
			switch (game_mode) {
				case ChosenGameMode::Grow: {
					float modAmount = Scale * (0.00010 + (GrowthRate * 0.25)) * 60 * Time::WorldTimeDelta();
					if (fabs(GrowthRate) < EPS) {
						return;
					}
					if ((targetScale + modAmount) < maxScale) {
						mod_target_scale(actor, modAmount);
					} else if (targetScale < maxScale) {
						set_target_scale(actor, maxScale);
					} // else let spring handle it
					break;
				}
				case ChosenGameMode::Shrink: {
					float modAmount = Scale * -(0.00025 + (ShrinkRate * 0.25)) * 60 * Time::WorldTimeDelta();
					if (fabs(ShrinkRate) < EPS) {
						return;
					}
					if ((targetScale + modAmount) > natural_scale) {
						mod_target_scale(actor, modAmount);
					} else if (targetScale > natural_scale) {
						set_target_scale(actor, natural_scale);
					} // Need to have size restored by someone
					break;
				}
				case ChosenGameMode::Standard: {
					if (actor->IsInCombat()) {
						float modAmount = Scale * (0.00008 + (GrowthRate * 0.17)) * 60 * Time::WorldTimeDelta();
						if (fabs(GrowthRate) < EPS) {
							return;
						}
						if ((targetScale + modAmount) < maxScale) {
							mod_target_scale(actor, modAmount);
						} else if (targetScale < maxScale) {
							set_target_scale(actor, maxScale);
						} // else let spring handle it
					} else {
						float modAmount = Scale * -(0.00029 + (ShrinkRate * 0.34)) * 60 * Time::WorldTimeDelta();
						if (fabs(ShrinkRate) < EPS) {
							return;
						}
						if ((targetScale + modAmount) > natural_scale) {
							mod_target_scale(actor, modAmount);
						} else if (targetScale > natural_scale) {
							set_target_scale(actor, natural_scale);
						} // Need to have size restored by someone
					}
				}
				case ChosenGameMode::Quest: {
					float modAmount = -ShrinkRate * Time::WorldTimeDelta();
					if (fabs(ShrinkRate) < EPS) {
						return;
					}
					if ((targetScale + modAmount) > natural_scale) {
						mod_target_scale(actor, modAmount);
					} else if (targetScale > natural_scale) {
						set_target_scale(actor, natural_scale);
					} // Need to have size restored by somethig
				}
			}
		}
	}

	void GameMode(Actor* actor)  {
		auto& runtime = Runtime::GetSingleton();

		ChosenGameMode gameMode = ChosenGameMode::None;
		float growthRate = 0.0;
		float shrinkRate = 0.0;
		int game_mode_int = 0;
		float QuestStage = runtime.MainQuest->GetCurrentStageID();

		if (QuestStage > 100.0) {
			if (actor->formID == 0x14) {
				game_mode_int = runtime.ChosenGameMode->value;
				growthRate = runtime.GrowthModeRate->value;
				shrinkRate = runtime.ShrinkModeRate->value;

			} else if (actor->IsPlayerTeammate() || actor->IsInFaction(runtime.FollowerFaction)) {
				game_mode_int = runtime.ChosenGameModeNPC->value;
				growthRate = runtime.GrowthModeRateNPC->value;
				shrinkRate = runtime.ShrinkModeRateNPC->value;
			}
		} else if (QuestStage < 100.0) {
			if (actor->formID == 0x14 && !actor->IsInCombat()) {
				game_mode_int = 4; // QuestMode
				if (QuestStage >= 40 && QuestStage < 60) {
					shrinkRate = 0.00046;
				} else if (QuestStage >= 60 && QuestStage < 70) {
					shrinkRate = 0.00046 / 1.5;
				}

				if (actor->HasMagicEffect(runtime.EffectGrowthPotion)) {
					shrinkRate *= 0.0;
				} else if (actor->HasMagicEffect(runtime.ResistShrinkPotion)) {
					shrinkRate *= 0.25;
				}

				if (fabs(shrinkRate) <= 1e-6) {
					game_mode_int = 0; // Nothing to do
				}
			}
		}


		if (game_mode_int >=0 && game_mode_int <= 4) {
			gameMode = static_cast<ChosenGameMode>(game_mode_int);
		}

		ApplyGameMode(actor, gameMode, growthRate, shrinkRate);
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

// Poll for updates
void GtsManager::Update() {
	for (auto actor: find_actors()) {
		if (!actor) {
			continue;
		}
		if (!actor->Is3DLoaded()) {
			continue;
		}
		//log::info("Found Actor {}", actor->GetDisplayFullName());
		update_actor(actor);
		apply_actor(actor);
		GameMode(actor);
	}
}

void GtsManager::reapply(bool force) {
	// Get everyone in loaded AI data and reapply
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
	// Reapply just this actor
	if (!actor) {
		return;
	}
	if (!actor->Is3DLoaded()) {
		return;
	}
	apply_actor(actor, force);
}
