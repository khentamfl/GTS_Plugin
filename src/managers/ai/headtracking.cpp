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
        float sizedifference = get_visual_scale(giant)/get_visual_scale(tiny);
        float modifier = 0.0;
        if (sizedifference > 1) {
            modifier = std::clamp(sizedifference*10, -0.0f, -240.0f);
        } else {
            modifier = std::clamp(sizedifference*5, 0.0f, 60.0f);
        }
		giant->SetGraphVariableFloat("GTSPitchOverride", modifier);
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

	void Headtracking::FixHeadtracking(Actor* me) {
        Profilers::Start("Headtracking: Headtracking Fix");
        float height = 127.0;
        float scale = get_visual_scale(me);

        auto ai = me->GetActorRuntimeData().currentProcess;
        bhkCharacterController* CharController = ai->GetCharController();
        if (CharController) {
            height = CharController->actorHeight * 70;
        }
        auto targetObj = ai->GetHeadtrackTarget().get().get();
        if (targetObj) {
            auto targetHeight = 0.0f;
            auto target = skyrim_cast<Actor*>(targetObj);
            if (target) {
                auto targetScale = get_visual_scale(target);
                auto targetChar = target->GetCharController();
                RotateSpine(me, target);
                if (targetChar) {
                    targetHeight = targetChar->actorHeight * 70.0 * targetScale;
                }
            }
            auto lookAt = targetObj.GetPosition();
            lookAt.z += targetHeight;
            auto head = me->GetPosition();
            head.z += height * scale;

            NiPoint3 directionToLook = (lookAt - head);

            NiPoint3 myOneTimeHead = me->GetPosition();
            myOneTimeHead.z += height;
                    

            NiPoint3 fakeLookAt = myOneTimeHead + directionToLook;
            fakeLookAt.z -= height * (scale - 1.0);

            ai->SetHeadtrackTarget(me, fakeLookAt);
            Profilers::Stop("Headtracking: Headtracking Fix");
            }
        }
    }
}
       
