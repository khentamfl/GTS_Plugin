#include "magic/effects/SizeRelatedDamage.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "data/transient.hpp"
#include "data/persistent.hpp"
#include "managers/Attributes.hpp"
#include "managers/CrushManager.hpp"
#include "util.hpp"
#include "timer.hpp"

namespace Gts {
	std::string SizeDamage::GetName() {
		return "SizeDamage";
	}


	bool SizeDamage::StartEffect(EffectSetting* effect) {
		return (effect == Runtime::GetMagicEffect("SizeRelatedDamage0") || effect == Runtime::GetMagicEffect("SizeRelatedDamage1") || effect == Runtime::GetMagicEffect("SizeRelatedDamage2"));
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
		if (caster == target) {
			return;
		}
		if (target == caster) {
			return;
		}

		float InstaCrushRequirement = 24.0;
		float caster_scale = get_target_scale(caster);
		float target_scale = get_target_scale(target);
		float BonusShrink = 1.0; //IsJumping(caster) * 3.0 + 1.0;
		float size_difference = caster_scale/target_scale;

		//log::info("Caster: {}, Target: {}, TargetScale: {}, CasterScale: {}, SizeDifference: {}", caster->GetDisplayFullName(),target->GetDisplayFullName(), target_scale, caster_scale, size_difference);


		if (caster->formID == 0x14 && target->IsPlayerTeammate() && Runtime::GetBool("GtsNPCEffectImmunityToggle")) {
			return;
		}
		if (caster->IsPlayerTeammate() && target->IsPlayerTeammate() && Runtime::GetBool("GtsNPCEffectImmunityToggle")) {
			return;
		} // ^ Do not apply if those are true

		if (Runtime::HasMagicEffect(target, "FakeCrushEffect")) {
			return;
		}

		if (Runtime::HasPerk(caster, "LethalSprint") && caster->IsSprinting()) {
			InstaCrushRequirement = 14.0;
		}

		if (size_difference >= InstaCrushRequirement && !target->IsPlayerTeammate() && this->crushtimer.ShouldRunFrame()) {
			CrushManager::Crush(caster, target);
			CrushToNothing(caster, target);
		}

		// ^ Crush anyway, no conditions needed since size difference is too massive
		if (Runtime::HasPerk(caster, "ExtraGrowth") && caster != target && (Runtime::HasMagicEffect(caster, "explosiveGrowth1") || Runtime::HasMagicEffect(caster, "explosiveGrowth2") || Runtime::HasMagicEffect(caster, "explosiveGrowth3"))) {
			ShrinkActor(target, 0.0026 * BonusShrink, 0.0);
			Grow(caster, 0.0006 * BonusShrink, 0.0);
		}

		if (Runtime::HasMagicEffect(caster, "SmallMassiveThreat")) {
			size_difference += 3.2;
			if (Runtime::HasPerk(caster, "SmallMassiveThreatSizeSteal") && caster != target) {
				SmallMassiveThreatModification(caster, target);
				float HpRegen = caster->GetPermanentActorValue(ActorValue::kHealth) * 0.0008;
				caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, (HpRegen * TimeScale()) * size_difference);

				ShrinkActor(target, 0.002 * BonusShrink, 0.0);
				Grow(caster, 0.001 * target_scale * BonusShrink, 0.0);
			}
		}


		if (size_difference >= 4.0 && target->IsDead() && !target->IsPlayerTeammate() && this->crushtimer.ShouldRunFrame()) {
			// ^ We don't want to crush allies
			CrushManager::Crush(caster, target);
			CrushToNothing(caster, target);
			Runtime::CreateExplosion(target, target_scale,"BloodExplosion");
		}
	}
	void SizeDamage::SmallMassiveThreatModification(Actor* Caster, Actor* Target) {
		if (!Caster || !Target || Caster == Target) {
			return;
		}
		auto& persistent = Persistent::GetSingleton();
		if (persistent.GetData(Caster)->smt_run_speed >= 1.0) {
			float caster_scale = get_visual_scale(Caster);
			float target_scale = get_visual_scale(Target);
			float Multiplier = (caster_scale/target_scale);
			float CasterHp = Caster->GetActorValue(ActorValue::kHealth);
			float TargetHp = Target->GetActorValue(ActorValue::kHealth);
			if (CasterHp >= (TargetHp / Multiplier) && !Runtime::HasMagicEffect(Target, "FakeCrushEffect") && !Runtime::HasSpell(Target, "FakeCrushSpell")) {
				CrushManager::Crush(caster, target);
				shake_camera(Caster, 0.25 * caster_scale, 0.25);

				if (!Runtime::HasPerk(Caster, "NoSpeedLoss")) {
					AttributeManager::GetSingleton().OverrideBonus(0.35); // Reduce speed after crush
				}
			} else if (CasterHp < (TargetHp / Multiplier) && !Runtime::HasMagicEffect(Target, "FakeCrushEffect") && !Runtime::HasSpell(Target, "FakeCrushSpell")) {
				Caster->ApplyCurrent(0.5 * target_scale, 0.5 * target_scale); Target->ApplyCurrent(0.5 * caster_scale, 0.5 * caster_scale);  // Else simulate collision
				Target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, -CasterHp * 0.35); Caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,ActorValue::kHealth, -CasterHp * 0.15);
				shake_camera(Caster, 0.35, 0.5);
				Runtime::PlaySound("lJumpLand", Caster, 0.5, 1.0);

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
