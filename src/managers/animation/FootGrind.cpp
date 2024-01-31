#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/FootGrind.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "ActionSettings.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	void ApplyDustRing(Actor* giant, FootEvent kind, std::string_view node, float mult) {
		auto& explosion = ExplosionManager::GetSingleton();
		Impact impact_data = Impact {
			.actor = giant,
			.kind = kind,
			.scale = get_visual_scale(giant) * mult,
			.nodes = find_node(giant, node),
		};
		explosion.OnImpact(impact_data); // Play explosion
	}

	void ApplyDamageOverTime_Left(Actor* giant) {
		auto gianthandle = giant->CreateRefHandle();
		std::string name = std::format("FGD_L_{}", giant->formID);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			GRumble::Once("FootGrindL", giantref, 1.0, 0.025, LNode);
			DoDamageEffect(giantref, Damage_Foot_Grind_DOT, Radius_Foot_Grind_DOT, 10000, 0.05, FootEvent::Left, 2.5, DamageSource::FootGrindedLeft);
			return true;
		});
	}

	void ApplyDamageOverTime_Right(Actor* giant) {
		auto gianthandle = giant->CreateRefHandle();
		std::string name = std::format("FGD_R_{}", giant->formID);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			GRumble::Once("FootGrindR", giantref, 1.0, 0.025, RNode);
			DoDamageEffect(giantref, Damage_Foot_Grind_DOT, Radius_Foot_Grind_DOT, 10000, 0.05, FootEvent::Right, 2.5, DamageSource::FootGrindedRight);
			return true;
		});
	}

	void CancelDamageOverTime(Actor* giant) {
		std::string Right = std::format("FGD_L_{}", giant->formID);
		std::string Left = std::format("FGD_R_{}", giant->formID);
		TaskManager::Cancel(Right);
		TaskManager::Cancel(Left);
	}

	void Footgrind_DoImpact(Actor* giant, FootEvent Event, DamageSource Source, std::string_view Node, std::string_view rumble) {
		float perk = GetPerkBonus_Basics(giant);
		ApplyDustRing(giant, Event, Node, 1.05);
		DoFootstepSound(giant, 1.0, Event, Node);
		DoLaunch(giant, 0.75 * perk, 1.35 * perk, Event);
		DoDamageEffect(giant, Damage_Foot_Grind_Impact, Radius_Foot_Grind_Impact, 20, 0.15, Event, 1.0, Source);

		DamageAV(giant, ActorValue::kStamina, 30 * GetWasteMult(giant));

		GRumble::Once(rumble, giant, 1.25, 0.05, Node);
	}

	//////////////////////////////////////////////////////////////////
	/// Events
	//////////////////////////////////////////////////////////////////

	void GTSstomp_FootGrindL_Enter(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.0;
		DrainStamina(&data.giant, "StaminaDrain_FootGrind", "DestructionBasics", true, 0.25);
	}

	void GTSstomp_FootGrindR_Enter(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = true;
		data.animSpeed = 1.0;
		DrainStamina(&data.giant, "StaminaDrain_FootGrind", "DestructionBasics", true, 0.25);
	}

	void GTSstomp_FootGrindL_MV_S(AnimationEventData& data) { // Feet starts to move: Left
		ApplyDamageOverTime_Left(&data.giant);
		ApplyDustRing(&data.giant, FootEvent::Left, LNode, 0.6);
	}

	void GTSstomp_FootGrindR_MV_S(AnimationEventData& data) { // Feet start to move: Right
		ApplyDamageOverTime_Right(&data.giant);
		ApplyDustRing(&data.giant, FootEvent::Right, RNode, 0.6);
	}

	void GTSstomp_FootGrindL_MV_E(AnimationEventData& data) { // When movement ends: Left
		CancelDamageOverTime(&data.giant);
		ApplyDustRing(&data.giant, FootEvent::Left, LNode, 0.6);
	}

	void GTSstomp_FootGrindR_MV_E(AnimationEventData& data) { // When movement ends: Right
		CancelDamageOverTime(&data.giant);
		ApplyDustRing(&data.giant, FootEvent::Right, RNode, 0.6);
	}

	void GTSstomp_FootGrindR_Impact(AnimationEventData& data) { // When foot hits the ground after lifting the leg up. R Foot
		Footgrind_DoImpact(&data.giant, FootEvent::Right, DamageSource::FootGrindedRight, RNode, "GrindStompR");
	}

	void GTSstomp_FootGrindL_Impact(AnimationEventData& data) { // When foot hits the ground after lifting the leg up. L Foot
		Footgrind_DoImpact(&data.giant, FootEvent::Left, DamageSource::FootGrindedLeft, LNode, "GrindStompL");
	}

	void GTSstomp_FootGrindR_Exit(AnimationEventData& data) { // Remove foot from enemy: Right
		data.stage = 1;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
		CancelDamageOverTime(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_FootGrind", "DestructionBasics", false, 0.25);
	}

	void GTSstomp_FootGrindL_Exit(AnimationEventData& data) { // Remove foot from enemy: Left
		data.stage = 1;
		data.canEditAnimSpeed = false;
		data.animSpeed = 1.0;
		CancelDamageOverTime(&data.giant);
		DrainStamina(&data.giant, "StaminaDrain_FootGrind", "DestructionBasics", false, 0.25);
	}
}

namespace Gts
{
	void AnimationFootGrind::RegisterEvents() {
		AnimationManager::RegisterEvent("GTSstomp_FootGrindL_Enter", "Stomp", GTSstomp_FootGrindL_Enter);
		AnimationManager::RegisterEvent("GTSstomp_FootGrindR_Enter", "Stomp", GTSstomp_FootGrindR_Enter);
		AnimationManager::RegisterEvent("GTSstomp_FootGrindL_MV_S", "Stomp", GTSstomp_FootGrindL_MV_S);
		AnimationManager::RegisterEvent("GTSstomp_FootGrindR_MV_S", "Stomp", GTSstomp_FootGrindR_MV_S);
		AnimationManager::RegisterEvent("GTSstomp_FootGrindL_MV_E", "Stomp", GTSstomp_FootGrindL_MV_E);
		AnimationManager::RegisterEvent("GTSstomp_FootGrindR_MV_E", "Stomp", GTSstomp_FootGrindR_MV_E);
		AnimationManager::RegisterEvent("GTSstomp_FootGrindR_Impact", "Stomp", GTSstomp_FootGrindR_Impact);
		AnimationManager::RegisterEvent("GTSstomp_FootGrindL_Impact", "Stomp", GTSstomp_FootGrindL_Impact);
		AnimationManager::RegisterEvent("GTSstomp_FootGrindR_Exit", "Stomp", GTSstomp_FootGrindR_Exit);
		AnimationManager::RegisterEvent("GTSstomp_FootGrindL_Exit", "Stomp", GTSstomp_FootGrindL_Exit);
	}

	void AnimationFootGrind::RegisterTriggers() {
		AnimationManager::RegisterTrigger("GrindRight", "Stomp", "GTSBEH_StartFootGrindR");
		AnimationManager::RegisterTrigger("GrindLeft", "Stomp", "GTSBEH_StartFootGrindL");
	}
}