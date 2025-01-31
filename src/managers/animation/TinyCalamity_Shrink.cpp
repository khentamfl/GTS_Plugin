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
    void AttachRune(Actor* giant, bool ShrinkRune, float speed, float scale) {
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
				float timepassed = std::clamp(((Finish - Start) * GetAnimationSlowdown(giantref)) * speed, 0.01f, 0.98f);
				if (node) {
					node->local.scale = std::clamp(1.0f - timepassed, 0.01f, 1.0f);
                    node->local.scale *= scale;
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
				float timepassed = std::clamp(((Finish - Start) * GetAnimationSlowdown(giantref)) * speed, 0.01f, 9999.0f);
				if (node) {
					node->local.scale = std::clamp(timepassed, 0.01f, 1.0f);
                    node->local.scale *= scale;
					update_node(node);
				}
				if (timepassed >= 1.0) {
					return false; // end it
				}
				return true;
			});
		}
	}

    void GTS_TC_RuneStart(AnimationEventData& data) {
        auto node = find_node(&data.giant, "GiantessRune", false);
        if (node) {
            node->local.scale = 0.01;
            update_node(node);
        }
        AttachRune(&data.giant, false, 0.8, 0.80);
    }
    void GTS_TC_ShrinkStart(AnimationEventData& data) {
        auto victim = Animation_TinyCalamity::GetShrinkActor(&data.giant);
        if (victim) {
            float until = Animation_TinyCalamity::GetShrinkUntil(&data.giant);
            ShrinkUntil(&data.giant, victim, until, 0.30, false);
            StartCombat(victim, &data.giant);
        }
    }
    void GTS_TC_ShrinkStop(AnimationEventData& data) {
        auto victim = Animation_TinyCalamity::GetShrinkActor(&data.giant);
        if (victim) {
            StaggerActor_Directional(&data.giant, 0.25, victim);
        }
    }
    void GTS_TC_RuneEnd(AnimationEventData& data) {
        AttachRune(&data.giant, true, 1.2, 0.80);
        Animation_TinyCalamity::GetSingleton().ResetActor(&data.giant);
    }
    // GTSBEH_TC_Shrink (Start it)
}

namespace Gts
{
    Animation_TinyCalamity& Animation_TinyCalamity::GetSingleton() noexcept {
		static Animation_TinyCalamity instance;
		return instance;
	}

	std::string Animation_TinyCalamity::DebugName() {
		return "Animation_TinyCalamity";
	}

	void Animation_TinyCalamity::RegisterEvents() {
		AnimationManager::RegisterEvent("GTS_TC_RuneStart", "Calamity", GTS_TC_RuneStart);
        AnimationManager::RegisterEvent("GTS_TC_ShrinkStart", "Calamity", GTS_TC_ShrinkStart);
        AnimationManager::RegisterEvent("GTS_TC_ShrinkStop", "Calamity", GTS_TC_ShrinkStop);
        AnimationManager::RegisterEvent("GTS_TC_RuneEnd", "Calamity", GTS_TC_RuneEnd);
	}

	void Animation_TinyCalamity::RegisterTriggers() {
		AnimationManager::RegisterTrigger("Calamity_ShrinkOther", "Calamity", "GTSBEH_TC_Shrink");
	}

    void Animation_TinyCalamity::Reset() {
		this->data.clear();
	}

	void Animation_TinyCalamity::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

    void Animation_TinyCalamity::AddToData(Actor* giant, Actor* tiny, float until) {
        Animation_TinyCalamity::GetSingleton().data.try_emplace(giant, tiny, until);
    }

	void Animation_TinyCalamity::Remove(Actor* giant) {
		Animation_TinyCalamity::GetSingleton().data.erase(giant);
	}

    Actor* Animation_TinyCalamity::GetShrinkActor(Actor* giant) {
		try {
			auto& me = Animation_TinyCalamity::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}
	}

    float Animation_TinyCalamity::GetShrinkUntil(Actor* giant) {
        try {
			auto& me = Animation_TinyCalamity::GetSingleton();
			return me.data.at(giant).until;
		} catch (std::out_of_range e) {
			return 1.0;
		}
    }

    CalamityData::CalamityData(Actor* tiny, float until) : tiny(tiny), until(until) {
	}
}