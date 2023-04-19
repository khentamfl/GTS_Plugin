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
  NiPoint3 HeadLocation(TESObjectREFR& obj, const float& scale) {
    NiPoint3 headOffset(0.0, 0.0, 0.0);
    auto location = obj.GetPosition();
    auto asActor = skyrim_cast<Actor*>(&obj);
    if (asActor) {
      auto charCont = asActor->GetCharController();
      if (charCont) {
        headOffset.z = charCont->actorHeight * 70.0 * scale;
      }
    }
    return location + headOffset;
  }
  NiPoint3 HeadLocation(TESObjectREFR& obj) {
    float scale = 1.0;
    auto asActor = skyrim_cast<Actor*>(&obj);
    if (asActor) {
      scale = get_visual_scale(asActor);
    }
    return HeadLocation(obj, scale);
  }
  NiPoint3 HeadLocation(TESObjectREFR* obj, const float& scale) {
    if (!obj) {
      return NiPoint3();
    } else {
      return HeadLocation(*obj, scale);
    }
  }
  NiPoint3 HeadLocation(TESObjectREFR* obj) {
    if (!obj) {
      return NiPoint3();
    } else {
      return HeadLocation(*obj);
    }
  }
  NiPoint3 HeadLocation(ActorHandle objRefr, const float& scale) {
    if (!objRefr) {
      return NiPoint3();
    } else {
      auto obj = objRefr.get().get();
      if (!obj) {
        return NiPoint3();
      }
      return HeadLocation(*obj, scale);
    }
  }
  NiPoint3 HeadLocation(ActorHandle objRefr) {
    if (!objRefr) {
      return NiPoint3();
    } else {
      auto obj = objRefr.get().get();
      if (!obj) {
        return NiPoint3();
      }
      return HeadLocation(*obj);
    }
  }

  NiPoint3 HeadLocation(ObjectRefHandle objRefr, const float& scale) {
    if (!objRefr) {
      return NiPoint3();
    } else {
      auto obj = objRefr.get().get();
      if (!obj) {
        return NiPoint3();
      }
      return HeadLocation(*obj, scale);
    }
  }
  NiPoint3 HeadLocation(ObjectRefHandle objRefr) {
    if (!objRefr) {
      return NiPoint3();
    } else {
      auto obj = objRefr.get().get();
      if (!obj) {
        return NiPoint3();
      }
      return HeadLocation(*obj);
    }
  }
	void SpellTest(Actor* caster) {
		//auto Projectile = caster->GetActorRuntimeData().currentProcess->high->muzzleFlash->projectile3D.get();
		auto node = find_node(caster, "AbsorbBeam01");
		if (node) {
			node->local.scale = get_visual_scale(caster);
		}
		/*if (Projectile) {
		    Projectile->world.scale = get_visual_scale(caster);
		    Projectile->local.scale = get_visual_scale(caster);
		    update_node(Projectile);
		   }*/
	}

  // Rotate spine to look at an actor either leaning back or looking down
	void RotateSpine(Actor* giant, Actor* tiny, HeadtrackingData& data) {
    const float REDUCTION_FACTOR = 0.666;
		bool Collision_Installed = false; //Used to detect 'Precision' mod
		float Collision_PitchMult = 0.0;
		giant->GetGraphVariableBool("Collision_Installed", Collision_Installed);
		if (Collision_Installed == true) {
			giant->GetGraphVariableFloat("Collision_PitchMult", Collision_PitchMult); // If true, obtain value to apply it
			//giant->SetGraphVariableFloat("Collision_PitchMult", 0.0);
			//log::info("Callision Pitch Mult: {}", Collision_PitchMult);
		}
    float finalAngle = 0.0;

		auto dialoguetarget = giant->GetActorRuntimeData().dialogueItemTarget.get().get();
    if (dialoguetarget) {
      // In dialogue
      if (tiny) {
        // With valid look at target
        giant->SetGraphVariableBool("GTSIsInDialogue", true); // Allow spine edits
        auto meHead = HeadLocation(giant);
        auto targetHead = HeadLocation(tiny);
        auto directionToLook = targetHead - meHead;
        directionToLook = directionToLook * (1/directionToLook.Length());
        NiPoint3 upDirection = NiPoint3(0.0, 0.0, 1.0);
        auto sinAngle = directionToLook.Dot(upDirection);
        auto angleFromUp = asin(sinAngle);
        float angleFromForward = (angleFromUp - 90.0) * REDUCTION_FACTOR;

    		finalAngle = std::clamp(angleFromForward, -45.f, 45.f);
      }
    } else {
      // Not in dialog
      if (fabs(data.spineSmooth.value) < 1e-3) {
        // Finihed smoothing back to zero
        giant->SetGraphVariableBool("GTSIsInDialogue", false); // Disallow
      }
    }
    data.spineSmooth.target = finalAngle;

    giant->SetGraphVariableFloat("GTSPitchOverride", data.spineSmooth.value);

		log::info("Pitch Override of {} is {}", giant->GetDisplayFullName(), data.spineSmooth.value);
	}
}

namespace Gts {

	Headtracking& Headtracking::GetSingleton() noexcept {
		static Headtracking instance;
		return instance;
	}

	std::string Headtracking::DebugName() {
		return "Headtracking";
	}

  void Headtracking::Update() {
    for (auto actor: find_actors()) {
      if (actor->formID == 0x14 || actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction")) {
        SpineUpdate(actor);
        FixHeadtracking(actor);
  		}
    }
  }

  void Headtracking::SpineUpdate(Actor* me) {
    auto profiler = Profilers::Profile("Headtracking: SpineUpdate");
    SpellTest(me);
    auto ai = me->GetActorRuntimeData().currentProcess;
    Actor* tiny = nullptr;
    if (ai) {
      auto targetObjHandle = ai->GetHeadtrackTarget();
      if (targetObjHandle) {
        auto target = targetObjHandle.get().get();
        if (target) {
          auto asActor = skyrim_cast<Actor*>(target);
          if (asActor) {
            tiny = asActor;
          }
        }
      }
    }
    this->data.try_emplace(me->formID);
    RotateSpine(me, tiny, this->data.at(me->formID));
  }

	void Headtracking::FixHeadtracking(Actor* me) {
		Profilers::Start("Headtracking: Headtracking Fix");
		float scale = get_visual_scale(me);
    auto ai = me->GetActorRuntimeData().currentProcess;
    if (ai) {
  		auto targetObjHandle = ai->GetHeadtrackTarget();
  		if (targetObjHandle) {
  			auto lookAt = HeadLocation(targetObjHandle);
  			auto head = HeadLocation(me);

  			NiPoint3 directionToLook = (lookAt - head);

  			NiPoint3 myOneTimeHead = HeadLocation(me, 1.0);

  			NiPoint3 fakeLookAt = myOneTimeHead + directionToLook;

  			ai->SetHeadtrackTarget(me, fakeLookAt);
  			Profilers::Stop("Headtracking: Headtracking Fix");
  		}
    }
	}
}
