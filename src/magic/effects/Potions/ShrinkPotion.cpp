#include "magic/effects/Potions/ShrinkPotion.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"


namespace Gts {
	std::string ShrinkPotion::GetName() {
		return "ShrinkPotion";
	}

	void ShrinkPotion::OnStart() {
		auto caster = GetCaster();
		auto player = PlayerCharacter::GetSingleton();
		if (!caster || !player) {
			return;
		}

		Rumble::Once("ShrinkPotion", caster, 2.0, 0.05, 1.0);

		float Volume = clamp(0.15, 2.0, get_visual_scale(caster)/8);
		Runtime::PlaySound("growthSound", caster, Volume, 1.0);
		log::info("Growth Potion start actor: {}", caster->GetDisplayFullName());
	}

	void ShrinkPotion::OnUpdate() {
		const float BASE_POWER = 0.000480;

		auto caster = GetCaster();
		if (!caster) {
			return;
		}

		float AlchemyLevel = clamp(1.0, 2.0, caster->AsActorValueOwner()->GetActorValue(ActorValue::kAlchemy)/100 + 1.0);
		Rumble::Once("ShrinkPotion", caster, 0.4, 0.05, 1.0);

		float Power = BASE_POWER * get_visual_scale(caster) * AlchemyLevel;

		ShrinkActor(caster, Power, 0.0);
	}

	void ShrinkPotion::OnFinish() {
	}
}
