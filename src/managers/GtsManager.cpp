#include "Config.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
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
#include "utils/debug.hpp"
#include "managers/Rumble.hpp"

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	void Experiment(Actor* actor) {
		static Timer timer = Timer(5.0);
		if (timer.ShouldRunFrame() && (actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction"))) {
			auto ai = actor->GetActorRuntimeData().currentProcess->high;
			if (ai) {
				//log::info("Fade of {} is {}", actor->GetDisplayFullName(), ai->fadeState);
			}
			auto node = find_node(actor, "NPC Root [Root]");
			auto node2 = find_node(actor, "NPC");
			NiAVObject* Root = node;
			NiAVObject* NPC = node2;
			if (Root) {
				log::info("AV Fade of Root {} is {}", actor->GetDisplayFullName(), Root->fadeAmount);
			} if (NPC) {
				log::info("AV Fade of NPC {} is {}", actor->GetDisplayFullName(), NPC->fadeAmount);
			}
			
			if (node) {
				BSFadeNode* fn = static_cast<BSFadeNode*>(node);
				if (fn) {
					float fl = fn->GetRuntimeData().currentFade;
					log::info("Fade Level NPC Root of {} is {}", actor->GetDisplayFullName(), fl);
				}
			} 
			if (node2) {
				BSFadeNode* fn2 = static_cast<BSFadeNode*>(node2);
				if (fn2) {
					float fl = fn2->GetRuntimeData().currentFade;
					log::info("Fade Level of NPC {} is {}", actor->GetDisplayFullName(), fl);
				}
			}
		}
	}

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

		if (Runtime::HasPerk(actor, "BonusSpeedPerk")) {
			//perkspeed = clamp(0.80, 1.0, speedmultcalc); // Used as a bonus 20% MS if PC has perk.
		}

		persi_actor_data->anim_speed = speedmultcalc*perkspeed;//MS_mult;
		if (actor->formID == 0x14) {
			//log::info("AnimSpeed: {}", persi_actor_data->anim_speed);
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
					float CalcAv = actor->AsActorValueOwner()->GetActorValue(ActorValue::kAlteration);
					float MaxSize = Runtime::GetFloat("CurseOfGrowthMaxSize");                                       // Slider that determines max size cap.
					float sizelimit = clamp(1.0, MaxSize, 1.00 * (CalcAv/100 * MaxSize));                          // Size limit between 1 and [Slider]], based on Alteration. Cap is Slider value.
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
							GrowthPower *= 4.0;                                                                       // Proc super growth if conditions are met
						}
						if (StrongGrowthChance >= 19.0) {
							GrowthPower *= 4.0;                                                                       // Stronger growth if procs
							Rumble::Once("CurseOfGrowth", actor, GrowthPower * 40);
						}
						if (targetScale >= sizelimit) {
							set_target_scale(actor, sizelimit);
						}
						if (((StrongGrowthChance >= 19 && Random >= 19.0) || (StrongGrowthChance >= 19 && MegaGrowth >= 19.0)) && Runtime::GetFloat("AllowMoanSounds") == 1.0) {
							Runtime::PlaySound("MoanSound", actor, targetScale/4, 1.0);
						}
						if (targetScale < maxScale) {
							mod_target_scale(actor, GrowthPower);
							Rumble::Once("CurseOfGrowth", actor, GrowthPower * 20);
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
		if (!actor) {
			return;
		}
		ChosenGameMode gameMode = ChosenGameMode::None;
		float growthRate = 0.0;
		float shrinkRate = 0.0;
		int game_mode_int = 0;
		float QuestStage = Runtime::GetStage("MainQuest");
		float BalanceMode = SizeManager::GetSingleton().BalancedMode();
		float scale = get_visual_scale(actor);
		float BonusShrink = 1.0;
		float bonus = 1.0;
		if (BalanceMode >= 2.0) {
			BonusShrink = (3.5 * (scale * 2));
		}

		if (QuestStage < 100.0 || BalanceMode >= 2.0) {
			if ((actor->formID == 0x14 || actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction"))) {
				game_mode_int = 6; // QuestMode
				if (QuestStage >= 40 && QuestStage < 60) {
					shrinkRate = 0.00086 * (((BalanceMode) * BonusShrink) * 2.2);
				} else if (QuestStage >= 60 && QuestStage < 70) {
					shrinkRate = 0.00086 * (((BalanceMode) * BonusShrink) * 1.6);
				} else if (BalanceMode >= 2.0 && QuestStage > 70) {
					shrinkRate = 0.00086 * (((BalanceMode) * BonusShrink) * 1.50);
				}

				if (Runtime::HasMagicEffect(actor, "EffectGrowthPotion")) {
					shrinkRate *= 0.0;
				} else if (Runtime::HasMagicEffect(actor, "ResistShrinkPotion")) {
					shrinkRate *= 0.25;
				}
				if (Runtime::HasMagicEffect(actor, "explosiveGrowth1") || Runtime::HasMagicEffect(actor, "explosiveGrowth2") || Runtime::HasMagicEffect(actor, "explosiveGrowth3")) {
					shrinkRate *= 0.15;
				}
				if (actor->IsInCombat() && BalanceMode == 1.0) {
					shrinkRate *= 0.0;
				} else if (actor->IsInCombat() && BalanceMode >= 2.0) {
					shrinkRate *= 0.15;
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
	//auto PC = PlayerCharacter::GetSingleton();
	//auto charCont = PC->GetCharController();
	//if (charCont) {
	//float velocity;
	//PC->SetGraphVariableFloat("GiantessVelocity", (charCont->outVelocity.quad.m128_f32[2] * 100)/get_visual_scale(PC));
	//PC->GetGraphVariableFloat("GiantessVelocity", velocity);
	//log::info("GiantessVelocity: {}", velocity);
	//}
	//auto ai = PC->GetActorRuntimeData().currentProcess;
	//static Timer atttimer = Timer(1.00);
	//auto charCont = PC->GetCharController();

	/*if (charCont) {
	        //log::info("JumpHeight: {}, FallStartHeight: {}, IsJumping: {}, Center: {}", charCont->jumpHeight, charCont->fallStartHeight, IsJumping(actor), charCont->center);
	        //log::info("Look At Location: {}", Vector2Str(actor->GetLookingAtLocation()));
	        //log::info("orientationCtrl: {}", charCont->orientationCtrl);
	        bhkCharProxyController* charProxyController = skyrim_cast<bhkCharProxyController*>(charCont);
	   if (charProxyController) {
	        auto proxy = charProxyController->GetCharacterProxy();
	                if (proxy) {
	                        proxy->characterStrength = Runtime::GetFloat("ConversationCameraComp");
	                }
	        }
	   }
	   if (ai) {
	   auto highAi = ai->high;
	   if (highAi && atttimer.ShouldRunFrame()) {
	    //log::info("Player DetectionMod:{}, DetectionModTimer: {}", highAi->detectionModifier, highAi->detectionModifierTimer);
	                //log::info("Player Melee Damage: {}, Unarmed Damage: {}", GetAV(PC, ActorValue::kMeleeDamage), GetAV(PC, ActorValue::kUnarmedDamage));
	                //log::info("{}, Sneak Power: {}, Sneak Mod: {}, Noise Mult: {}", PC->GetDisplayFullName(), GetAV(PC, ActorValue::kSneakingPowerModifier), GetAV(PC, ActorValue::kSneakingModifier), GetAV(PC, ActorValue::kMovementNoiseMult));
	   }
	   }*/

	for (auto actor: find_actors()) {
		if (!actor) {
			continue;
		}
		if (!actor->Is3DLoaded()) {
			continue;
		}

		Experiment(actor);

		auto& accuratedamage = AccurateDamage::GetSingleton();
		auto& sizemanager = SizeManager::GetSingleton();

		if (sizemanager.GetPreciseDamage()) {
			if (actor->formID == 0x14 || actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction")) {
				accuratedamage.DoAccurateCollision(actor);
				ClothManager::GetSingleton().CheckRip();
			}
		}
		if (Runtime::GetBool("PreciseDamageOthers")) {
			if (actor->formID != 0x14 && !actor->IsPlayerTeammate() && !Runtime::InFaction(actor, "FollowerFaction")) {
				accuratedamage.DoAccurateCollision(actor);
			}
		}

		float current_health_percentage = GetHealthPercentage(actor);
		//log::info("Health% before scale: {}", current_health_percentage);
		//log::info("MaxHP Before: {}", GetMaxAV(actor, ActorValue::kHealth));

		update_actor(actor);
		apply_actor(actor);

		SetHealthPercentage(actor, current_health_percentage);
		//log::info("Health% after scale: {}", GetHealthPercentage(actor));
		//log::info("MaxHP After: {}", GetMaxAV(actor, ActorValue::kHealth));

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
