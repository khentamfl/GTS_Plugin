#include "managers/animation/AnimationManager.hpp"
#include "magic/effects/grow_button.hpp"
#include "magic/effects/common.hpp"
#include "managers/Rumble.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace {
	void GrowthTask(Actor* actor) {
		float Start = Time::WorldTimeElapsed();
		ActorHandle gianthandle = actor->CreateRefHandle();
		std::string name = std::format("ManualGrowth_{}", actor->formID);
		TaskManager::RunFor(name, 2.0, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto caster = gianthandle.get().get();

			float timeelapsed = Time::WorldTimeElapsed() - Start;
			float multiply = bezier_curve(timeelapsed, 0, 0.9, 1, 1, 2);
			
			float caster_scale = get_visual_scale(caster);
			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));

			DamageAV(caster, ActorValue::kStamina, 0.45 * (caster_scale * 0.5 + 0.5) * stamina * TimeScale() * multiply);
			Grow(caster, 0.0030 * stamina * multiply, 0.0);

			Rumble::Once("GrowButton", caster, 1.0, 0.05);
			return true;
		});
	}
}

namespace Gts {
	std::string GrowButton::GetName() {
		return "GrowButton";
	}

	void GrowButton::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));
		float Volume = clamp(0.10, 2.0, stamina * get_visual_scale(caster)/8);
		Runtime::PlaySoundAtNode("growthSound", caster, Volume, 1.0, "NPC Pelvis [Pelv]");
		AnimationManager::StartAnim("TriggerGrowth", caster);
		GrowthTask(caster);

		//log::info("Grow Button actor: {}", caster->GetDisplayFullName());

	}

	void GrowButton::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		/*
		float caster_scale = get_visual_scale(caster);
		float stamina = clamp(0.05, 1.0, GetStaminaPercentage(caster));
		DamageAV(caster, ActorValue::kStamina, 0.45 * (caster_scale * 0.5 + 0.5) * stamina * TimeScale());
		Grow(caster, 0.0030 * stamina, 0.0);
		Rumble::Once("GrowButton", caster, 1.0, 0.05);
		*/
	}
}
