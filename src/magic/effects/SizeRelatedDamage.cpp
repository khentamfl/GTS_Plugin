#include "magic/effects/SizeRelatedDamage.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/highheel.hpp"
#include "magic/effects/common.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "data/transient.hpp"
#include "data/persistent.hpp"
#include "managers/Attributes.hpp"
#include "managers/CrushManager.hpp"
#include "timer.hpp"
#include "node.hpp"


namespace Gts {
	std::string SizeDamage::GetName() {
		return "SizeDamage";
	}

	void SizeDamage::OnUpdate() { // This is used when actors have Size effect on them from magic effect, and when Precise Damage is off.
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
		if (SizeManager::GetSingleton().GetPreciseDamage() && !Runtime::HasMagicEffect(caster, "SmallMassiveThreat")) { // This method damages actors once per 0.10 sec because of Papyrus being 'amazing'.
			return;
		}
		float castersize = get_visual_scale(caster);
		float targetsize = get_visual_scale(target);
		float sizedifference = castersize/targetsize;
		if (Runtime::HasMagicEffect(caster, "SmallMassiveThreat")) {
			sizedifference += 4.0;
		}

		if (sizedifference >= 1.5) {
			AccurateDamage::GetSingleton().DoSizeDamage(caster, target, 0, 0, false);
		}
	}
}
