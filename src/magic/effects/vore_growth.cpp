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
		if (Runtime::HasPerk(caster, "AdditionalGrowth")) {
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
		
	}


	void VoreGrowth::VoreRegeneration(Actor* Caster) {
		return;
	}

	void VoreGrowth::VoreBuffAttributes() {
		return;
	}
}
