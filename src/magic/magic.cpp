#include "magic/magic.h"
#include "util.h"
#include "magic/explosive_growth.h"

namespace Gts {
	MagicManager& MagicManager::GetSingleton() noexcept {
		static MagicManager instance;
		return instance;
	}

	void MagicManager::poll() {
		auto actors = find_actors();
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

				if (base_spell == find_form<EffectSetting>("GTS.esp|01234")) {
					explosive_growth(caster);
				}
			}
		}
	}
}
