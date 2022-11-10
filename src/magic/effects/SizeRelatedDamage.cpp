#include "magic/effects/SizeRelatedDamage.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "data/transient.hpp"
#include "data/persistent.hpp"
#include "managers/Attributes.hpp"
#include "util.hpp"
#include "timer.hpp"

namespace Gts {
	std::string SizeDamage::GetName() {
		return "SizeDamage";
	}


	bool SizeDamage::StartEffect(EffectSetting* effect) { 
		auto& runtime = Runtime::GetSingleton();
		return (effect == runtime.SizeRelatedDamage0 || effect == runtime.SizeRelatedDamage1 || effect == runtime.SizeRelatedDamage2);
	}

	void SizeDamage::OnUpdate() {
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		//if (!caster->Is3DLoaded()) {
			//return;
		//}

		auto target = GetTarget();
		if (!target) {
			return;
		}
		//if (!target->Is3DLoaded()) {
			//return;
		//}
		if (caster == target) {
			return;
		}
		if (target == caster) {
			return;
		}

		auto& runtime = Runtime::GetSingleton();
		float caster_scale = get_target_scale(caster);
		float target_scale = get_target_scale(target);

		float BonusShrink = 1.0; //IsJumping(caster) * 3.0 + 1.0;

		float size_difference = caster_scale/target_scale;
		log::info("Caster: {}, Target: {}, TargetScale: {}, CasterScale: {}, SizeDifference: {}", caster->GetDisplayFullName(),target->GetDisplayFullName(), target_scale, caster_scale, size_difference);


		if (caster->formID == 0x14 && target->IsPlayerTeammate() && runtime.GtsNPCEffectImmunityToggle->value == 1.0) {
			return;
		} 
		if (caster->IsPlayerTeammate() && target->IsPlayerTeammate() && runtime.GtsNPCEffectImmunityToggle->value == 1.0) {
			return;
		} // Do not apply if those are true

		if (target->HasMagicEffect(runtime.FakeCrushEffect))
		{
			return;
		}

		 if (size_difference >= 24.0 && !target->IsPlayerTeammate() && this->crushtimer.ShouldRunFrame()) {
		 	caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.FakeCrushSpell, false, target, 1.00f, false, 0.0f, caster);
		 	CrushToNothing(caster, target);
		 }
		
		 // ^ Crush anyway, no conditions needed since size difference is too massive
		 if (caster->HasPerk(runtime.ExtraGrowth) && caster != target && (caster->HasMagicEffect(runtime.explosiveGrowth1) || caster->HasMagicEffect(runtime.explosiveGrowth2) || caster->HasMagicEffect(runtime.explosiveGrowth3))) {
		 	ShrinkActor(target, 0.0026 * BonusShrink, 0.0);
		 	Grow(caster, 0.0006 * BonusShrink, 0.0);
		 }
		
		 if (caster->HasMagicEffect(runtime.SmallMassiveThreat)) {
		 	size_difference += 3.2;
		 	if (caster->HasPerk(runtime.SmallMassiveThreatSizeSteal) && caster != target) {
		 		SmallMassiveThreatModification(caster, target);
		 		float HpRegen = caster->GetPermanentActorValue(ActorValue::kHealth) * 0.0008;
		 		caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, (HpRegen * TimeScale()) * size_difference);
		
		 		ShrinkActor(target, 0.002 * BonusShrink, 0.0);
		 		Grow(caster, 0.001 * target_scale * BonusShrink, 0.0);
		 	}
		 }
		
		
		 if (size_difference >= 4.0 && target->IsDead() && !target->IsPlayerTeammate() && this->crushtimer.ShouldRunFrame()) {
		 	// ^ We don't want to crush allies
		 	caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.FakeCrushSpell, false, target, 1.00f, false, 0.0f, caster);
		 	CrushToNothing(caster, target);
		 	NiPointer<TESObjectREFR> instance_ptr = target->PlaceObjectAtMe(runtime.BloodExplosion, false);
		 	if (!instance_ptr) {
		 		return;
		 	}
		 	TESObjectREFR* instance = instance_ptr.get();
		 	if (!instance) {
		 		return;
		 	}
		 	Explosion* explosion = instance->AsExplosion();
		 	explosion->radius *= target_scale;
		 	explosion->imodRadius *= target_scale;
		 }
	}
	void SizeDamage::SmallMassiveThreatModification(Actor* Caster, Actor* Target) {
		if (!Caster || !Target || Caster == Target) {
			return;
		}
		 auto& runtime = Runtime::GetSingleton();
		 auto& persistent = Persistent::GetSingleton();
		 if (persistent.GetData(Caster)->smt_run_speed >= 1.0) {
		 	float caster_scale = get_visual_scale(Caster);
		 	float target_scale = get_visual_scale(Target);
		 	float Multiplier = (caster_scale/target_scale);
		 	float CasterHp = Caster->GetActorValue(ActorValue::kHealth);
		 	float TargetHp = Target->GetActorValue(ActorValue::kHealth);
		 	if (CasterHp >= (TargetHp / Multiplier) && !Target->HasMagicEffect(runtime.FakeCrushEffect) && !Target->HasSpell(runtime.FakeCrushSpell)) {
		 		Caster->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.FakeCrushSpell, false, Target, 1.00f, false, 0.0f, Caster); //  Crush someone
		 		shake_camera(Caster, 0.25 * caster_scale, 0.25);
		
		 		if (!Caster->HasPerk(runtime.NoSpeedLoss)) {
		 			AttributeManager::GetSingleton().OverrideBonus(0.35); // Reduce speed after crush
		 		}
		 	} else if (CasterHp < (TargetHp / Multiplier) && !Target->HasMagicEffect(runtime.FakeCrushEffect) && !Target->HasSpell(runtime.FakeCrushSpell)) {
		 		Caster->ApplyCurrent(0.5 * target_scale, 0.5 * target_scale); Target->ApplyCurrent(0.5 * caster_scale, 0.5 * caster_scale);  Else simulate collision
		 		Target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, -CasterHp * 0.35); Caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,ActorValue::kHealth, -CasterHp * 0.15);
		 		shake_camera(Caster, 0.35, 0.5);
		 		PlaySound(runtime.lJumpLand, Caster, 0.5, 1.0);
		
		 		std::string text_a = Target->GetDisplayFullName();
		 		std::string text_b = " is too tough to be crushed";
		 		std::string Message = text_a + text_b;
		 		DebugNotification(Message.c_str(), 0, true);
		
		 		AttributeManager::GetSingleton().OverrideBonus(0.75); // Less speed loss after force crush
		 	}
		 }
	}

	void SizeDamage::DoSizeRelatedDamage(Actor* Caster, Actor* Target) {
		 static Timer timer = Timer(0.01);
		 float castersize = get_visual_scale(Caster);
		 float targetsize = get_visual_scale(Target);
		 float multiplier = castersize / targetsize;
		 float SprintDamage = 1.0;
		 if (timer.ShouldRunFrame() && multiplier >= 1.33) {
		 	if (Caster->IsSprinting()) {
		 		SprintDamage *= 10.5;
		 	}
		 	DamageAV(Target, ActorValue::kHealth, multiplier);
		 }
	}
}
