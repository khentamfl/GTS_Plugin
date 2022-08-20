#include "magic/effects/SizeRelatedDamage.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "data/transient.hpp"

namespace Gts {
	std::string SizeDamage::GetName() {
		return "SizeDamage";
	}


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

		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;
		float caster_scale = get_visual_scale(caster);
		float target_scale = get_visual_scale(target);

		float size_difference = caster_scale/target_scale;
		if (target->IsPlayerTeammate() && runtime.GtsNPCEffectImmunityToggle->value == 1.0
		    || target->HasMagicEffect(runtime.FakeCrushEffect) == true
		    || !target->Is3DLoaded()) {
			return;
		} // Do not apply if those are true

		if (size_difference >= 24.0 && !target->IsPlayerTeammate()) { // NOLINT
			caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.FakeCrushSpell, false, target, 1.00f, false, 0.0f, caster);
		}
		// ^ Crush anyway, no conditions needed since size difference is too massive
		else if (size_difference >= 4.0 && target->IsDead() && !target->IsPlayerTeammate()) {
			// ^ We don't want to crush allies
			caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.FakeCrushSpell, false, target, 1.00f, false, 0.0f, caster);
			//target->PlaceObjectAtMe(runtime.BloodExplosion, false);
			NiPointer<TESObjectREFR> instance_ptr = target->PlaceObjectAtMe(base_explosion, false);
			if (!instance_ptr) return;
			TESObjectREFR* instance = instance_ptr.get();
			if (!instance) return;
			Explosion* explosion = instance->AsExplosion();
			explosion->radius *= target_scale;
			explosion->imodRadius *= target_scale;
		}
		// ^ Crush only if size difference is > than 4.0

	}

	
}
