#include "magic/effects/CrushGrowth.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "util.hpp"


namespace Gts {
	std::string CrushGrowth::GetName() {
		return "CrushGrowth";
	}

	bool CrushGrowth::StartEffect(EffectSetting* effect) { // NOLINT
		auto& runtime = Runtime::GetSingleton();
		return effect == runtime.CrushGrowthMGEF;
	}

	void CrushGrowth::OnStart() {
		auto sizemanager = SizeManager::GetSingleton();
		auto target = GetTarget();
		auto caster = GetCaster();
		
		if (caster != target) {
			this->magnitude = GetActiveEffect()->magnitude;
			float CrushStacks = this->magnitude;
			sizemanager.SetCrushGrowthStacks(caster, CrushStacks);
			this->ScaleOnCrush = get_target_scale(target);
			log::info("Crush Receiver: {}, victim: {}, amount: {}", caster->GetDisplayFullName(), target->GetDisplayFullName(), sizemanager.GetCrushGrowthStacks(caster));
		}
	}

	void CrushGrowth::OnUpdate() {
		auto& runtime = Runtime::GetSingleton();
		auto sizemanager = SizeManager::GetSingleton();
		auto caster = GetCaster();
        auto target = GetTarget();
		auto player = PlayerCharacter::GetSingleton();

		if (!caster) {
			return;
		}
		if (!target) {
			return;
		}
        float GrowAmount = clamp(1.0, 1000.0, sizemanager.GetCrushGrowthStacks(caster));
        float Rate = 0.00050 * GrowAmount * this->ScaleOnCrush;
        if (player->HasPerk(runtime.AdditionalAbsorption)) {
			Rate *= 2.0;
		}
		
        CrushGrow(caster, Rate, 0);
	}


    void CrushGrowth::OnFinish() {
		auto& runtime = Runtime::GetSingleton();
        auto sizemanager = SizeManager::GetSingleton();
		auto target = GetTarget();
		auto caster = GetCaster();
    }
}