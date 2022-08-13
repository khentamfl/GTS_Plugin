#include "magic/explosive_growth.h"
#include "magic/magic.h"
#include "scale/scale.h"
#include "data/runtime.h"

namespace Gts {
	
	void explosive_growth1(Actor* caster) {
		auto& runtime = Runtime::GetSingleton();
		float one = 2.0;
		float two = 3.0;
		float three = 4.0;
		float GrowthTick = 120.0;
		float BonusGrowth = 1.0;
		float progression_multiplier = runtime.ProgressionMultiplier->value;

		
		//BSSoundHandle GrowthSound = BSSoundHandle::BSSoundHandle();
		//auto audio_manager = BSAudioManager::GetSingleton();
		//BSISoundDescriptor* sound_descriptor = find_form<BSISoundDescriptor>("GTS.esp|271EF6");
		//audio_manager->BuildSoundDataFromDescriptor(GrowthSound, sound_descriptor);
		
		float size_limit = runtime.sizeLimit->value;

		if (caster->HasPerk(runtime.ExtraGrowthMax)) {
		 one = 6.0;
		 two = 8.0;
		 three = 12.0;
		 BonusGrowth = 2.0;
		}
		else if (caster->HasPerk(runtime.ExtraGrowth)) {
		 one = 4.0;
		 two = 6.0;
		 three = 8.0;
		}
		
		
		float scale = get_visual_scale(caster);
		if (scale <= size_limit && scale < one) 
		{
		      mod_target_scale(caster, (0.00170 * progression_multiplier * BonusGrowth));	
		}
	}
	
	void explosive_growth2(Actor* caster) {
		auto& runtime = Runtime::GetSingleton();
		float one = 2.0;
		float two = 3.0;
		float three = 4.0;
		float GrowthTick = 120.0;
		float BonusGrowth = 1.0;
		float progression_multiplier = runtime.ProgressionMultiplier->value;

		
		//BSSoundHandle GrowthSound = BSSoundHandle::BSSoundHandle();
		//auto audio_manager = BSAudioManager::GetSingleton();
		//BSISoundDescriptor* sound_descriptor = find_form<BSISoundDescriptor>("GTS.esp|271EF6");
		//audio_manager->BuildSoundDataFromDescriptor(GrowthSound, sound_descriptor);
		
		float size_limit = runtime.sizeLimit->value;

		if (caster->HasPerk(runtime.ExtraGrowthMax)) {
		 one = 6.0;
		 two = 8.0;
		 three = 12.0;
		 BonusGrowth = 2.0;
		}
		else if (caster->HasPerk(runtime.ExtraGrowth)) {
		 one = 4.0;
		 two = 6.0;
		 three = 8.0;
		}
		
		float scale = get_visual_scale(caster);
		if (scale <= size_limit && scale < two) 
		{
		      mod_target_scale(caster, (0.00300 * progression_multiplier * BonusGrowth));	
		}
	}
	
	void explosive_growth3(Actor* caster) {
		auto& runtime = Runtime::GetSingleton();
		float one = 2.0;
		float two = 3.0;
		float three = 4.0;
		float GrowthTick = 120.0;
		float BonusGrowth = 1.0;
		float progression_multiplier = runtime.ProgressionMultiplier->value;

		
		//BSSoundHandle GrowthSound = BSSoundHandle::BSSoundHandle();
		//auto audio_manager = BSAudioManager::GetSingleton();
		//BSISoundDescriptor* sound_descriptor = find_form<BSISoundDescriptor>("GTS.esp|271EF6");
		//audio_manager->BuildSoundDataFromDescriptor(GrowthSound, sound_descriptor);
		
		float size_limit = runtime.sizeLimit->value;

		if (caster->HasPerk(runtime.ExtraGrowthMax)) {
		 one = 6.0;
		 two = 8.0;
		 three = 12.0;
		 BonusGrowth = 2.0;
		}
		else if (caster->HasPerk(runtime.ExtraGrowth)) {
		 one = 4.0;
		 two = 6.0;
		 three = 8.0;
		}
		
		float scale = get_visual_scale(caster);
		if (scale <= size_limit && scale < three) 
		{
		      mod_target_scale(caster, (0.00480 * progression_multiplier * BonusGrowth));	
		}
	}
}
