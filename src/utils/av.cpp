#include "utils/av.hpp"

using namespace RE;
using namespace SKSE;

namespace Gts {

	float GetMaxAV(Actor* actor, ActorValue av) {
		auto baseValue = actor->GetPermanentActorValue(av);
		auto tempMod = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIERS::kTemporary, av);
		return baseValue + tempMod;
	}
	float GetAV(Actor* actor, ActorValue av) {
		// actor->GetActorValue(av); returns a cached value so we calc directly from mods
		float max_av = GetMaxAV(actor, av);
		auto damageMod = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIERS::kDamage, av);
		return max_av + damageMod;
	}
	void ModAV(Actor* actor, ActorValue av, float amount) {
		actor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kTemporary, av, amount);
	}
	void SetAV(Actor* actor, ActorValue av, float amount) {
		float currentValue = GetAV(actor, av);
		float delta = amount - currentValue;
		ModAV(actor, av, delta);
	}

	void DamageAV(Actor* actor, ActorValue av, float amount) {
		actor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, av, -amount);
	}

	float GetPercentageAV(Actor* actor, ActorValue av) {
		return GetAV(actor, av)/GetMaxAV(actor, av);
	}

	void SetPercentageAV(Actor* actor, ActorValue av, float target) {
		auto currentValue = GetAV(actor, av);
		auto maxValue = GetMaxAV(actor, av);
		auto percentage = currentValue/maxValue;
		auto targetValue = target * maxValue;
		float delta = targetValue - currentValue;
		actor->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, av, delta);
	}

	float GetStaminaPercentage(Actor* actor) {
		return GetPercentageAV(actor, ActorValue::kStamina);
	}

	void SetStaminaPercentage(Actor* actor, float target) {
		SetPercentageAV(actor, ActorValue::kStamina, target);
	}

	float GetHealthPercentage(Actor* actor) {
		return GetPercentageAV(actor, ActorValue::kHealth);
	}

	void SetHealthPercentage(Actor* actor, float target) {
		SetPercentageAV(actor, ActorValue::kHealth, target);
	}

	float GetMagikaPercentage(Actor* actor) {
		return GetPercentageAV(actor, ActorValue::kMagicka);
	}

	void SetMagickaPercentage(Actor* actor, float target) {
		SetPercentageAV(actor, ActorValue::kMagicka, target);
	}

}
