#include "magic/effects/grow_other_button.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"

namespace Gts {
	std::string GrowOtherButton::GetName() {
		return "GrowOtherButton";
	}

	bool GrowOtherButton::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();

		return effect == runtime.GrowAllySizeButton;
	}

	void GrowOtherButton::OnStart() {
		Actor* caster = GetCaster();
		if (!caster) {
			return;
		}
		Actor* target = GetTarget();
		if (!target) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
		auto GrowthSound = runtime.growthSound;
		float Volume = clamp(0.50, 2.0, get_visual_scale(target));
		PlaySound(GrowthSound, target, Volume, 0.0);
		log::info("Grow Other Button, actor: {}", target->GetDisplayFullName());
	}

	void GrowOtherButton::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();

		float target_scale = get_visual_scale(target);
		float magicka = clamp(0.05, 1.0, GetMagikaPercentage(caster));

		DamageAV(caster, ActorValue::kMagicka, 0.45 * (target_scale * 0.25 + 0.75) * magicka * TimeScale());
		Grow(target, 0.0030 * target_scale * magicka, 0.0);
		GrowthTremorManager::GetSingleton().CallRumble(target, caster, 1.0);
	}
}
