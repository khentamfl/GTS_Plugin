#include "magic/effects/Poisons/Poison_Of_Shrinking.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"


namespace Gts {
	std::string Shrink_Poison::GetName() {
		return "Shrink_Poison";
	}

	void Shrink_Poison::OnStart() {
        auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		Rumble::Once("Shrink_Poison", target, 2.0, 0.05);

		float Volume = clamp(0.15, 2.0, get_visual_scale(target)/12);
		Runtime::PlaySound("shrinkSound", target, Volume, 0.6);
	}

	void Shrink_Poison::OnUpdate() {
		const float BASE_POWER = 0.003800;

		auto caster = GetCaster();
		if (!caster) {
			return;
		} 
        auto target = GetTarget();
        if (!target) {
            return;
        }

		float AlchemyLevel = clamp(1.0, 2.0, caster->AsActorValueOwner()->GetActorValue(ActorValue::kAlchemy)/100 + 1.0);
		Rumble::Once("Shrink_Poison", target, 0.4, 0.05);
		float Power = BASE_POWER * get_visual_scale(caster) * AlchemyLevel;

		ShrinkActor(target, Power, 0.0);
        if (ShrinkToNothing(caster, target)) {
            //empty
        }
	}

	void Shrink_Poison::OnFinish() {
	}
}