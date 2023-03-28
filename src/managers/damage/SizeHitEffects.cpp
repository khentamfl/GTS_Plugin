#include "Config.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/hitmanager.hpp"
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
		log::info("Stagger: Receiver: {}, Attacker: {}, difference: {}", receiver->GetDisplayFullName(), attacker->GetDisplayFullName(), sizedifference);
		if (charCont) {
			receiver->SetGraphVariableFloat("GiantessScale", sizedifference); // Manages Stagger Resistance inside Behaviors.
		}
	}

	void DoDamage(Actor* attacker, Actor* receiver, float a_damage) {
		float sizedifference = get_visual_scale(attacker)/get_visual_scale(receiver);
		float damagemult = AttributeManager::GetSingleton().GetAttributeBonus(attacker, ActorValue::kAttackDamageMult);
		float damage = (a_damage * damagemult) - a_damage;
		log::info("Damage: Receiver: {}, Attacker: {}, difference: {}, damage: {}", receiver->GetDisplayFullName(), attacker->GetDisplayFullName(), sizedifference, a_damage);
		if (damage < 0) {
			Overkill(attacker, receiver, damage * sizedifference);
			DamageAV(receiver, ActorValue::kHealth, -damage * sizedifference); // Damage hp
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
}