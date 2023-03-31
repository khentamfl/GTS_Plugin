#include "Config.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/hitmanager.hpp"
#include "managers/Attributes.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "utils/actorUtils.hpp"
#include "node.hpp"
#include "timer.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;
 
namespace {
    void Overkill(Actor* attacker, Actor* receiver, float damage) {
		if (damage > GetAV(receiver, ActorValue::kHealth) * 1.5) { // Overkill effect
			float attackerscale = get_visual_scale(attacker);
			float receiverscale = get_visual_scale(receiver);
			if (IsDragon(receiver)) {
				receiverscale *= 2.0;
			}
			float size_difference = attackerscale/receiverscale;
			if (size_difference >= 18.0) {
				HitManager::GetSingleton().Overkill(receiver, attacker);
			}
		}
	}

	void StaggerImmunity(Actor* attacker, Actor* receiver) {
		float sizedifference = get_visual_scale(receiver)/get_visual_scale(attacker);
		auto charCont = receiver->GetCharController();
		if (charCont) {
			receiver->SetGraphVariableFloat("GiantessScale", sizedifference); // Manages Stagger Resistance inside Behaviors.
		}
	}


	void HealthGate(Actor* attacker, Actor* receiver, float a_damage) {
		if (a_damage > GetAV(receiver, ActorValue::kHealth)) {
			if (Runtime::HasPerk(receiver, "HealthGate")) {
				static Timer protect = Timer(30.00);
				if (protect.ShouldRunFrame()) {
					float maxhp = GetMaxAV(receiver, ActorValue::kHealth);

					attacker->SetGraphVariableFloat("staggerMagnitude", 100.00f); // Stagger actor
					attacker->NotifyAnimationGraph("staggerStart");

					if (get_visual_scale(receiver) > 1.0) {
						mod_target_scale(receiver, -0.25);
					}

					Rumble::For("CheatDeath", receiver, 120.0, 0.10, "NPC COM [COM ]", 0.75);
					Runtime::PlaySound("TriggerHG", receiver, 2.0, 0.5);
					receiver->SetGraphVariableFloat("staggerMagnitude", 100.00f); // Stagger actor
					receiver->NotifyAnimationGraph("staggerStart");

					float overkill = a_damage + maxhp/5;
		
					receiver->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, overkill); // Restore to full
					log::info("Applying Health Gate, overkill: {}, damage: {}", overkill, a_damage);
				}
			}
		}
	}

	void DoDamage(Actor* attacker, Actor* receiver, float a_damage) {
		float damagemult = AttributeManager::GetSingleton().GetAttributeBonus(attacker, ActorValue::kAttackDamageMult);
		float damage = (a_damage * damagemult) - a_damage;
		//log::info("Damage: Receiver: {}, Attacker: {}, a_damage: {}, damage: {}", receiver->GetDisplayFullName(), attacker->GetDisplayFullName(), a_damage, damage);

		HealthGate(attacker, receiver, -(a_damage + damage));

		if (damage < 0) {
			Overkill(attacker, receiver, -(a_damage + damage));
			DamageAV(receiver, ActorValue::kHealth, -damage); // Damage hp
			return;
		}
		if (damage > 0) {
			receiver->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, damage); // Restore hp
		}
	}
}

namespace Gts {

	SizeHitEffects& SizeHitEffects::GetSingleton() noexcept {
		static SizeHitEffects instance;
		return instance;
	}

	std::string SizeHitEffects::DebugName() {
		return "SizeHitEffects";
	}

    void SizeHitEffects::ApplyEverything(Actor* attacker, Actor* receiver, float damage) {
        DoDamage(attacker, receiver, damage);
        StaggerImmunity(attacker, receiver);
    }

	void SizeHitEffects::BreakBones(Actor* giant, Actor* tiny, float damage, int random) { // Used as a debuff 
		if (tiny->IsDead()) {
			return;
		}
		if (!Runtime::HasPerkTeam(giant, "BoneCrusher")) {
			return;
		}
		int rng = (rand()% random + 1);
		if (rng <= 2) {
			float gs = get_visual_scale(giant);
    		float ts = get_visual_scale(tiny);
			if (Runtime::HasMagicEffect(giant, "SmallMassiveThreat")) {
				gs += 3.0; // Allow to break bones with SMT
			}
    		float sizediff = gs/ts;
    		if (sizediff < 3.0) {
        		return;
			}

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> dis(-0.2, 0.2);

			Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreSmallest", "NPC Spine [Spn0]", NiPoint3{dis(gen), 0, -1}, 512, true, true);
        	SizeManager::GetSingleton().ModSizeVulnerability(tiny, 0.15);
			DamageAV(tiny, ActorValue::kHealth, damage * 10);
		}
	}
}