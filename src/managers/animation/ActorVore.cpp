	
#include "managers/animation/ActorVore.hpp"
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

#include <random>

using namespace RE;
using namespace REL;
using namespace Gts;
using namespace std;

namespace {
    const std::vector<std::string_view> Anim_Vore = {
		"GTSvore_sit_start",         // [0] Start air rumble and camera shake
		"GTSvore_sit_end",           // [1] Sit down completed
		"GTSvore_hand_extend",       // [2] Hand starts to move in space
		"GTSvore_hand_grab",         // [3] Hand reached someone, grab actor
		"GTSvore_bringactor_start",  // [4] Hand brings someone to mouth
		"GTSvore_bringactor_end",    // [5] Hand brought someone to mouth, release fingers (may not be needed since we do stuff to actor based on AnimObjectA position?)
        "GTSvore_swallow",           // [6] Actor was swallowed by Giantess
		"GTSvore_swallow_sound",     // [7] Play gulp sound (May not be needed, probably just use GTSvore_Swallow instead)
        "GTSvore_hand_reposition"    // [8] Hand returns to original position (?)
        "GTSvore_standup_start"      // [9] Actor begins to stand up, eventually returning to original position      
        "GTSvore_eat_actor",         // [10] Actor reached specific depth inside Giantess, gain bonuses and eat(kill) actor completely.
        "GTSvore_standup_end",       // [11] Actor completely returned to original position: end animation, stop rumble and camera shake.
	};
}

namespace Gts {
	ActorVore& ActorVore::GetSingleton() noexcept {
		static ActorVore instance;
		return instance;
	}

	std::string ActorVore::DebugName() {
		return "ActorVore";
	}

	void ActorVore::Update() {
        //Attach actor to "AnimObjectA" node on Giant
        for (auto &[giant, data]: this->data) {
			if (!giant) {
				continue;
			}
			auto tiny = data.tiny;
			if (!tiny) {
				continue;
			}

			auto bone = find_node(giant, "AnimObjectA");
			if (!bone) {
				return;
			}

			float giantScale = get_visual_scale(giant);

			NiPoint3 giantLocation = giant->GetPosition();
			NiPoint3 tinyLocation = tiny->GetPosition();

			tiny->SetPosition(bone->world.translate);
			Actor* tiny_is_actor = skyrim_cast<Actor*>(tiny);
			if (tiny_is_actor) {
				auto charcont = tiny_is_actor->GetCharController();
				if (charcont) {
					charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
				}
			}
        }
    }
    
    void ActorVore::EatActor(Actor* giant, Actor* tiny) {
        for (auto &[giant, data]: ActorVore::GetSingleton().data) {
            auto tiny = data.tiny;
            ActorVore::GetSingleton().data.erase(giant);
        }
        ///Will do same stuff that the Scripts do here, mainly heal gainer and increase size, as well as other stuff i think.
        ///Would be nice to do stuff based on time passed, but that's probably too tedious to do (Since Script uses Utilit.wait(time) to do something based on delay)
    }

    void ActorVore::GrabVoreActor(Actor* giant, Actor* tiny) {
        //Add Actor(s) to data so Update will manage it
        ActorVore::GetSingleton().data.try_emplace(giant, tiny);
    }

    void ActorVore::Release(Actor* giant) {
		ActorVore::GetSingleton().data.erase(giant);
	}

    TESObjectREFR* ActorVore::GetHeldVoreObj(Actor* giant) {
        try {
			auto& me = ActorVore::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}
  	
	}

    Actor* ActorVore::GetHeldVoreActors(Actor* giant) {
        //Return all Actors that we are currently Voring, to do things to them
        //Or maybe this function won't be needed since we send Actors from Vore.cpp?
       auto obj = Grab::GetHeldVoreObj(giant);
    	Actor* actor = skyrim_cast<Actor*>(obj);
    	if (actor) {
      		return actor;
    	} else {
      		return nullptr;
    	}
	}

    VoreData::VoreData(TESObjectREFR* tiny) : tiny(tiny) {
	}
}

