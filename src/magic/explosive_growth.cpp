#include "magic/explosive_growth.h"
#include "magic/magic.h"
#include "scale/scale.h"

namespace Gts {
	void explosive_growth(Actor* target) {
		float one = 2.0;
		float two = 4.0;
		float three = 6.0;

		BGSPerk* extra_growth = find_form<BGSPerk>("GTS.esp|332563");
		TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
		float progression_multiplier = progression_multiplier_global->value;

		if (target.HasPerk(extra_growth)) {
			one = 4;
			two = 8;
			three = 12;
		}
		float scale = get_visual_scale(target);
		float size_limit = 256.0;
		if (scale <= size_limit) {
			EffectSetting* growth_three = find_form<EffectSetting>("GTS.esp|007928");
			if (target->HasMagicEffect(growth_three) && scale <= three) // Explosive Growth Part 3 (Strongest)
			{
				mod_target_scale(target, (0.00480 * progression_multiplier));
			}
		}
	}
}
