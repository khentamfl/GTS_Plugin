#include "magic/explosive_growth.h"
#include "magic/magic.h"
#include "scale/scale.h"

namespace Gts {
	
	void explosive_growth1(Actor* caster) {
		float one = 2.0;
		float two = 4.0;
		float three = 6.0;
		float GrowthTick = 120.0;

		BGSPerk* extra_growth = find_form<BGSPerk>("GTS.esp|332563");
		TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
		TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
		
		//BSSoundHandle GrowthSound = BSSoundHandle::BSSoundHandle();
		//auto audio_manager = BSAudioManager::GetSingleton();
		//BSISoundDescriptor* sound_descriptor = find_form<BSISoundDescriptor>("GTS.esp|271EF6");
		//audio_manager->BuildSoundDataFromDescriptor(GrowthSound, sound_descriptor);
		
		float progression_multiplier = progression_multiplier_global->value;
		float size_limit = SizeLimit->value;

		if (caster->HasPerk(extra_growth)) {
		 one = 4.0;
		 two = 8.0;
		 three = 12.0;
		}
		log::info("Explosive Growth.cpp initialized");
		
		float scale = get_visual_scale(caster);
		if (scale <= size_limit && scale < one) 
		{
		      mod_target_scale(caster, (0.00480 * progression_multiplier));	
		}
	}
	
	void explosive_growth2(Actor* caster) {
		float one = 2.0;
		float two = 4.0;
		float three = 6.0;
		float GrowthTick = 120.0;

		BGSPerk* extra_growth = find_form<BGSPerk>("GTS.esp|332563");
		TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
		TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
		
		float progression_multiplier = progression_multiplier_global->value;
		float size_limit = SizeLimit->value;

		if (caster->HasPerk(extra_growth)) {
		 one = 4.0;
		 two = 8.0;
		 three = 12.0;
		}
		log::info("Explosive Growth.cpp initialized");
		
		float scale = get_visual_scale(caster);
		if (scale <= size_limit && scale < two) 
		{
				mod_target_scale(caster, (0.00300 * progression_multiplier));
				log::info("Scaling Actor");	
		}
	}
	
	void explosive_growth3(Actor* caster) {
		float one = 2.0;
		float two = 4.0;
		float three = 6.0;
		float GrowthTick = 120.0;

		BGSPerk* extra_growth = find_form<BGSPerk>("GTS.esp|332563");
		TESGlobal* progression_multiplier_global = find_form<TESGlobal>("GTS.esp|37E46E");
		TESGlobal* SizeLimit = find_form<TESGlobal>("GTS.esp|2028B4");
		
		float progression_multiplier = progression_multiplier_global->value;
		float size_limit = SizeLimit->value;

		if (caster->HasPerk(extra_growth)) {
		 one = 4.0;
		 two = 8.0;
		 three = 12.0;
		}
		log::info("Explosive Growth.cpp initialized");
		
		float scale = get_visual_scale(caster);
		if (scale <= size_limit && scale < three) 
		{
				mod_target_scale(caster, (0.00175 * progression_multiplier));
				log::info("Scaling Actor");	
		}
	}
}
