#include "managers/animation/Animation_Stomp.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
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
	const std::vector<std::string_view> Stomp_Triggers = { // Triggers Behavior_Stomp when matching event is sent
		"StompRight",
		"StompLeft",
	};

	const std::vector<std::string_view> Behavior_Stomp = { // Behavior triggers
		"GtsModStompAnimRight",     // [0]
		"GtsModStompAnimLeft",      // [1]
	};

	const std::vector<std::string_view> Anim_Stomp = { // Animation Events
		"GTSstompimpactR", 			// [0] stomp impacts, strongest effect
 		"GTSstompimpactL",          // [1]
		"GTSstomplandR", 			// [2] when landing after stomping, decreased power
 		"GTSstomplandL",            // [3]
 		"GTSstompstartR", 			// [4] For starting loop of camera shake and air rumble sounds
 		"GTSstompstartL",           // [5]
 		"GTSstompendR", 			// [6] disable loop of camera shake and air rumble sounds
 		"GTSstompendL",             // [7] 
		"GTS_Next",                 // [8] 
	};

	void ShakeAndSound(Actor* caster, Actor* receiver, float volume, const std::string_view& node) { // Applies camera shake and sounds
		Runtime::PlaySoundAtNode("lFootstepL", actor, volume, 1.0, node);
		auto bone = find_node(actor, node);
		if (bone) {
			NiAVObject* attach = bone;
			if (attach) {
				ApplyShakeAtNode(actor, actor, volume * 4, attach->world.translate);
			}
		}
	}
}


namespace Gts {
	Stomp& Stomp::GetSingleton() noexcept {
		static Stomp instance;
		return instance;
	}

	std::string Stomp::DebugName() {
		return "Stomp";
	}	

	void Stomp::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) { // Manages additional effects such as camera shake
		auto PC = PlayerCharacter::GetSingleton();
		auto transient = Transient::GetSingleton().GetActorData(PC);
		auto scale = get_visual_scale(actor);
		float volume = scale * 0.20;
        if (actor->formID == 0x14) {
            if (tag == Anim_Stomp[0]) {
				ShakeAndSound(actor, PC, volume, "NPC R Foot [Rft ]");
			} if (tag == Anim_Stomp[1]) {
				ShakeAndSound(actor, PC, volume, "NPC L Foot [Lft ]");
            } if (tag == Anim_Stomp[2]) {
				transient->rumblemult = 0.25;
				ShakeAndSound(actor, PC, volume * 0.5, "NPC R Foot [Rft ]");
            } if (tag == Anim_Stomp[3]) {
				ShakeAndSound(actor, PC, volume * 0.5, "NPC L Foot [Lft ]");
			} if (tag == Anim_Stomp[4] || tag == Anim_Stomp[5]) {
				transient->rumblemult = 0.0;
			} if (tag == Anim_Stomp[6] || tag == Anim_Stomp[7] || tag == Anim_Stomp[8]) {
				transient->rumblemult = 0.0;
			}
        }
	}

	void Stomp::ApplyStomp(Actor* actor, std::string_view condition) { // Calls Behavior events, triggering animation
		if (!actor) {
			return;
		}
		if (condition == Stomp_Triggers[0]) {
			actor->NotifyAnimationGraph(Behavior_Stomp[0]);
			return;
		}
		if (condition == Stomp_Triggers[1]) {
			actor->NotifyAnimationGraph(Behavior_Stomp[1]);
			return;
		}	
	}
}

