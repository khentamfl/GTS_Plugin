#include "magic/effects/shrink_other_button.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"

namespace Gts {
	std::string ShrinkOtherButton::GetName() {
		return "ShrinkOtherButton";
	}

	bool ShrinkOtherButton::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();

		return effect == runtime.ShrinkAllySizeButton;
	}

	void ShrinkOtherButton::OnStart() {
		auto target = GetTarget();
		if (!target) {
			return;
		}
		auto& runtime = Runtime::GetSingleton();
		auto ShrinkSound = runtime.shrinkSound;
		float Volume = clamp(0.50, 1.0, get_visual_scale(target));
		PlaySound(ShrinkSound, target, Volume, 0.0);
		log::info("Shrink Other Button, actor: {}", target->GetDisplayFullName());
	}
	

	void ShrinkOtherButton::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}

		float target_scale = get_visual_scale(target);
		float magicka = clamp(0.05, 1.0, GetMagikaPercentage(caster));

		if (target_scale > get_natural_scale(target)) {
			DamageAV(caster, ActorValue::kMagicka, 0.25 * (target_scale * 0.25 + 0.75) * magicka * TimeScale());
			ShrinkActor(target, 0.0030*magicka, 0.0);
		}
	}
}
