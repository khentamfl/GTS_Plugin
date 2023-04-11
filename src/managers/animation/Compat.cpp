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
#include "managers/damage/AccurateDamage.hpp"
#include "managers/GtsSizeManager.hpp"
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

	void TriggerKillZone(Actor* giant) {
		if (!giant) {
			return;
		}
		float BASE_CHECK_DISTANCE = 90.0;
		float SCALE_RATIO = 0.0;
		float giantScale = get_visual_scale(giant);
		NiPoint3 giantLocation = giant->GetPosition();
		for (auto otherActor: find_actors()) {
		if (otherActor != giant) {
			if (otherActor->IsInKillMove()) {
				float tinyScale = get_visual_scale(otherActor); 
				if (giantScale / tinyScale > SCALE_RATIO) {
					NiPoint3 actorLocation = otherActor->GetPosition();
					if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
							CrushManager::Crush(giant, otherActor);
						}
					}
				}
			}
		}
	}


	void GTScrush_caster(AnimationEventData& data) {
		//data.stage = 0;
		log::info("GTScrush_caster");
		TriggerKillZone(&data.giant);
	}

	void GTScrush_victim(AnimationEventData& data) {
		//data.stage = 0;
		if (data.giant.formID != 0x14) {
			auto player = PlayerCharacter::GetSingleton();
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
		DoDamageEffect(&data.giant, 1.20, 1.4, 10, 0.25);
		DoSizeEffect(&data.giant, 1.10, FootEvent::Right, RNode);
		DoSizeEffect(&data.giant, 1.10, FootEvent::Left, LNode);
	}
	void MCO_DodgeSound(AnimationEventData& data) {
		data.stage = 0;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20;
		DoDamageEffect(&data.giant, 1.25, 1.4, 10, 0.25);
		DoSizeEffect(&data.giant, 1.10, FootEvent::Right, RNode);
		DoSizeEffect(&data.giant, 1.10, FootEvent::Left, LNode);
	}

	void JumpDown(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& sizemanager = SizeManager::GetSingleton(); 
		float damage = sizemanager.GetSizeAttribute(giant, 2) * 2.0;
		DoDamageEffect(&data.giant, 2.2 * damage, 8.0, 20, 0.45);
	}
}

namespace Gts
{
	void AnimationCompat::RegisterEvents() {
		AnimationManager::RegisterEvent("GTScrush_caster", "Compat1", GTScrush_caster);
		AnimationManager::RegisterEvent("GTScrush_victim", "Compat2", GTScrush_victim);
		AnimationManager::RegisterEvent("MCO_SecondDodge", "MCOCompat1", MCO_SecondDodge);
		AnimationManager::RegisterEvent("SoundPlay.MCO_DodgeSound", "MCOCompat2", MCO_DodgeSound);
		AnimationManager::RegisterEvent("JumpDown", "JumpCompat1", JumpDown);
	}

	void AnimationCompat::RegisterTriggers() {
	}
}
