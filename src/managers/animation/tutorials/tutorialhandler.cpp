/*#include "managers/animation/tutorials/tutorialhandler.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/GtsSizeManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "utils/findActor.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "spring.hpp"
#include "scale/scale.hpp"
#include "colliders/RE.hpp"
#include "colliders/actor.hpp"
#include "timer.hpp"
#include "node.hpp"
#include "utils/av.hpp"
#include "colliders/RE.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	void TutorialMessage(std::string_view message, std::string_view type) {
        bool AllowMessages = true; // Persistent::GetSingleton().AllowTutorials;
        if (!AllowMessages) {
            return;
        }
        bool HugSeen = true; //Persistent::GetSingleton().HugTutorialSeen;
        bool HugCrushSeen = true; //Persistent::GetSingleton().HugCrushTutorialSeen;
        bool CrushSeen = true; //Persistent::GetSingleton().CrushTutorialSeen;
        bool GrabSeen = true; //Persistent::GetSingleton().GrabTutorialSeen;
        bool CalamitySeen = true; //Persistent::GetSingleton().CalamityTutorialSeen;
        bool GrowthSpurtSeen = true; //Persistent::GetSingleton().GrowthTutorialSeen;
        bool BreastsSeen = true; //Persistent::GetSingleton().BreastTutorialSeen;
        bool SandwichStartSeen = true; //Persistent::GetSingleton().SandwichStartTutorialSeen;
        bool SandwichControlSeen = true; //Persistent::GetSingleton().SandwichControlTutorialSeen;
        bool VoreSeen = true; //Persistent::GetSingleton().VoreTutorialSeen;
        if (type == "Hugs") { 
            MessageBox(message);
            HugSeen = true;
        } else if (type == "HugCrush") {
            MessageBox(message);
            HugCrushSeen = true;
        } else if (type == "Crush") {
            MessageBox(message);
            CrushSeen = true;
        } else if (type == "Grab") {
            MessageBox(message);
            GrabSeen = true;
        } else if (type == "Calamity") {
            MessageBox(message);
            CalamitySeen = true;
        } else if (type == "GrowthSpurt") {
            MessageBox(message);
            GrowthSpurtSeen = true;
        } else if (type == "Breasts") {
            MessageBox(message);
            BreastsSeen = true;
        } else if (type == "SandwichStart") {
            MessageBox(message);
            SandwichControlSeen = true;
        } else if (type == "SandwichControls") {
            MessageBox(message);
            SandwichControlSeen = true;
        } else if (type == "Vore") {
            MessageBox(message);
        }
	}

    void CheckTutorialTargets(Actor* actor) {
        if (actor->formID != 0x14) {
            return;
        }
        NiPoint3 giantLocation = actor->GetPosition();
        float CrushRatio = 8.0;
        float SandwichRatio = 6.0;
        float VoreRatio = 7.2;
        if (Runtime::HasPerk(actor, "VorePerk")) {
            VoreRatio = 6.0;
        }
        float BASE_CHECK_DISTANCE = 256.0;
        for (auto otherActor: find_actors()) {
            if (otherActor != actor) {
                float tinyScale = get_visual_scale(otherActor);
                float giantScale = get_visual_scale(actor);

                if (giantScale / tinyScale > CrushRatio) {
                    NiPoint3 actorLocation = otherActor->GetPosition();
                    if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
                        std::string message = std::format("When the size difference between you and your target is greater than eight times, you're able to crush your target into nothing and receive extra size-related experience");
                        TutorialMessage(message, "Crush");
                        return;
                    }
                } else if (giantScale / tinyScale > SandwichRatio) {
                    NiPoint3 actorLocation = otherActor->GetPosition();
                    if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
                        std::string message = std::format("When the size difference is greater than six times, you're able to perform Grab and Thigh Sandwich attacks. You're able to perform Thigh Sandwich/Grab attacks on {}. Press L.Shift + C for Sandwich, or hold F to grab {}. ", otherActor->GetDisplayFullName(), otherActor->GetDisplayFullName());
                        TutorialMessage(message, "SandwichStart");
                        return;
                    }
                } else if (giantScale / tinyScale > VoreRatio) {
                    NiPoint3 actorLocation = otherActor->GetPosition();
                    if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
                        std::string message = std::format("You can eat {} by pressing L.Shift + V. You can eat other actors When the size difference is greater than 7.2 times (or 6.0 times with 'Vore' perk)", otherActor->GetDisplayFullName());
                        TutorialMessage(message, "Vore");
                        return;
                    }
                } 
            }
        }
    }
}*/