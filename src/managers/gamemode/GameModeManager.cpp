#include "managers/animation/AnimationManager.hpp"
#include "managers/gamemode/GameModeManager.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "utils/debug.hpp"
#include "data/time.hpp"
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
    enum class ChosenGameMode {
		None = 0,
		Grow = 1,
		Shrink = 2,
		Standard = 3,
		StandardNoShrink = 4,
		CurseOfGrowth = 5,
		Quest = 6,
	};
}

namespace Gts {

    GameModeManager& GameModeManager::GetSingleton() noexcept {
		static GameModeManager instance;
		return instance;
	}

	std::string GameModeManager::DebugName() {
		return "GameModeManager";
	}

	void GameModeManager::ApplyGameMode(Actor* actor, const ChosenGameMode& game_mode, const float& GrowthRate, const float& ShrinkRate)  {
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
					break;
				}
				case ChosenGameMode::StandardNoShrink: {
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
					float CalcAv = actor->AsActorValueOwner()->GetActorValue(ActorValue::kAlteration);
					float MaxSize = Runtime::GetFloat("CurseOfGrowthMaxSize");                                       // Slider that determines max size cap.
					float sizelimit = clamp(1.0, MaxSize, 1.00 * (CalcAv/100 * MaxSize));                          // Size limit between 1 and [Slider]], based on Alteration. Cap is Slider value.
					int Random = rand() % 20;                                                                        // Randomize power
					int GrowthTimer = rand() % 6 + 1;                                                                    // Randomize 're-trigger' delay, kinda
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
							Rumble::Once("CurseOfGrowth", actor, GrowthPower * 40, 0.10);
						}
						if (targetScale >= sizelimit) {
							set_target_scale(actor, sizelimit);
						}
						if (((StrongGrowthChance >= 19 && Random >= 19.0) || (StrongGrowthChance >= 19 && MegaGrowth >= 19.0)) && Runtime::GetFloat("AllowMoanSounds") == 1.0) {
							Runtime::PlaySound("MoanSound", actor, targetScale/4, 1.0);
						}
						if (targetScale < maxScale) {
							mod_target_scale(actor, GrowthPower);
							Rumble::Once("CurseOfGrowth", actor, GrowthPower * 20, 0.10);
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
					} // Need to have size restored by something
				}
				break;
			}
		}
	}

	void GameModeManager::GameMode(Actor* actor)  {
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

		GameModeManager::GetSingleton().ApplyGameMode(actor, gameMode, growthRate/2 * ScaleLimit, shrinkRate);
	}
}