
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "utils/papyrusUtils.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "data/transient.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
    void AllowToDoVore(Actor* actor, bool toggle) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			transient->can_do_vore = toggle;
		}
	}

	void AllowToBeCrushed(Actor* actor, bool toggle) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			transient->can_be_crushed = toggle;
		}
	}

	void ManageCamera(Actor* giant, bool enable, float type) {
		auto& sizemanager = SizeManager::GetSingleton();
		sizemanager.SetActionBool(giant, enable, type);
	}

    void DoLaunch(Actor* giant, float radius, float damage, std::string_view node, float override) {
		float bonus = 1.0;
		if (HasSMT(giant)) {
			bonus = override;
		}
		LaunchActor::GetSingleton().ApplyLaunch(giant, radius * bonus, damage, node);
	}

    void GrabStaminaDrain(Actor* giant, Actor* tiny, float sizedifference) {
		float WasteMult = 1.0;
		if (Runtime::HasPerkTeam(giant, "DestructionBasics")) {
			WasteMult *= 0.65;
		}
		float WasteStamina = (1.00 * WasteMult)/sizedifference;
		DamageAV(giant, ActorValue::kStamina, WasteStamina);
	}

    void DrainStamina(Actor* giant, std::string_view TaskName, std::string_view perk, bool decide, float waste, float power) {
		float WasteMult = 1.0;
		if (Runtime::HasPerkTeam(giant, perk)) {
			WasteMult *= 0.65;
		}
		std::string name = std::format("StaminaDrain_{}_{}", TaskName, giant->formID);
		if (decide) {
			TaskManager::Run(name, [=](auto& progressData) {
				ActorHandle casterhandle = giant->CreateRefHandle();
				if (!casterhandle) {
					return false;
				}
				float multiplier = AnimationManager::GetAnimSpeed(giant);
				float WasteStamina = 0.50 * power * multiplier;
				DamageAV(giant, ActorValue::kStamina, WasteStamina * WasteMult);
				return true;
			});
		} else {
			TaskManager::Cancel(name);
		}
	}

    void SpawnHurtParticles(Actor* giant, Actor* grabbedActor, float mult, float dustmult) {
		auto hand = find_node(giant, "NPC L Hand [LHnd]");
		if (hand) {
			if (IsLiving(grabbedActor)) {
				if (!LessGore()) {
					SpawnParticle(giant, 25.0, "GTS/Damage/Explode.nif", hand->world.rotate, hand->world.translate, get_visual_scale(grabbedActor) * 3* mult, 4, hand);
					SpawnParticle(giant, 25.0, "GTS/Damage/Crush.nif", hand->world.rotate, hand->world.translate, get_visual_scale(grabbedActor) * 3 *  mult, 4, hand);
				} else if (LessGore()) {
					Runtime::PlaySound("BloodGushSound", grabbedActor, 1.0, 0.5);
				}
			} else {
				SpawnDustParticle(giant, grabbedActor, "NPC L Hand [LHnd]", dustmult);
			}
		}
	}

    void ToggleEmotionEdit(Actor* giant, bool allow) {
		auto& Emotions = EmotionManager::GetSingleton().GetGiant(giant);
		Emotions.AllowEmotionEdit = allow;
	}
	void AdjustFacialExpression(Actor* giant, int ph, float power, std::string_view type) {
		auto& Emotions = EmotionManager::GetSingleton().GetGiant(giant);
		float AnimSpeed = AnimationManager::GetSingleton().GetAnimSpeed(giant);

		if (type == "phenome") {
			Emotions.OverridePhenome(ph, 0.0, 0.08/AnimSpeed, power);
		}
		if (type == "expression") {
			auto fgen = giant->GetFaceGenAnimationData();
			if (fgen) {
				fgen->exprOverride = false;
				fgen->SetExpressionOverride(ph, power);
				fgen->expressionKeyFrame.SetValue(ph, power); // Expression doesn't need Spring since it is already smooth by default
				fgen->exprOverride = true;
			}
		}
		if (type == "modifier") {
			Emotions.OverrideModifier(ph, 0.0, 0.25/AnimSpeed, power);
		}
	}

    float GetPerkBonus_Basics(Actor* Giant) {
		if (Runtime::HasPerkTeam(Giant, "DestructionBasics")) {
			return 1.25;
		} else {
			return 1.0;
			}
		}

    float GetPerkBonus_Thighs(Actor* Giant) {
		if (Runtime::HasPerkTeam(Giant, "KillerThighs")) {
			return 1.15;
		} else {
			return 1.0;
		}
	}
}
