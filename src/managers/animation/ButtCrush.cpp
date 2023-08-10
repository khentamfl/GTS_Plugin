#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/animation/ButtCrush.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
    const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

    bool CanDoButtCrush(Actor* actor) {
		static Timer Default = Timer(30);
		static Timer UnstableGrowth = Timer(25.5);
        static Timer LoomingDoom = Timer(20.4);
		bool lvl70 = Runtime::HasPerk(actor, "ButtCrush_UnstableGrowth");
        bool lvl100 = Runtime::HasPerk(actor, "ButtCrush_LoomingDoom");
        if (lvl100) {
            return LoomingDoom.ShouldRunFrame();
        } else if (lvl70) {
			return UnstableGrowth.ShouldRunFrame();
		} else {
			return Default.ShouldRunFrame();
		}
	}

    void ModGrowthCount(Actor* giant, float value, bool reset) {
        auto transient = Transient::GetSingleton().GetData(giant);
		if (transient) {
			transient->ButtCrushGrowthAmount += value;
            if (reset) {
                transient->ButtCrushGrowthAmount = 0.0;
            }
		}
    }

    float GetGrowthLimit(Actor* actor) {
        float limit = 0;
        if (Runtime::HasPerkTeam(actor, "ButtCrush_GrowingDisaster")) {
            limit += 2.0;
        } if (Runtime::HasPerkTeam(actor, "ButtCrush_UnstableGrowth")) {
            limit += 2.0;
        } if (Runtime::HasPerkTeam(actor, "ButtCrush_LoomingDoom")) {
            limit += 4.0;
        }
        return limit;
    }

    float GetButtCrushCost(Actor* actor) {
        float cost = 1.0;
        if (Runtime::HasPerkTeam(actor, "ButtCrush_KillerBooty")) {
            cost -= 0.20;
        } if (Runtime::HasPerkTeam(actor, "ButtCrush_LoomingDoom")) {
            cost -= 0.25;
        }
        return cost;
    }

    float GetButtCrushDamage(Actor* actor) {
        float damage = 1.0;
        if (Runtime::HasPerkTeam(actor, "ButtCrush_KillerBooty")) {
            damage += 0.30;
        } if (Runtime::HasPerkTeam(actor, "ButtCrush_UnstableGrowth")) {
            damage += 0.70;
        }
        return damage;
    }

    void GTSButtCrush_GrowthStart(AnimationEventData& data) {
        auto giant = &data.giant;
        ModGrowthCount(giant, 1.0, false);
        Runtime::PlaySoundAtNode("growthSound", giant, 1.0, 1.0, "NPC Pelvis [Pelv]");
		Runtime::PlaySoundAtNode("MoanSound", giant, 1.0, 1.0, "NPC Head [Head]");
    }

    void GTSButtCrush_FallDownImpact(AnimationEventData& data) {
        auto giant = &data.giant;

        float perk = GetPerkBonus_Basics(&data.giant);
        float launch = 1.0;
        float dust = 1.0;
        
        if (HasSMT(giant)) {
            launch = 1.25;
            dust = 1.25;
        }
        float damage = GetButtCrushDamage(giant);
        auto ThighL = find_node(giant, "NPC L Thigh [LThg]");
		auto ThighR = find_node(giant, "NPC R Thigh [RThg]");
        auto ButtR = find_node(giant, "NPC R Butt");
        auto ButtL = find_node(giant, "NPC L Butt");
        if (ButtR && ButtL) {
            if (ThighL && ThighR) {
                DoDamageAtPoint(giant, 24, 480.0 * damage, ThighL, 4, 0.70, 0.85);
                DoDamageAtPoint(giant, 24, 480.0 * damage, ThighR, 4, 0.70, 0.85);
                DoDustExplosion(giant, 1.45 * dust * damage, FootEvent::Right, "NPC R Butt");
                DoDustExplosion(giant, 1.45 * dust * damage, FootEvent::Left, "NPC L Butt");
                DoFootstepSound(giant, 1.25, FootEvent::Right, RNode);
                DoFootstepSound(giant, 1.25, FootEvent::Left, LNode);
                DoLaunch(&data.giant, 28.00 * launch * perk, 4.20, 1.4, FootEvent::Butt, 1.20);
                Rumble::Once("Butt_L", &data.giant, 4.40, 0.02, "NPC R Butt");
                Rumble::Once("Butt_R", &data.giant, 4.40, 0.02, "NPC L Butt");
            }
        }
    }

    void GTSButtCrush_Exit(AnimationEventData& data) {
        ModGrowthCount(&data.giant, 0, true); // Reset limit
    }

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////
    ///
    ///                     T R I G G E R S
    ///
    ///////////////////////////////////////////////////////////////////////////////////////////////////////////


    void ButtCrushStartEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
        if (CanDoButtCrush(player)) {
            float WasteStamina = 200.0 * GetButtCrushCost(player);
            DamageAV(player, ActorValue::kStamina, WasteStamina);
            if (Runtime::HasPerk(player, "ButtCrush_KillerBooty")) {
                AnimationManager::StartAnim("ButtCrush_Start", player);
            } else {
                AnimationManager::StartAnim("ButtCrush_StartFast", player);
            }
        } else {
			TiredSound(player, "Butt Crush is on a cooldown");
		}
	}

    void ButtCrushGrowEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
        if (IsButtCrushing(player)) {
            float GrowthCount = GetGrowthLimit(player);
            bool CanGrow = ButtCrush_IsAbleToGrow(player, GrowthCount);
            if (CanGrow) {
                AnimationManager::StartAnim("ButtCrush_Growth", player);
            } else {
                TiredSound(player, "Your body can't grow any further");
            }
        }
	}

    void ButtCrushAttackEvent(const InputEventData& data) {
        auto player = PlayerCharacter::GetSingleton();
        if (IsButtCrushing(player)) {
            AnimationManager::StartAnim("ButtCrush_Attack", player);
        }
    }
}

namespace Gts
{
	void AnimationButtCrush::RegisterEvents() {
        AnimationManager::RegisterEvent("GTSButtCrush_Exit", "ButtCrush", GTSButtCrush_Exit);
        AnimationManager::RegisterEvent("GTSButtCrush_GrowthStart", "ButtCrush", GTSButtCrush_GrowthStart);
        AnimationManager::RegisterEvent("GTSButtCrush_FallDownImpact", "ButtCrush", GTSButtCrush_FallDownImpact);
        
        InputManager::RegisterInputEvent("ButtCrushStart", ButtCrushStartEvent);
        InputManager::RegisterInputEvent("ButtCrushGrow", ButtCrushGrowEvent);
		InputManager::RegisterInputEvent("ButtCrushAttack", ButtCrushAttackEvent);
	}

    void AnimationButtCrush::RegisterTriggers() {
		AnimationManager::RegisterTrigger("ButtCrush_Start", "ButtCrush", "GTSBEH_ButtCrush_Start");
        AnimationManager::RegisterTrigger("ButtCrush_Attack", "ButtCrush", "GTSBEH_ButtCrush_Attack");
        AnimationManager::RegisterTrigger("ButtCrush_Growth", "ButtCrush", "GTSBEH_ButtCrush_Grow");
        AnimationManager::RegisterTrigger("ButtCrush_StartFast", "ButtCrush", "GTSBEH_ButtCrush_StartFast");
	}
}