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

	void RotateSpine(Actor* giant, Actor* tiny) { // Manages Spine rotation and helps the spells to land properly
		bool Collision_Installed = false; //Used to detect 'Precision' mod
		float Collision_PitchMult = 0.0;
		giant->GetGraphVariableBool("Collision_Installed", Collision_Installed);
		if (Collision_Installed == true) {
			giant->GetGraphVariableFloat("Collision_PitchMult", Collision_PitchMult); // If true, obtain value to apply it
			//giant->SetGraphVariableFloat("Collision_PitchMult", 0.0);
			//log::info("Callision Pitch Mult: {}", Collision_PitchMult);
		}
		float sizedifference = (get_visual_scale(giant)/get_visual_scale(tiny) - 1.0);
		float modifier = 0.0;
		if (sizedifference > 1) {
			modifier = std::clamp(sizedifference*2, 0.0f, 240.0f); // look down
			giant->SetGraphVariableFloat("GTSPitchOverride", -modifier -Collision_PitchMult);
		} else if (sizedifference < 1) {
			modifier = std::clamp(sizedifference*6, 0.0f, 60.0f); // look up
			giant->SetGraphVariableFloat("GTSPitchOverride", modifier -Collision_PitchMult);
		}
		log::info("Pitch Override of {} is {}", giant->GetDisplayFullName(), modifier);
	}

	void DialogueCheck(Actor* giant) {
		bool GTSIsInDialogue; // Toggles Dialogue Spine edits
		auto dialoguetarget = giant->GetActorRuntimeData().dialogueItemTarget.get().get();
		if (dialoguetarget) {
			giant->SetGraphVariableBool("GTSIsInDialogue", true); // Allow spine edits
		} else {
			giant->SetGraphVariableBool("GTSIsInDialogue", false); // Disallow
		}
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

	void Headtracking::FixHeadtracking(Actor* me) {
		Profilers::Start("Headtracking: Headtracking Fix");
		SpellTest(me);
    NiPoint3 headOffset(0.0, 0.0, 127.0);
		DialogueCheck(me); // Check for Dialogue
		float scale = get_visual_scale(me);
		auto ai = me->GetActorRuntimeData().currentProcess;
		bhkCharacterController* CharController = ai->GetCharController();
		if (CharController) {
			headOffset.z = CharController->actorHeight * 70;
		}
		auto targetObjHandle = ai->GetHeadtrackTarget();
		if (targetObjHandle) {
			auto targetObj = targetObjHandle.get().get();
			if (targetObj) {
				NiPoint3 targetHeadOffset(0.0, 0.0, 0.0);
        float targetScale = 1.0;
				auto target = skyrim_cast<Actor*>(targetObj);
				if (target) {
					targetScale = get_visual_scale(target);
					auto targetChar = target->GetCharController();
					RotateSpine(me, target);
					if (targetChar) {
						targetHeadOffset.z = targetChar->actorHeight * 70.0;
					}
				}

				auto lookAt = targetObj->GetPosition() + targetHeadOffset * targetScale;
				auto head = me->GetPosition() + headOffset * scale;

				NiPoint3 directionToLook = (lookAt - head);

				NiPoint3 myOneTimeHead = me->GetPosition() + headOffset;

				NiPoint3 fakeLookAt = myOneTimeHead + directionToLook;

				ai->SetHeadtrackTarget(me, fakeLookAt);
				Profilers::Stop("Headtracking: Headtracking Fix");
				return;
			}
		} else {
			float PitchOverride;
			me->GetGraphVariableFloat("GTSPitchOverride", PitchOverride);
			if (PitchOverride != 0) {
				me->SetGraphVariableFloat("GTSPitchOverride", 0);
			}
		}

	}
}
