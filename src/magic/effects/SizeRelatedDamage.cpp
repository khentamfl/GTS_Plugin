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

	void SizeDamage::OnUpdate() {
		if (SizeManager::GetSingleton().GetPreciseDamage()) {
			return; 
		}
		auto caster = GetCaster();
		if (!caster) {
			return;
		} 
		auto target = GetTarget();
		if (!target) {
			return;
		}
		AccurateDamage::GetSingleton().DoSizeDamage(caster, target, 10.0, 1.0);
	}
}
