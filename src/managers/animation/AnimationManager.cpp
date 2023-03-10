#include "managers/animation/AnimationManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/impact.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
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
 		"GTSstompstartR", 			// [4] For starting loop of camera shake and air rumble sounds
 		"GTSstompstartL",           // [5]
 		"GTSstompendR", 			// [6] disable loop of camera shake and air rumble sounds
 		"GTSstompendL",             // [7]
	};
	
	const std::vector<std::string_view> Anim_ThighCrush = {
		"GTStosit", 				// [0] Start air rumble and camera shake
		"GTSsitloopenter", 			// [1] Sit down completed
		"GTSsitloopstart", 			// [2] Start to spread legs, call air rumble and camera shake. Enable feet damage and knockdown.
 		"GTSsitloopend", 			// [3] unused
		"GTSsitcrushstart",			// [4] Start crush loop: strong air rumble and camera shake. Enable strong feet damage and knockdown
		"GTSsitcrushend", 			// [5] end crush loop
		"GTSsitloopexit", 			// [6] stand up, small air rumble and camera shake
		"GTSstandR", 				// [7] feet collides with ground when standing up
		"GTSstandL",                // [8]
		"GTSstandRS",               // [9] Silent impact of right feet
		"GTStoexit", 				// [10] Leave animation, disable air rumble and such
	};

	const std::vector<std::string_view> Anim_Vore = {
		"GTSvore_sitstart", 		// [0] Start air rumble and camera shake
		"GTSvore_sitend",           // [1] Sit down completed
		"GTSvore_handextend", 		// [2] Hand starts to move in space
		"GTSvore_handgrab",         // [3] Hand reached someone, grab actor
		"GTSvore_handbringstart",   // [4] Hand brings someone to mouth
		"GTSvore_handbringend",     // [5] Hand brought someone to mouth, release fingers
		"GTSvore_handswallow",      // [6] Actor was swallowed by Giantess
		"GTSvore_hand_wallow_sound",// [7] Play gulp sound, eat actor completely (kill)
	};

	const std::vector<std::string_view> Anim_ThighSandwich = {
		"GTSsandwich_crouchstart",  // [0] Start air rumble and camera shake
		"GTSsandwich_grabactor",    // [1] Grab actor
		"GTSsandwich_crouchend",    // [2] Return to sit position, put actor on leg
		"GTSsandwich_enterloop", 	// [3] Start idle loop with someone between legs
		"GTSsandwich_sandwichstart",// [4] Sandwich someone between legs, dealing damage and playing sound 
		"GTSsandwich_sandwichhit",  // [5] Apply damage and sound
		"GTSsandwich_sandwichend",  // [6] Leg returns to idle position
		"GTSsandwich_exit",         // [7] Exit sandwich loop and leave animation in general
	};

	const std::vector<std::string_view> Anim_Compatibility = {
		"GTScrush_caster",          //[0] For compatibility with other mods. The gainer.
		"GTScrush_victim",          //[1] The one to crush
	};

	const std::vector<std::string_view> LegRumbleNodes = { // used with Anim_ThighCrush
		"NPC L Foot [Lft ]",
		"NPC R Foot [Rft ]",
		"NPC L Toe0 [LToe]",
		"NPC R Toe0 [RToe]",
		"NPC L Calf [LClf]",
		"NPC R Calf [RClf]",
		"NPC L FrontThigh",
		"NPC R FrontThigh",
		"NPC R RearCalf [RrClf]",
		"NPC L RearCalf [RrClf]",
	};

	const std::vector<std::string_view> MCO = {
		"MCO_SecondDodge",                       // MCO compatibility, enables GTS sounds and footstep effects
		"SoundPlay.MCO_DodgeSound",
	};

	

	void AdjustFallBehavior(Actor* actor) {
		auto charCont = actor->GetCharController();
		if (charCont) {
			actor->SetGraphVariableFloat("GiantessVelocity", (charCont->outVelocity.quad.m128_f32[2] * 100)/get_visual_scale(actor));
		}
	}

	void ApplyRumbleSounds(Actor* caster, Actor* receiver) {
		auto transient = Transient::GetSingleton().GetActorData(caster);
		float volume = 0.0;	
		static Timer timer = Timer(0.40);
		if (transient) {
			//if (transient->legsspreading > = 1.0 || transient->legsclosing > 1.0) {
			for (auto nodes: LegRumbleNodes) {
				auto bone = find_node(caster, nodes);
				if (bone) {
					NiAVObject* attach = bone;
					if (attach) {
						volume = (15 * get_visual_scale(caster))/get_distance_to_camera(attach->world.translate);
						ApplyShakeAtNode(caster, receiver, 0.4, attach->world.translate);
						//volume *= transient->legsspreading + transient->legsclosing;
					}
				}
				if (timer.ShouldRunFrame()) {
					Runtime::PlaySoundAtNode("RumbleWalkSound", caster, volume, 1.0, nodes);
				}	
			}
		}
	}
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
		AdjustFallBehavior(PC);
		//ApplyRumbleSounds(PC, PC);
	}
	

	void AnimationManager::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {
		auto PC = PlayerCharacter::GetSingleton();
        if (actor->formID == 0x14) {
			auto scale = get_visual_scale(actor);
			float volume = scale * 0.20;
            if (tag == Anim_Stomp[0] || tag == Anim_Stomp[1] || tag == MCO[0] || tag == MCO[1]) {
				//Call UnderFoot event here somehow with x scale bonus
				Runtime::PlaySound("lFootstepL", actor, volume, 1.0);
            } 
			if (tag == Anim_Stomp[2] || tag == Anim_Stomp[3]) {
				Runtime::PlaySound("lFootstepL", actor, volume * 0.5, 1.0);
            }
        }
		if (tag == Anim_Compatibility[0]) {
			//Actors[0] = actor; // caster
			log::info("GTScrush_caster");
		}
		if (tag == Anim_Compatibility[1]) {
			log::info("GTScrush_victim, Trying to crush: {}", actor->GetDisplayFullName());
			float giantscale = get_visual_scale(PC);
			float tinyscale = get_visual_scale(actor);
			float sizedifference = giantscale/tinyscale;
			if (sizedifference >= 0.0) { 
				CrushManager::GetSingleton().Crush(PC, actor);
			}
			
		}
		log::info("Actor: {}, tag: {}", actor->GetDisplayFullName(), tag);
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