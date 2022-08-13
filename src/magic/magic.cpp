#include "magic/magic.h"
#include "util.h"
#include "magic/explosive_growth.h"
#include "magic/MagicManager.h"
#include "data/runtime.h"

namespace Gts {
	MagicManager& MagicManager::GetSingleton() noexcept {
		static MagicManager instance;
		return instance;
	}

	void MagicManager::poll() {
		auto actors = find_actors();
		auto& runtime = Runtime::GetSingleton();
		for (auto actor: actors) {
			auto effect_list =actor->GetActiveEffectList();
			if (!effect_list) continue;
			for (auto effect: (*effect_list)) {
				EffectSetting* base_spell = effect->GetBaseObject();
				Actor* caster = effect->caster.get().get();
				Actor* target = actor;
				float magnitude = effect->magnitude;
				float elapsedSeconds = effect->elapsedSeconds;
				float duration = effect->duration;

				if (base_spell == runtime.explosiveGrowth1) {
					explosive_growth1(caster);
				}
				else if (base_spell == runtime.explosiveGrowth2) {
					explosive_growth2(caster);
				}
				else if (base_spell == runtime.explosiveGrowth3) {
					explosive_growth3(caster);
				}
				if (base_spell == runtime.ShrinkPCButton) {
					ShrinkPCFunction(caster);
				}
				if (base_spell == runtime.GrowPcButton) {
					GrowPCFunction(caster);
				}

				if (base_spell == runtime.ShrinkEnemy) {
					ShrinkFoe(caster, target);
				}
				if (base_spell == runtime.ShrinkEnemyAOE) {
					ShrinkFoeAoe(caster, target);
				}
				if (base_spell == runtime.ShrinkEnemyAOEMast) {
					ShrinkFoeAoeMast(caster, target);
				}

				if (base_spell == runtime.SlowGrowth) {
					SlowGrowthFunction(caster);
				}

				if (base_spell == runtime.GrowthSpell) {
					GrowthSpellFunction(caster);
				}

				if (base_spell == runtime.ShrinkSpell) {
					ShrinkSpellFunction(caster);
				}

				if (base_spell == runtime.AllyGrowSize) {
					GrowAllyFunction(caster, target);
				}

				if (base_spell == runtime.GrowAllySizeButton) {
					GrowAllyButtonFunction(target);
				}

				if (base_spell == runtime.ShrinkAllySizeButton) {
					ShrinkAllyButtonFunction(target);
				}

				if (base_spell == runtime.ShrinkBack) {
					ShrinkBackFunction(caster);
				}

				if (base_spell == runtime.ShrinkBackNPC) {
					ShrinkBackNPCFunction(target); 
				}

				if (base_spell == runtime.GlobalVoreGrowth) {
					VoreGrowthFunction(caster); 
				}
			}
		}
	}
}
