	
#include "managers/animation/VoreHandler.hpp"
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

namespace Gts {
	VoreHandler& VoreHandler::GetSingleton() noexcept {
		static VoreHandler instance;
		return instance;
	}

	std::string VoreHandler::DebugName() {
		return "VoreHandler";
	}

	void VoreHandler::Update() {
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
    
    void VoreHandler::EatActor(Actor* giant, Actor* tiny) {
        for (auto &[giant, data]: VoreHandler::GetSingleton().data) {
            auto tiny = data.tiny;
            VoreHandler::GetSingleton().data.erase(giant);
        }
        ///Will do same stuff that the Scripts do here, mainly heal gainer and increase size, as well as other stuff i think.
        ///Would be nice to do stuff based on time passed, but that's probably too tedious to do (Since Script uses Utilit.wait(time) to do something based on delay)
    }

    void VoreHandler::GrabVoreActor(Actor* giant, Actor* tiny) {
        //Add Actor(s) to data so Update will manage it
        VoreHandler::GetSingleton().data.try_emplace(giant, tiny);
    }

    void VoreHandler::ClearData(Actor* giant) {
		VoreHandler::GetSingleton().data.erase(giant);
	}

    TESObjectREFR* VoreHandler::GetHeldVoreObj(Actor* giant) {
        try {
			auto& me = VoreHandler::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}
  	
	}

    Actor* VoreHandler::GetHeldVoreActors(Actor* giant) {
        //Return all Actors that we are currently Voring, to do things to them
        //Or maybe this function won't be needed since we send Actors from Vore.cpp?
       auto obj = VoreHandler::GetHeldVoreObj(giant);
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

