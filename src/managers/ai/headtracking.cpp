#include "managers/animation/AnimationManager.hpp"
#include "managers/gamemode/GameModeManager.hpp"
#include "magic/effects/smallmassivethreat.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/RipClothManager.hpp"
#include "managers/ai/headtracking.hpp"
#include "managers/GtsSizeManager.hpp"
#include "scale/scalespellmanager.hpp"
#include "managers/InputManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/hitmanager.hpp"
#include "managers/highheel.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "utils/debug.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "Config.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <vector>
#include <string>



using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
    void RotateSpine(Actor* giant, Actor* tiny) {
		float GTSPitchOverride;
        float sizedifference = get_visual_scale(giant)/get_visual_scale(tiny);
        float modifier = 0.0;
        if (sizedifference > 1) {
            modifier = std::clamp(sizedifference*10, -0.0f, -240.0f);
        } else (
            modifier = std::clamp(sizedifference*5, 0.0f, 60.0f);
        )
		giant->GetGraphVariableFloat("GTSPitchOverride", GTSPitchOverride);
		giant->SetGraphVariableFloat("GTSPitchOverride", modifier);
	}

    void fixhtold(Actor* me) {
		float height = 127.0;
		float scale = get_visual_scale(me);

		auto ai = me->GetActorRuntimeData().currentProcess;
		bhkCharacterController* CharController = ai->GetCharController();
		if (CharController) {
			height = CharController->actorHeight * 70;
		}
		auto lookAt = me->GetLookingAtLocation();
		log::info("Look at of {} is {}", me->GetDisplayFullName(), Vector2Str(lookAt));
		auto head = me->GetPosition();
		log::info("Head of {} is {}", me->GetDisplayFullName(), Vector2Str(head));
		head.z += height * scale;
		log::info("Head + Scale + Height of {} is {}, bonus: {}", me->GetDisplayFullName(), Vector2Str(head), height * scale);

		NiPoint3 directionToLook = (lookAt - head);
		log::info("DirectionToLook of {} is {}", me->GetDisplayFullName(), Vector2Str(directionToLook));

		NiPoint3 myOneTimeHead = me->GetPosition();
		log::info("MyOneTimeHead of {} is {}", me->GetDisplayFullName(), Vector2Str(myOneTimeHead));
		myOneTimeHead.z += height;
		log::info("MyOneTimeHead + height of {} is {}", me->GetDisplayFullName(), Vector2Str(myOneTimeHead));
		

		NiPoint3 fakeLookAt = myOneTimeHead + directionToLook;
		fakeLookAt.z -= height * (scale - 1.0);
		log::info("{} is Looking at {}", me->GetDisplayFullName(), Vector2Str(fakeLookAt));

		ai->SetHeadtrackTarget(me, fakeLookAt);
		log::info("Set look of {} at {}", me->GetDisplayFullName(), Vector2Str(fakeLookAt));
	}
}

namespace Gts {

    Headtracking& Headtracking::GetSingleton() noexcept {
	    static Headtracking instance;

	    static std::atomic_bool initialized;
	    static std::latch latch(1);
	    if (!initialized.exchange(true)) {
		    latch.count_down();
	    }
	    latch.wait();

	    return instance;
    }

    std::string Headtracking::DebugName() {
	    return "Headtracking";
    }
	void Headtracking::FixHeadtracking(Actor* actor) {
        Profilers::Start("Headtracking: Headtracking Fix");
		auto ai = actor->GetActorRuntimeData().currentProcess;
		bhkCharacterController* CharController = ai->GetCharController();
		if (actor->formID != 0x14) {
			auto combat = actor->GetActorRuntimeData().combatController;
			auto target = ai->GetHeadtrackTarget().get().get();
			auto cast = skyrim_cast<Actor*>(target); 
			if (combat) {
				auto CombatTarget = combat->targetHandle.get().get();
				if (CombatTarget) {
                    float size_difference = get_visual_scale(actor)/get_visual_scale(CombatTarget);
					auto headnode = find_node(CombatTarget, "NPC Head [Head]");
					auto casternode = find_node(actor, "NPC Head [Head]");
					auto headlocation = headnode->world.translate;
					auto casterlocation = casternode->world.translate;
					NiPoint3 result = headlocation;
					result.z -= (casterlocation.z);
					actor->GetActorRuntimeData().currentProcess->SetHeadtrackTarget(actor, result);
                    RotateSpine(actor, CombatTarget);
				}
			} else if (cast) {
				float size_difference = get_visual_scale(actor)/get_visual_scale(cast);
				auto headnode = find_node(cast, "NPC Head [Head]");
				auto casternode = find_node(actor, "NPC Head [Head]");
				auto headlocation = headnode->world.translate;
				auto casterlocation = casternode->world.translate;
				NiPoint3 result = headlocation;
				result.z -= (casterlocation.z);
				actor->GetActorRuntimeData().currentProcess->SetHeadtrackTarget(actor, result);
                RotateSpine(actor, cast);
			}
		}
		Profilers::Stop("Headtracking: Headtracking Fix");
	}
}