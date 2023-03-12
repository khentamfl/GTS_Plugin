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

	const std::vector<std::string_view> Anim_Vore = {
		"GTSvore_sitstart", 		// [0] Start air rumble and camera shake
		"GTSvore_sitend",           // [1] Sit down completed
		"GTSvore_handextend", 		// [2] Hand starts to move in space
		"GTSvore_handgrab",         // [3] Hand reached someone, grab actor
		"GTSvore_bringactorstart",  // [4] Hand brings someone to mouth
		"GTSvore_bringactorend",    // [5] Hand brought someone to mouth, release fingers
		"GTSvore_swallow",          // [6] Actor was swallowed by Giantess
		"GTSvore_swallow_sound",    // [7] Play gulp sound, eat actor completely (kill)
	};

	const std::vector<std::string_view> Anim_ThighSandwich = {
		"GTSsandwich_crouchstart",  // [0] Start air rumble and camera shake
		"GTSsandwich_grabactor",    // [1] Grab actor
		"GTSsandwich_crouchend",    // [2] Return to sit position
		"GTSsandwich_putactor",     // [3] Put actor on leg
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
			if (transient->legsspreading >= 1.0 || transient->legsclosing > 1.0 || transient->rumblemult >= 0.05) {
			for (auto nodes: LegRumbleNodes) {
				auto bone = find_node(caster, nodes);
				if (bone) {
					NiAVObject* attach = bone;
					if (attach) {
						float modifier = transient->legsspreading + transient->legsclosing + transient->rumblemult;
						volume = (4 * get_visual_scale(caster))/get_distance_to_camera(attach->world.translate);
						ApplyShakeAtNode(caster, receiver, 0.4 * modifier, attach->world.translate);
						volume *= modifier;
					}
				}
				if (timer.ShouldRunFrame()) {
					Runtime::PlaySoundAtNode("RumbleWalkSound", caster, volume, 1.0, nodes);
					}	
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

	void AnimationManager::UpdateActors(Actor* target) {
		auto PC = PlayerCharacter::GetSingleton();
		if (target == PC) {
			AdjustFallBehavior(PC);
		}
		ApplyRumbleSounds(target, PC);
	}

	void AnimationManager::AdjustAnimSpeed(Actor* actor, float bonus) {
		auto transient = Transient::GetSingleton().GetActorData(actor);
		if (transient) {
			bool AllowEdit = transient->Allowspeededit;
			if (AllowEdit) {
				transient->animspeedbonus += bonus;
				if (transient->animspeedbonus <= 0.15) {
					transient->animspeedbonus = 0.15;
				}
			} else if (!AllowEdit){
				transient->animspeedbonus = 1.0;
			}
			ConsoleLog::GetSingleton()->Print("Anim Speed of %s is %g", actor->GetDisplayFullName(), transient->animspeedbonus);
		}
	}
	

	void AnimationManager::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {
		auto PC = PlayerCharacter::GetSingleton();
		auto transient = Transient::GetSingleton().GetActorData(PC);
		auto scale = get_visual_scale(actor);
		if (actor->formID == 0x14 || Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
			//log::info("Actor: {}, tag: {}", actor->GetDisplayFullName(), tag);
		}
		float volume = scale * 0.20;
        if (tag == MCO[0] || tag == MCO[1]) {
			//Call UnderFoot event here somehow with x scale bonus
			Runtime::PlaySound("lFootstepL", actor, volume, 1.0);
        }
		if (tag == Anim_Compatibility[0]) {
			log::info("GTScrush_caster");
		}
		if (tag == Anim_Compatibility[1]) {
			float giantscale = get_visual_scale(PC);
			float tinyscale = get_visual_scale(actor);
			float sizedifference = giantscale/tinyscale;
			if (sizedifference >= 0.0) { 
				CrushManager::GetSingleton().Crush(PC, actor);
			}
		}
		//log::info("Actor: {}, tag: {}", actor->GetDisplayFullName(), tag);
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