#include "magic/explosive_growth.h"
#include "magic/magic.h"
#include "scale/scale.h"

namespace Gts {
	void explosive_growth(Actor* target) {
		float one = 2.0;
		float two = 4.0;
		float three = 6.0;
		float GrowthTick = 120.0;

		BGSPerk* extra_growth = find_form<BGSPerk>("GTS.esp|332563");
		TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
		TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
		
		BSSoundHandle GrowthSound = BSSoundHandle::BSSoundHandle();
		auto audio_manager = BSAudioManager::GetSingleton();
		BSISoundDescriptor* sound_descriptor = find_form<BSISoundDescriptor>("GTS.esp|271EF6");
		audio_manager->BuildSoundDataFromDescriptor(GrowthSound, sound_descriptor);
		
		float progression_multiplier = progression_multiplier_global->value;
		float size_limit = SizeLimit->value;

		if (target->HasPerk(extra_growth)) {
		 one = 4;
		 two = 8;
		 three = 12;
		}
		
		float scale = get_visual_scale(target);
		if (scale <= size_limit) 
		{
			GrowthTick -= 1.0;
			EffectSetting* growth3 = find_form<EffectSetting>("GTS.esp|007928"); // 3
			EffectSetting* growth2 = find_form<EffectSetting>("GTS.esp|1E42A5"); // 2
			EffectSetting* growth1 = find_form<EffectSetting>("GTS.esp|1E42A6"); // 1
			if (target->HasMagicEffect(growth3) && scale <= three) // Explosive Growth Part 3 (Strongest)
			{
				mod_target_scale(target, (0.00480 * progression_multiplier));
			}
			else if (target->HasMagicEffect(growth2) && scale <= two) // Explosive Growth Part 2 (Medium)
			{
				mod_target_scale(target, (0.00300 * progression_multiplier));
			}
			else if (target->HasMagicEffect(growth1) && scale <= one) // Explosive Growth Part 1 (Small)
			{
				mod_target_scale(target, (0.00175 * progression_multiplier));
			}
			for (Actor* target)
			{
				if (GrowthTick == 120.0)
				{GrowthSound.Play();GrowthTick = 0.0;}
			}
			
		}
	}
}
