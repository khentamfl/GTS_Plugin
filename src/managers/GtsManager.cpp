#include "Config.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GtsManager.hpp"
#include "managers/highheel.hpp"
#include "managers/Attributes.hpp"
#include "managers/InputManager.hpp"
#include "managers/hitmanager.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "scale/scale.hpp"
#include "scale/scalespellmanager.hpp"
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

		float scaleOverride = persi_actor_data->scaleOverride;
		if (scaleOverride >= 1e-4) {
			visual_scale = scaleOverride;
			log::info("Scale overrided to {}", visual_scale);
		}

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
		float scale = get_target_scale(actor);
		if (scale < 1e-5) {
			//log::info("!SCALE IS < 1e-5! {}", actor->GetDisplayFullName());
			return;
		}
		SoftPotential& speed_adjustment = Persistent::GetSingleton().speed_adjustment;
		SoftPotential& MS_adjustment = Persistent::GetSingleton().MS_adjustment;

		SoftPotential speed_adjustment_others { // Even though it is named 'sprint', it is used for all other movement states
			.k = 0.142, // 0.125
			.n = 0.82, // 0.86
			.s = 1.90, // 1.12
			.o = 1.0,
			.a = 0.0,  //Default is 0
		};

		SoftPotential speed_adjustment_walk { // Used for normal walk, it has faster animation speed for some reason, so it needs a custom one
			.k = 0.265, // 0.125
			.n = 1.11, // 0.86
			.s = 2.0, // 1.12
			.o = 1.0,
			.a = 0.0,  //Default is 0
		};

		float speed_mult_others = soft_core(scale, speed_adjustment_others); // For all other movement types
		float speed_mult_walk = soft_core(scale, speed_adjustment_walk); // For Walking

		float speed_mult = soft_core(scale, speed_adjustment);
		float MS_mult = soft_core(scale, MS_adjustment);
		float Bonus = Persistent::GetSingleton().GetActorData(actor)->smt_run_speed;
		float MS_mult_sprint_limit = clamp(0.65, 1.0, MS_mult); // For sprint
		float MS_mult_limit = clamp(0.750, 1.0, MS_mult); // For Walk speed
		float Multy = clamp(0.70, 1.0, MS_mult); // Additional 30% ms
		//float WalkSpeedLimit = clamp(0.33, 1.0, MS_mult);
		float WalkSpeedLimit = clamp(0.02, 1.0, MS_mult);
		float PerkSpeed = 1.0;

		static Timer timer = Timer(0.10); // Run every 0.10s or as soon as we can
		float IsFalling = Runtime::GetInt("IsFalling");

		if (actor->formID == 0x14 && IsJumping(actor) && IsFalling == 0.0) {
			Runtime::SetInt("IsFalling", 1.0);
		} else if (actor->formID == 0x14 && !IsJumping(actor) && IsFalling >= 1.0) {
			Runtime::SetInt("IsFalling", 0);
		}
		if (Runtime::HasPerk(actor, "BonusSpeedPerk")) {
			PerkSpeed = clamp(0.80, 1.0, speed_mult_walk); // Used as a bonus 20% MS if PC has perk.
		}

		if (!actor->IsRunning()) {
			persi_actor_data->anim_speed = speed_mult_others;//MS_mult;
		} else if (actor->IsRunning() && !actor->IsSprinting() && !actor->IsSneaking()) {
			persi_actor_data->anim_speed = speed_mult_walk * PerkSpeed;
		}


		if (timer.ShouldRunFrame()) {
			if (scale < 1.0) {
				actor->SetActorValue(ActorValue::kSpeedMult, trans_actor_data->base_walkspeedmult * scale * (Bonus/2.2 + 1.0));
				if (actor->formID == 0x14) {
					//log::info("Player Scale is < 1.0, Scale: {}, BaseWalkSpeedMult with ADjustments: {}, Total: {} ", scale, trans_actor_data->base_walkspeedmult * scale, trans_actor_data->base_walkspeedmult);
				}
			} else {
				actor->SetActorValue(ActorValue::kSpeedMult, ((trans_actor_data->base_walkspeedmult * (Bonus/2.2 + 1.0)))/ (MS_mult)/MS_mult_limit/Multy/PerkSpeed);
				if (actor->formID == 0x14) {
					//log::info("Player Scale is > 1.0, Scale: {}, BaseWalkSpeedMult with Adjustments: {}, Total: {} ", scale, ((trans_actor_data->base_walkspeedmult * (Bonus/2.4 + 1.0)))/ (MS_mult)/MS_mult_limit/Multy/PerkSpeed, trans_actor_data->base_walkspeedmult);
				}
			}
		}
		// Experiement
		if (false) {
			auto& rot_speed = actor->currentProcess->middleHigh->rotationSpeed;
			auto actor_name = actor->GetDisplayFullName();
			if (fabs(rot_speed.x) > 1e-5 || fabs(rot_speed.y) > 1e-5 || fabs(rot_speed.z) > 1e-5) {
				log::info("{} rotationSpeed: {},{},{}", actor_name, rot_speed.x,rot_speed.y,rot_speed.z);
				actor->currentProcess->middleHigh->rotationSpeed.x *= speed_mult;
				actor->currentProcess->middleHigh->rotationSpeed.y *= speed_mult;
				actor->currentProcess->middleHigh->rotationSpeed.z *= speed_mult;
			}
			auto& animationDelta = actor->currentProcess->high->animationDelta;
			if (fabs(animationDelta.x) > 1e-5 || fabs(animationDelta.y) > 1e-5 || fabs(animationDelta.z) > 1e-5) {
				log::info("{} animationDelta: {},{},{}", actor_name, animationDelta.x,animationDelta.y,animationDelta.z);
			}
			auto& animationAngleMod = actor->currentProcess->high->animationAngleMod;
			if (fabs(animationAngleMod.x) > 1e-5 || fabs(animationAngleMod.y) > 1e-5 || fabs(animationAngleMod.z) > 1e-5) {
				log::info("{} animationAngleMod: {},{},{}", actor_name, animationAngleMod.x,animationAngleMod.y,animationAngleMod.z);
			}
			auto& pathingCurrentRotationSpeed = actor->currentProcess->high->pathingCurrentRotationSpeed;
			if (fabs(pathingCurrentRotationSpeed.x) > 1e-5 || fabs(pathingCurrentRotationSpeed.y) > 1e-5 || fabs(pathingCurrentRotationSpeed.z) > 1e-5) {
				log::info("{} pathingCurrentRotationSpeed: {},{},{}", actor_name, pathingCurrentRotationSpeed.x,pathingCurrentRotationSpeed.y,pathingCurrentRotationSpeed.z);
			}
			auto& pathingDesiredRotationSpeed = actor->currentProcess->high->pathingDesiredRotationSpeed;
			if (fabs(pathingDesiredRotationSpeed.x) > 1e-5 || fabs(pathingDesiredRotationSpeed.y) > 1e-5 || fabs(pathingDesiredRotationSpeed.z) > 1e-5) {
				log::info("{} pathingDesiredRotationSpeed: {},{},{}", actor_name, pathingDesiredRotationSpeed.x,pathingDesiredRotationSpeed.y,pathingDesiredRotationSpeed.z);
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
		if (Runtime::HasMagicEffect(actor, "SmallMassiveThreat")) {
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

	enum class ChosenGameMode {
		None = 0,
		Grow = 1,
		Shrink = 2,
		Standard = 3,
		StandardNoShrink = 4,
		CurseOfGrowth = 5,
		Quest = 6,
	};


	void ApplyGameMode(Actor* actor, const ChosenGameMode& game_mode, const float& GrowthRate, const float& ShrinkRate )  {
		const float EPS = 1e-7;
		if (game_mode != ChosenGameMode::None) {
			auto player = PlayerCharacter::GetSingleton();
			float natural_scale = get_natural_scale(actor);
			float Scale = get_target_scale(actor);
			float maxScale = get_max_scale(actor);
			float targetScale = get_target_scale(actor);

			if (Runtime::GetFloat("MultiplyGameModePC") == 0 && actor == player) {
				Scale = 1.0;
			}
			if (Runtime::GetFloat("MultiplyGameModeNPC") == 0 && actor != player) {
				Scale = 1.0;
			}

			switch (game_mode) {
				case ChosenGameMode::Grow: {
					//log::info("Growth, GameMode PC: {}, NPC: {}", Runtime::GetInt("ChosenGameMode"), Runtime::GetInt("ChosenGameModeNPC"));
					float modAmount = Scale * (0.00010 + (GrowthRate * 0.25)) * 60 * Time::WorldTimeDelta();
					if (fabs(GrowthRate) < EPS) {
						log::info("Returning");
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
					//log::info("Shrink, GameMode PC: {}, NPC: {}", Runtime::GetInt("ChosenGameMode"), Runtime::GetInt("ChosenGameModeNPC"));
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
					//log::info("Growth In Combat, GameMode PC: {}, NPC: {}", Runtime::GetInt("ChosenGameMode"), Runtime::GetInt("ChosenGameModeNPC"));
					//log::info("Standart Game Mode; Character {} is In Combat: {}", actor->GetDisplayFullName(), actor->IsInCombat());
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
					break;
				}
				case ChosenGameMode::StandardNoShrink: {
					//log::info("Slow Growth In Combat, GameMode PC: {}, NPC: {}", Runtime::GetInt("ChosenGameMode"), Runtime::GetInt("ChosenGameModeNPC"));
					//log::info("Standart No Shrink Game Mode; Character {} is In Combat: {}", actor->GetDisplayFullName(), actor->IsInCombat());
					if (actor->IsInCombat()) {
						float modAmount = Scale * (0.00008 + (GrowthRate * 0.17)) * 60 * Time::WorldTimeDelta();
						if (fabs(GrowthRate) < EPS) {
							return;
						}
						if ((targetScale + modAmount) < maxScale) {
							mod_target_scale(actor, modAmount * 0.33);
						} else if (targetScale < maxScale) {
							set_target_scale(actor, maxScale);
						} // else let spring handle it
					}
					break;
				}
				case ChosenGameMode::CurseOfGrowth: {
					//log::info("Curse Of Growth, GameMode PC: {}, NPC: {}", Runtime::GetInt("ChosenGameMode"), Runtime::GetInt("ChosenGameModeNPC"));
					float CalcAv = actor->GetActorValue(ActorValue::kAlteration);
					float MaxSize = Runtime::GetFloat("CurseOfGrowthMaxSize");                                                               // Slider that determines max size cap.
					float sizelimit = clamp(1.0, MaxSize, (1.00 * (CalcAv/100 * MaxSize)));                                                  // Size limit between 1 and [Slider]], based on Alteration. Cap is Slider value.
					int Random = rand() % 20;                                                                        // Randomize power
					int GrowthTimer = rand() % 7;                                                                    // Randomize 're-trigger' delay, kinda
					int StrongGrowthChance = rand() % 20;                                                            // Self-explanatory
					int MegaGrowth = rand() % 20;                                                                    // A chance to multiply growth again
					float GrowthPower = CalcAv*0.00240 / Random;                                                     // Randomized strength of growth
					static Timer timer = Timer(1.40 * GrowthTimer);                                                  // How often it procs
					if (targetScale >= sizelimit || Random <= 0 || GrowthTimer <= 0) {
						return; // Protections against infinity
					}
					if (timer.ShouldRunFrame()) {
						if (StrongGrowthChance >= 19 && MegaGrowth >= 19.0) {
							GrowthPower *= 4.0;                                                                        // Proc super growth if conditions are met
						}
						if (StrongGrowthChance >= 19.0) {
							GrowthPower *= 4.0;                                                                         // Stronger growth if procs
							GrowthTremorManager::GetSingleton().CallRumble(actor, player, GrowthPower * 40);
						}
						if (targetScale >= sizelimit) {
							set_target_scale(actor, sizelimit);
						}
						if (((StrongGrowthChance >= 19 && Random >= 19.0) || (StrongGrowthChance >= 19 && MegaGrowth >= 19.0)) && Runtime::GetFloat("AllowMoanSounds") == 1.0) {
							Runtime::PlaySound("MoanSound", actor, targetScale/4, 1.0);
						}
						if (targetScale < maxScale) {
							mod_target_scale(actor, GrowthPower);
							GrowthTremorManager::GetSingleton().CallRumble(actor, player, GrowthPower * 20);
							Runtime::PlaySound("growthSound", actor, GrowthPower * 6, 1.0);
						}
					}
					break;
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
				break;
			}
		}
	}

	void GameMode(Actor* actor)  {
		ChosenGameMode gameMode = ChosenGameMode::None;
		float growthRate = 0.0;
		float shrinkRate = 0.0;
		int game_mode_int = 0;
		float QuestStage = Runtime::GetStage("MainQuest");
		float BalanceMode = SizeManager::GetSingleton().BalancedMode();
		float scale = get_target_scale(actor);
		float BonusShrink = 1.0;
		float bonus = 1.0;
		if (BalanceMode >= 2.0) {
			BonusShrink = (3.5 * (scale * 2));
		}

		if (QuestStage < 100.0 || BalanceMode >= 2.0) {
			if (actor->formID == 0x14 && !actor->IsInCombat()) {
				game_mode_int = 6; // QuestMode
				if (QuestStage >= 40 && QuestStage < 60) {
					shrinkRate = 0.00086 * (((BalanceMode) * BonusShrink) * 2.2);
				} else if (QuestStage >= 60 && QuestStage < 70) {
					shrinkRate = 0.00086 * (((BalanceMode) * BonusShrink) * 1.6);
				} else if (BalanceMode >= 2.0 && QuestStage > 70) {
					shrinkRate = 0.00086 * (((BalanceMode) * BonusShrink) * 0.75);
				}


				if (Runtime::HasMagicEffect(actor, "EffectGrowthPotion")) {
					shrinkRate *= 0.0;
				} else if (Runtime::HasMagicEffect(actor, "ResistShrinkPotion")) {
					shrinkRate *= 0.25;
				}

				if (fabs(shrinkRate) <= 1e-6) {
					game_mode_int = 0; // Nothing to do
				}
			}
		} else if (QuestStage > 100.0 && BalanceMode <= 1.0) {
			if (actor->formID == 0x14) {
				if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(), "EffectSizeAmplifyPotion")) {
					bonus = scale * 0.25 + 0.75;
				}
				game_mode_int = Runtime::GetInt("ChosenGameMode");
				growthRate = Runtime::GetFloat("GrowthModeRate");
				shrinkRate = Runtime::GetFloat("ShrinkModeRate");

			} else if (actor->formID != 0x14 && (actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction"))) {
				if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(), "EffectSizeAmplifyPotion")) {
					bonus = scale * 0.25 + 0.75;
				}
				game_mode_int = Runtime::GetInt("ChosenGameModeNPC");
				growthRate = Runtime::GetFloat("GrowthModeRateNPC") * bonus;
				shrinkRate = Runtime::GetFloat("ShrinkModeRateNPC");
			}
		}

		if (game_mode_int >=0 && game_mode_int <= 6) {
			gameMode = static_cast<ChosenGameMode>(game_mode_int);
		}

		if (Runtime::GetFloat("MultiplyGameModePC") == 0 && actor->formID == 0x14) {
			scale = 1.0;
		}
		if (Runtime::GetFloat("MultiplyGameModeNPC") == 0 && actor->formID != 0x14) {
			scale = 1.0;
		}
		float ScaleCheck = scale * 0.15;
		float ScaleLimit = Gts::clamp(1.0, 10.0, ScaleCheck);
		//log::info("Growth Scale Limit is: {}", ScaleLimit);

		ApplyGameMode(actor, gameMode, growthRate/2 * ScaleLimit, shrinkRate);
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
	auto PC = PlayerCharacter::GetSingleton();
	/*auto ai = PC->currentProcess;
	   if (ai) {
	        auto highAi = ai->high;
	        if (highAi) {
	                log::info("Heatracking Test: {}", highAi->headTrackTargetOffset);
	        }
	   }*/

	ClothManager::GetSingleton().CheckRip();

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
		static Timer timer = Timer(3.00); // Add Size-related spell once per 3 sec
		if (timer.ShouldRunFrame()) {
			ScaleSpellManager::GetSingleton().CheckSize(actor);
		}
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
