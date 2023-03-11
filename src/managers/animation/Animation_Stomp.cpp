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

	const std::vector<std::string_view> Behaviors = {
		"GtsModStompAnimRight",     // [0]
		"GtsModStompAnimLeft",      // [1]
	};

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


namespace Gts {
	Stomp& Stomp::GetSingleton() noexcept {
		static Stomp instance;
		return instance;
	}

	std::string Stomp::DebugName() {
		return "Stomp";
	}	

	void Stomp::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {
		auto PC = PlayerCharacter::GetSingleton();
		auto transient = Transient::GetSingleton().GetActorData(PC);
		auto scale = get_visual_scale(actor);
		float volume = scale * 0.20;
        if (actor->formID == 0x14) {
            if (tag == Anim_Stomp[0] || tag == Anim_Stomp[1]) {
				//Call UnderFoot event here somehow with x scale bonus
				Runtime::PlaySound("lFootstepL", actor, volume, 1.0);
            } if (tag == Anim_Stomp[2] || tag == Anim_Stomp[3]) {
				Runtime::PlaySound("lFootstepL", actor, volume * 0.5, 1.0);
            } if (tag == Anim_Stomp[4] || tag == Anim_Stomp[5]) {
				transient->rumblemult = 0.25;
			} if (tag == Anim_Stomp[6] || tag == Anim_Stomp[7]) {
				transient->rumblemult = 0.0;
			}
        }
	}

	void Stomp::ApplyStomp(Actor* actor, std::string_view condition) {
		if (condition == Behaviors[0])
			actor->NotifyAnimationGraph(Behaviors[0]);
		else if (condition == Behaviors[1]) {
			actor->NotifyAnimationGraph(Behaviors[1]);
		}	
	}
}

