#include "magic/effects/SizeRelatedDamage.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"

namespace Gts {

	bool SizeDamage::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.SizeRelatedDamage0 || effect == runtime.SizeRelatedDamage1 || effect == runtime.SizeRelatedDamage2);
	}

	void SizeDamage::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		auto target = GetTarget();
		if (!target) {
			return;
		}
		auto name = target->GetDisplayFullName();
		if (actor->IsDead()) {
			log::info("====== SizeRelatedDamage: ====== Actor found DOA: {}", name);
		} else {
			log::info("====== SizeRelatedDamage: ====== Actor found: {}", name);
		}

		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float caster_scale = get_visual_scale(caster);
		float target_scale = get_visual_scale(target);

		float size_difference = caster_scale/target_scale;
		log::info("{}: size_difference: {}, IsPlayerTeammate: {}, IsDead: {}", name, size_difference, target->IsPlayerTeammate(), target->IsDead());
		if (target->IsPlayerTeammate() && runtime.GtsNPCEffectImmunityToggle->value == 1.0
		    || target->HasMagicEffect(runtime.FakeCrushEffect) == true
		    || !target->Is3DLoaded()) {
			return;
		} // Do not apply if those are true

		if (size_difference >= 24.0 && !target->IsPlayerTeammate()) { // NOLINT
			log::info("{}: BranchA");
			caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.FakeCrushSpell, false, target, 1.00f, false, 0.0f, caster);
		}
		// ^ Crush anyway, no conditions needed since size difference is too massive
		else if (size_difference >= 4.0 && target->IsDead() && !target->IsPlayerTeammate()) {
			// ^ We don't want to crush allies
			log::info("{}: BranchB");
			caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.FakeCrushSpell, false, target, 1.00f, false, 0.0f, caster);
		}
		// ^ Crush only if size difference is > than 4.0
		else {
			log::info("{}: Branch NEITHER");
		}

	}
}
