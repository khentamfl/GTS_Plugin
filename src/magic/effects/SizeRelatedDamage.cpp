#include "magic/effects/SizeRelatedDamage.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "managers/GtsSizeManager.hpp"
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
#include "node.hpp"

namespace Gts {
	std::string SizeDamage::GetName() {
		return "SizeDamage";
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
		NiAVObject* npc_node = find_node(caster, "NPC");
		if (!npc_node) {
				return;
		}
		float HighHeels = 1.0 + npc_node->local.translate.z/100;
		//log::info("High Heels: {}, {}", caster->GetDisplayFullName(), HighHeels);

		float Gigantism = 1.0 - SizeManager::GetSingleton().GetEnchantmentBonus(caster)/200; // 50% less effective threshold decrease.
		float InstaCrushRequirement = 24.0 * Gigantism;
		float caster_scale = get_target_scale(caster) * HighHeels;
		float target_scale = get_target_scale(target);
		float BonusShrink = (IsJumping(caster) * 3.0) + 1.0;
		float size_difference = caster_scale/target_scale;


		//log::info("Caster: {}, Target: {}, TargetScale: {}, CasterScale: {}, SizeDifference: {}", caster->GetDisplayFullName(),target->GetDisplayFullName(), target_scale, caster_scale, size_difference);


		if (caster->formID == 0x14 && target->IsPlayerTeammate() && Runtime::GetBool("GtsNPCEffectImmunityToggle")) {
			return;
		}
		if (caster->IsPlayerTeammate() && target->IsPlayerTeammate() && Runtime::GetBool("GtsNPCEffectImmunityToggle")) {
			return;
		} // ^ Do not apply if those are true

		if (Runtime::HasMagicEffect(target, "FakeCrushEffect") || CrushManager::AlreadyCrushed(target)) {
			return;
		}

		DoSizeRelatedDamage(caster, target); // Apply Damage

		if (Runtime::HasPerk(caster, "LethalSprint") && caster->IsSprinting()) {
			InstaCrushRequirement = 14.0 * HighHeels * Gigantism;
		}

		if (size_difference >= InstaCrushRequirement && !target->IsPlayerTeammate() && this->crushtimer.ShouldRunFrame()) {
			CrushManager::Crush(caster, target);
			CrushToNothing(caster, target);
		}

		// ^ Crush anyway, no conditions needed since size difference is too massive
		if (Runtime::HasPerk(caster, "ExtraGrowth") && caster != target && (Runtime::HasMagicEffect(caster, "explosiveGrowth1") || Runtime::HasMagicEffect(caster, "explosiveGrowth2") || Runtime::HasMagicEffect(caster, "explosiveGrowth3"))) {
			ShrinkActor(target, 0.0014 * BonusShrink, 0.0);
			Grow(caster, 0.0, 0.0004 * BonusShrink);
			// ^ Augmentation for Growth Spurt: Steal size of enemies.
		}

		if (Runtime::HasMagicEffect(caster, "SmallMassiveThreat") && caster != target) {
			SmallMassiveThreatModification(caster, target);
			size_difference += 3.2; // Allows to crush same size targets.

			if (Runtime::HasPerk(caster, "SmallMassiveThreatSizeSteal")) {
				float HpRegen = caster->GetPermanentActorValue(ActorValue::kHealth) * 0.0008;
				caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, (HpRegen * TimeScale()) * size_difference);

				ShrinkActor(target, 0.0015 * BonusShrink, 0.0);
				Grow(caster, 0.00045 * target_scale * BonusShrink, 0.0);
			}
		}


		if (size_difference >= 4.0 * Gigantism && target->IsDead() && !target->IsPlayerTeammate() && this->crushtimer.ShouldRunFrame()) {
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
			log::info("Caster's: {} SMT run speed is >= 1.0", Caster->GetDisplayFullName());
			float caster_scale = get_target_scale(Caster);
			float target_scale = get_target_scale(Target);
			float Multiplier = (caster_scale/target_scale);
			float CasterHp = Caster->GetActorValue(ActorValue::kHealth);
			float TargetHp = Target->GetActorValue(ActorValue::kHealth);
			if (CasterHp >= (TargetHp / Multiplier) && !CrushManager::AlreadyCrushed(Target)) {
				CrushManager::Crush(Caster, Target);
				shake_camera(Caster, 0.25 * caster_scale, 0.25);
				log::info("Caster: {} is ready to crush {}", Caster->GetDisplayFullName(), Target->GetDisplayFullName());
				ConsoleLog::GetSingleton()->Print("%s was instantly turned into mush by the body of %s", Target->GetDisplayFullName(), Caster->GetDisplayFullName());
				if (Runtime::HasPerk(Caster, "NoSpeedLoss")) {
					AttributeManager::GetSingleton().OverrideBonus(0.65); // Reduce speed after crush
				}
				else if (!Runtime::HasPerk(Caster, "NoSpeedLoss")) {
					AttributeManager::GetSingleton().OverrideBonus(0.35); // Reduce more speed after crush
				}
			} else if (CasterHp < (TargetHp / Multiplier) && !CrushManager::AlreadyCrushed(Target)) {
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
		float castersize = get_visual_scale(Caster);
		float targetsize = get_visual_scale(Target);
		float multiplier = castersize / targetsize;
		float SprintDamage = 1.0;
		log::info("Caster: {}, Target: {}, Multiplier: {}",Caster->GetDisplayFullName(), Target->GetDisplayFullName(), multiplier);
		if (multiplier >= 1.33) {
			if (Caster->IsSprinting()) {
				SprintDamage *= 10.5;
			}
			DamageAV(Target, ActorValue::kHealth, multiplier);
		}
	}
}
