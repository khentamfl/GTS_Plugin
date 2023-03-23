#include "magic/effects/vore_growth.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "utils/actorUtils.hpp"


namespace Gts {
	std::string VoreGrowth::GetName() {
		return "VoreGrowth";
	}

	void VoreGrowth::OnStart() {
		auto target = GetTarget();
		float Scale = get_target_scale(target);
		this->ScaleOnVore = Scale;
		if (IsDragon(target)) {
			this->ScaleOnVore = 1.0;
		}
	}

	void VoreGrowth::OnUpdate() {
		float BASE_POWER = 0.0003800;
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
		float bonus = 1.0;
		float GrowAmount = 1.0;
		if (IsDragon(target)) {
			GrowAmount *= 6.0;
		}
		BASE_POWER *= GrowAmount;
		if (Runtime::HasPerk(caster, "AdditionalAbsorption")) {
			BASE_POWER *= 2.0;
		}

		if (Runtime::HasMagicEffect(PlayerCharacter::GetSingleton(),"EffectSizeAmplifyPotion")) {
			bonus = get_visual_scale(caster) * 0.25 + 0.75;
		}
		//log::info("Vore Growth Actor: {}, Target: {}", caster->GetDisplayFullName(), target->GetDisplayFullName());
		VoreRegeneration(caster);
		Grow(caster, 0, BASE_POWER * bonus);
	}

	void VoreGrowth::OnFinish() {
		auto giant = GetCaster();
		if (!giant) {
			return;
		}
		auto tiny = GetTarget();
		if (!tiny) {
			return;
		}
		if (giant == tiny) {
			return;
		}
		VoreBuffAttributes();
		auto progressionQuest = Runtime::GetQuest("MainQuest");
		if (giant->formID == 0x14 && IsDragon(tiny)) {
			if (progressionQuest) {
				CallFunctionOn(progressionQuest, "Quest", "DevourDragon");
			}
		}
	}


	void VoreGrowth::VoreRegeneration(Actor* Caster) {
		float HpRegen = GetMaxAV(Caster, ActorValue::kHealth) *  0.00130;
		float SpRegen = GetMaxAV(Caster, ActorValue::kStamina) * 0.00130;

		if (Runtime::HasPerkTeam(Caster, "VorePerkRegeneration")) {
			Caster->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, HpRegen * TimeScale());
			Caster->AsActorValueOwner()->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kStamina, SpRegen * TimeScale());
		}
	}

	void VoreGrowth::VoreBuffAttributes() {
		auto Caster = GetCaster();
		if (!Caster) { // Don't apply bonuses if caster is not player.
			return;
		}
		auto Target = GetTarget();
		if (!Target) { // Don't apply bonuses if caster is not player.
			return;
		}
		if (Caster == Target) {
			return; // Don't apply to self
		}

		if (Runtime::HasPerk(Caster, "SoulVorePerk")) { // Permamently increases random AV after eating someone
			float TotalMod = (0.75 * get_visual_scale(Target));
			int Boost = rand() % 2;
			if (Boost == 0) {
				Caster->AsActorValueOwner()->ModActorValue(ActorValue::kHealth, TotalMod);
			} else if (Boost == 1) {
				Caster->AsActorValueOwner()->ModActorValue(ActorValue::kMagicka, TotalMod);
			} else if (Boost == 2) {
				Caster->AsActorValueOwner()->ModActorValue(ActorValue::kStamina, TotalMod);
			}
			//log::info("Buffing Attributes {}, Target: {}, Caster: {}", Boost, Target->GetDisplayFullName(), Caster->GetDisplayFullName());
		}
	}
}
