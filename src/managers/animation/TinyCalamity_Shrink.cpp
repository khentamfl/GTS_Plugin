#include "managers/animation/TinyCalamity_Shrink.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "rays/raycast.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

// Spawns Rune on the hand and then we shrink a tiny that we've found.
// AnimObjectR

namespace {
    /*void AttachRune(Actor* giant, bool ShrinkRune, float speed, float scale) {
		string node_name = "GiantessRune";

        float Start = Time::WorldTimeElapsed();
        std::string name = std::format("Calamity_{}_{}", giant->formID, ShrinkRune);
        ActorHandle gianthandle = giant->CreateRefHandle();

		if (ShrinkRune) {
			
			TaskManager::Run(name, [=](auto& progressData) {
				if (!gianthandle) {
					return false;
				}
				auto giantref = gianthandle.get().get();
                float Finish = Time::WorldTimeElapsed();
				auto node = find_node(giantref, node_name, false);
				float timepassed = std::clamp(((Finish - Start) * GetAnimationSlowdown(giantref)), 0.01f, 0.98f);
				if (node) {
					node->local.scale = std::clamp(1.0f - timepassed, 0.01f, 1.0f);
					update_node(node);
				}
				if (timepassed >= 0.98) {
					return false; // end it
				}
				return true;
			});
		} else {
			TaskManager::Run(name, [=](auto& progressData) {
				if (!gianthandle) {
					return false;
				}
				auto giantref = gianthandle.get().get();
                float Finish = Time::WorldTimeElapsed();
				auto node = find_node(giantref, node_name, false);
				float timepassed = std::clamp(((Finish - Start) * GetAnimationSlowdown(giantref)), 0.01f, 9999.0f);
				if (node) {
					node->local.scale = std::clamp(timepassed, 0.01f, 1.0f);
					update_node(node);
				}
				if (timepassed >= 1.0) {
					return false; // end it
				}
				return true;
			});
		}
	}*/
}

namespace Gts
{
    /*Animation_TinyCalamity& Animation_TinyCalamity::GetSingleton() noexcept {
		static Animation_TinyCalamity instance;
		return instance;
	}

	std::string Animation_TinyCalamity::DebugName() {
		return "Animation_TinyCalamity";
	}

	void Animation_TinyCalamity::RegisterEvents() {
		AnimationManager::RegisterEvent("GTS_Crawl_Knee_Trans_Impact", "Crawl", GTS_Crawl_Knee_Trans_Impact);
	}

	void Animation_TinyCalamity::RegisterTriggers() {
		AnimationManager::RegisterTrigger("SwipeLight_Left", "Crawl", "GTSBeh_SwipeLight_L");
	}

    void Animation_TinyCalamity::Reset() {
		this->data.clear();
	}

	void Animation_TinyCalamity::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

    void Animation_TinyCalamity::AddToData(Actor* giant, TESObjectREFR* tiny) {
        Animation_TinyCalamity::GetSingleton().data.try_emplace(giant, tiny);
    }

	void Animation_TinyCalamity::Remove(Actor* giant) {
		Animation_TinyCalamity::GetSingleton().data.erase(giant);
	}

    Actor* Animation_TinyCalamity::GetHeldObj(Actor* giant) {
		try {
			auto& me = Animation_TinyCalamity::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}

	}

    CalamityData::CalamityData(Actor* tiny) : tiny(tiny) {
	}*/
}