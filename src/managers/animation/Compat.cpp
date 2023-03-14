// Animation: Compatibility
// Notes: Made avaliable for other generic anim mods
//  - Stages
//    - "GTScrush_caster",          //[0] The gainer.
//    - "GTScrush_victim",          //[1] The one to crush
// Notes: Modern Combat Overhaul compatibility
// - Stages
//   - "MCO_SecondDodge",           // enables GTS sounds and footstep effects
//   - "SoundPlay.MCO_DodgeSound",

#include "managers/animation/Compat.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/Rumble.hpp"
#include "managers/CrushManager.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";
	const std::string_view RSound = "rFootstepR";
	const std::string_view LSound = "lFootstepL";

	void GTScrush_caster(AnimationEventData& data) {
		data.stage = 0;
		log::info("GTScrush_caster");
	}

	void GTScrush_victim(AnimationEventData& data) {
		data.stage = 0;
		if (data.giant.formID != 0x14) {
      		Actor* player = PlayerCharacter::GetSingleton();
			float giantscale = get_visual_scale(player);
			float tinyscale = get_visual_scale(&data.giant);
			float sizedifference = giantscale/tinyscale;
			if (sizedifference >= 0.0) {
				CrushManager::Crush(player, &data.giant);
			}
		}
	}

	void MCO_SecondDodge(AnimationEventData& data) {
		data.stage = 0;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20;
		Runtime::PlaySound("lFootstepL", &data.giant, volume, 1.0);
	}
	void MCO_DodgeSound(AnimationEventData& data) {
		data.stage = 0;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20;
		Runtime::PlaySound("lFootstepL", &data.giant, volume, 1.0);
	}

}

namespace Gts
{
	void AnimationCompat::RegisterEvents() {
		AnimationManager::RegisterEvent("GTScrush_caster", "Compat1", GTScrush_caster);
		AnimationManager::RegisterEvent("GTScrush_victim", "Compat2", GTScrush_victim);
		AnimationManager::RegisterEvent("MCO_SecondDodge", "MCOCompat1", MCO_SecondDodge);
		AnimationManager::RegisterEvent("SoundPlay.MCO_DodgeSound", "MCOCompat2", MCO_DodgeSound);
	}

	void AnimationCompat::RegisterTriggers() {
	}
}
