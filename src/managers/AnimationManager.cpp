#include "managers/AnimationManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/impact.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;
using namespace std;

namespace { 

	const std::vector<std::string_view> Anim_Stomp = {
		"GTSstompimpactR", 			// [0] stomp impacts, strongest effect
 		"GTSstompimpactL",          // [1]
		"GTSstomplandR", 			// [2] when landing after stomping, decreased power
 		"GTSstomplandL",            // [3]
 		"GTSstompstartR", 			// [4] For starting loop of camera shake, air rumble sounds
 		"GTSstompstartL",           // [5]
 		"GTSstompendR", 			// [6] disable loop of camera shake, air rumble sounds
 		"GTSstompendL",             // [7]
	};
	
	const std::vector<std::string_view> Anim_ThighCrush = {
		"GTStosit", 				// [0] start air rumble and camera shake
		"GTSsitloopenter", 			// [1] Sit down completed
		"GTSsitloopstart", 			// [2] Start to spread legs, call air rumble and camera shake. Enable feet damage and knockdown.
 		"GTSsitloopend", 			// [3] unused
		"GTSsitcrushstart",			// [4] Start strong air rumble and camera shake. Enable strong feet damage and knockdown
		"GTSsitcrushend", 			// [5] end it
		"GTSsitloopexit", 			// [6] stand up, small air rumble and camera shake
		"GTSstandR", 				// [7] feet collides with ground when standing up
		"GTSstandL",                // [8]
		"GTStoexit", 				// [9] Leave animation, disable air rumble and such
	};

	const std::vector<std::string_view> Anim_Compatibility = {
		"GTScrush_caster",          //[0] For compatibility with other mods. The gainer.
		"GTScrush_victim",          //[1] The one to crush
	};
}


namespace Gts {
	AnimationManager& AnimationManager::GetSingleton() noexcept {
		static AnimationManager instance;
		return instance;
	}

	std::string AnimationManager::DebugName() {
		return "AnimationManager";
	}

	void AnimationManager::Update() {
		auto PC = PlayerCharacter::GetSingleton();
		auto charCont = PC->GetCharController();
		auto transient = Transient::GetSingleton().GetActorData(actor);
		if (charCont) {
			PC->SetGraphVariableFloat("GiantessVelocity", (charCont->outVelocity.quad.m128_f32[2] * 100)/get_visual_scale(PC));
		}
		if (transient) {
			static Timer timer = Timer(0.20);
			if (timer.ShouldRunFrame()) {
				Runtime::PlaySoundAtNode("RumbleWalkSound", actor, volume, 1.0, "NPC L Foot [Lft]");
				Runtime::PlaySoundAtNode("RumbleWalkSound", actor, volume, 1.0, "NPC R Foot [Rft]");
			}
		}
	}

	void AnimationManager::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {
		auto PC = PlayerCharacter::GetSingleton();
        if (actor->formID == 0x14) {
			auto scale = get_visual_scale(actor);
			float volume = scale * 0.20;
            if (tag == Anim_Stomp[0] || tag == Anim_Stomp[1]) {
				//Call UnderFoot event here somehow with x scale bonus
				Runtime::PlaySound("lFootstepL", actor, volume, 1.0);
            } 
			if (tag == Anim_Stomp[2] || tag == Anim_Stomp[3]) {
				Runtime::PlaySound("lFootstepL", actor, volume * 0.5, 1.0);
            }
        }
		if (tag == Anim_Compatibility[0]) {
			//Actors[0] = actor; // caster
		}
		if (tag == Anim_Compatibility[1]) {
			float giantscale = get_visual_scale(PC);
			float tinyscale = get_visual_scale(actor);
			float sizedifference = giantscale/tinyscale;
			if (sizedifference >= 4.0) { 
				CrushManager::GetSingleton().Crush(PC, actor);
			}
			
		}
		//log::info("Actor: {}", actor->GetDisplayFullName());
    }

	void AnimationManager::GrabActor(Actor* giant, Actor* tiny, std::string_view findbone) {
		if (giant == tiny) {
			return;
		}
		auto bone = find_node(giant, findbone);
		if (!bone) {
			return;
		}
		float giantScale = get_visual_scale(giant);
		NiAVObject* attach = bone;
		NiPoint3 giantLocation = giant->GetPosition();
		NiPoint3 tinyLocation = tiny->GetPosition();
		if ((tinyLocation-giantLocation).Length() < 460*giantScale) {
			TESObjectREFR* ref = static_cast<TESObjectREFR*>(tiny);
			ref->SetPosition(attach->world.translate);
			tiny->SetPosition(attach->world.translate, false);
			auto charcont = tiny->GetCharController();
			if (charcont) {
				if (charcont->gravity > 0.0) {
					//log::info("Gravity of {} = {}", tiny->GetDisplayFullName(), charcont->gravity);
					//charcont->gravity = 0.0;
					charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0));
				}
			}
		}
	}
	void AnimationManager::Test(Actor * giant, Actor* tiny) {
		if (giant != tiny) {
			return;
		}
		float giantScale = get_visual_scale(giant);
		NiPoint3 giantLocation = giant->GetPosition();
		NiPoint3 tinyLocation = tiny->GetPosition();
		auto charCont = tiny->GetCharController();
		if ((tinyLocation-giantLocation).Length() < 460*giantScale) {
			if (charCont) {
				hkVector4 velocity;
				charCont->GetLinearVelocityImpl(velocity);
			//auto tinyai = tiny->GetActorRuntimeData().currentProcess->high;
			//if (tinyai) {
				log::info("{} OutVelicty = {}, Initial Vel: {}, Vel Total = {}", tiny->GetDisplayFullName(), Vector2Str(charCont->outVelocity), Vector2Str(charCont->initialVelocity), Vector2Str(velocity));
			//}
			}
		}
	}
}