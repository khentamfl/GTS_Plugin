#include "managers/GtsManager.hpp"
#include "magic/effects/Potions/GrowthPotion.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "timer.hpp"
#include "managers/Rumble.hpp"

namespace Gts {
	std::string GrowthPotion::GetName() {
		return "GrowthPotion";
	}

	void GrowthPotion::OnStart() {
		auto caster = GetCaster();
		auto player = PlayerCharacter::GetSingleton();
		if (!caster || !player) {
			return;
		}

		Rumble::Once("GrowthPotion", caster, 2.0, 0.05);

		float Volume = clamp(0.15, 2.0, get_visual_scale(caster)/8);
		Runtime::PlaySound("growthSound", caster, Volume, 1.0);
		log::info("Growth Potion start actor: {}", caster->GetDisplayFullName());
	}

	void GrowthPotion::OnUpdate() {
		const float BASE_POWER = 0.000180;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		float AlchemyLevel = clamp(1.0, 2.0, caster->AsActorValueOwner()->GetActorValue(ActorValue::kAlchemy)/100 + 1.0);
		if (caster == PlayerCharacter::GetSingleton()) {
			Rumble::Once("GrowthPotion", caster, 0.4, 0.05);
		}

		if (this->timer.ShouldRun()) {
			float Volume = clamp(0.15, 1.0, get_visual_scale(caster)/4);
			Runtime::PlaySound("growthSound", caster, Volume, 1.0);
			log::info("Growth Potion Loop caster: {}", caster->GetDisplayFullName());
		}
		float HP = GetMaxAV(caster, ActorValue::kHealth) * 0.00035;
		caster->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HP * TimeScale());

		float Power = BASE_POWER * get_visual_scale(caster) * AlchemyLevel;

		Grow(caster, Power, 0.0);
		Rumble::Once("GrowButton", caster, 0.6, 0.05);
	}

	void GrowthPotion::OnFinish() {
	}
}
