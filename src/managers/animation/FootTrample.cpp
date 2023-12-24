#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/FootTrample.hpp"
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

	void DeplenishStamina(Actor* giant, float WasteStamina) {
		DamageAV(giant, ActorValue::kStamina, WasteStamina * GetWasteMult(giant));
	}

	void DoSounds(Actor* giant, float animspeed, std::string_view feet) {
		float bonus = 1.0;
		if (HasSMT(giant)) {
			bonus = 8.0;
		}
		float scale = get_visual_scale(giant);
		Runtime::PlaySoundAtNode("HeavyStompSound", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
		Runtime::PlaySoundAtNode("xlFootstepR", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
		Runtime::PlaySoundAtNode("xlRumbleR", giant, 0.14 * bonus * scale * animspeed, 1.0, feet);
	}

	void GTS_Trample_Leg_Raise_L(AnimationEventData& data) {
		data.stage = 1;
		data.canEditAnimSpeed = false;
		if (data.animSpeed == 1.0) {
			data.animSpeed = 1.3;
		}
	}
	void GTS_Trample_Leg_Raise_R(AnimationEventData& data) {
		//TrackFeet(&data.giant, 6, true);
		data.stage = 1;
		data.canEditAnimSpeed = false;
		if (data.animSpeed == 1.0) {
			data.animSpeed = 1.3;
		}
	}

	void GTS_Trample_Cam_Start_L(AnimationEventData& data) {
		TrackFeet(&data.giant, 5, true);
	}
	void GTS_Trample_Cam_Start_R(AnimationEventData& data) {
		TrackFeet(&data.giant, 6, true);
	}

	void GTS_Trample_Cam_End_L(AnimationEventData& data) {
		TrackFeet(&data.giant, 5, false);
		DrainStamina(&data.giant, "StaminaDrain_Trample", "DestructionBasics", false, 1.0, 1.2);

		data.animSpeed = 1.0;
		data.canEditAnimSpeed = false;
		data.stage = 0;
	}
	void GTS_Trample_Cam_End_R(AnimationEventData& data) {
		TrackFeet(&data.giant, 6, false);
		DrainStamina(&data.giant, "StaminaDrain_Trample", "DestructionBasics", false, 1.0, 1.2);

		data.animSpeed = 1.0;
		data.canEditAnimSpeed = false;
		data.stage = 0;
	}

	void GTS_Trample_Footstep_L(AnimationEventData& data) { // Stage 1 footsteps
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.25;
		}
		GRumble::Once("TrampleL", &data.giant, 1.60 * shake, 0.0, LNode);
		DoDamageEffect(&data.giant, 0.7 * launch * perk, 1.45 * launch, 100, 0.10, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.65 * perk, 0.75 * perk, FootEvent::Left);
		DrainStamina(&data.giant, "StaminaDrain_Trample", "DestructionBasics", true, 1.0, 1.2);
		FootGrindCheck_Left(&data.giant, 1.65, true);

		data.animSpeed = 1.0;
		data.canEditAnimSpeed = false;
		data.stage = 0;
	}
	void GTS_Trample_Footstep_R(AnimationEventData& data) { // stage 1 footsteps
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.25;
		}
		GRumble::Once("TrampleR", &data.giant, 1.60 * shake, 0.0, RNode);
		DoDamageEffect(&data.giant, 0.7 * launch * perk, 1.45 * launch, 100, 0.10, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.65 * perk, 0.75 * perk, FootEvent::Right);
		DrainStamina(&data.giant, "StaminaDrain_Trample", "DestructionBasics", true, 1.0, 1.2);
		FootGrindCheck_Right(&data.giant, 1.65, true);

		data.animSpeed = 1.0;
		data.canEditAnimSpeed = false;
		data.stage = 0;
	}

	void GTS_Trample_Impact_L(AnimationEventData& data) { // Stage 2 repeating footsteps
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.25;
		}
		GRumble::Once("TrampleL", &data.giant, 2.20 * shake, 0.0, LNode);
		DoDamageEffect(&data.giant, 1.1 * launch * perk, 1.65 * launch, 100, 0.10, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.85 * perk, 1.75 * perk, FootEvent::Left);
		DeplenishStamina(&data.giant, 30.0);

		data.stage = 1;
		data.canEditAnimSpeed = false;
		if (data.animSpeed == 1.0) {
			data.animSpeed = 1.15;
		}
	}

	void GTS_Trample_Impact_R(AnimationEventData& data) { // Stage 2 repeating footsteps
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.0;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.25;
		}
		GRumble::Once("TrampleR", &data.giant, 2.20 * shake, 0.0, RNode);
		DoDamageEffect(&data.giant, 1.1 * launch * perk, 1.65 * launch, 100, 0.10, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.85 * perk, 1.75 * perk, FootEvent::Right);
		DeplenishStamina(&data.giant, 30.0);

		data.stage = 1;
		data.canEditAnimSpeed = false;
		if (data.animSpeed == 1.00) {
			data.animSpeed = 1.15;
		}
	}

	void GTS_Trample_Finisher_L(AnimationEventData& data) { // last hit that deals huge chunk of damage
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.25;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.50;
		}
		GRumble::Once("TrampleL", &data.giant, 3.20 * shake, 0.0, RNode);
		DoDamageEffect(&data.giant, 5.0 * launch * perk, 1.80 * launch, 10, 0.15, FootEvent::Left, 0.90, DamageSource::CrushedLeft);
		DoLaunch(&data.giant, 1.25 * perk, 3.20 * perk, FootEvent::Left);
		DoFootstepSound(&data.giant, 1.15, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Left, LNode);
		DeplenishStamina(&data.giant, 80.0);
		DoSounds(&data.giant, 1.25, LNode);
	}
	void GTS_Trample_Finisher_R(AnimationEventData& data) { // last hit that deals huge chunk of damage
		float shake = 1.0;
		float launch = 1.0;
		float dust = 1.25;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.2;
			dust = 1.50;
		}
		GRumble::Once("TrampleR", &data.giant, 3.20 * shake, 0.0, RNode);
		DoDamageEffect(&data.giant, 5.0 * launch * perk, 1.80 * launch, 10, 0.15, FootEvent::Right, 0.90, DamageSource::CrushedRight);
		DoLaunch(&data.giant, 1.25 * perk, 3.20 * perk, FootEvent::Right);
		DoFootstepSound(&data.giant, 1.15, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Right, RNode);
		DeplenishStamina(&data.giant, 80.0);
		DoSounds(&data.giant, 1.25, LNode);
	}

	/////////////////////////////////////////////////////////// Triggers

	void TrampleLeftEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1)) {
			return;
		}
		if (IsCrawling(player) || player->IsSneaking() || IsProning(player)) {
			return;
		}
		float WasteStamina = 35.0 * GetWasteMult(player);

		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("TrampleL", player);
		} else {
			TiredSound(player, "You're too tired to perform trample");
		}
	}

	void TrampleRightEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 1)) {
			return;
		}
		if (IsCrawling(player) || player->IsSneaking() || IsProning(player)) {
			return;
		}
		float WasteStamina = 35.0 * GetWasteMult(player);
		if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
			AnimationManager::StartAnim("TrampleR", player);
		} else {
			TiredSound(player, "You're too tired to perform trample");
		}
	}
}

namespace Gts
{
	void AnimationFootTrample::RegisterEvents() {
		InputManager::RegisterInputEvent("TrampleLeft", TrampleLeftEvent);
		InputManager::RegisterInputEvent("TrampleRight", TrampleRightEvent);

		AnimationManager::RegisterEvent("GTS_Trample_Leg_Raise_L", "Trample", GTS_Trample_Leg_Raise_L);
		AnimationManager::RegisterEvent("GTS_Trample_Leg_Raise_R", "Trample", GTS_Trample_Leg_Raise_R);

		AnimationManager::RegisterEvent("GTS_Trample_Cam_Start_L", "Trample", GTS_Trample_Cam_Start_L);
		AnimationManager::RegisterEvent("GTS_Trample_Cam_Start_R", "Trample", GTS_Trample_Cam_Start_R);

		AnimationManager::RegisterEvent("GTS_Trample_Cam_End_L", "Trample", GTS_Trample_Cam_End_L);
		AnimationManager::RegisterEvent("GTS_Trample_Cam_End_R", "Trample", GTS_Trample_Cam_End_R);

		AnimationManager::RegisterEvent("GTS_Trample_Impact_L", "Trample", GTS_Trample_Impact_L);
		AnimationManager::RegisterEvent("GTS_Trample_Impact_R", "Trample", GTS_Trample_Impact_R);

		AnimationManager::RegisterEvent("GTS_Trample_Footstep_L", "Trample", GTS_Trample_Footstep_L);
		AnimationManager::RegisterEvent("GTS_Trample_Footstep_R", "Trample", GTS_Trample_Footstep_R);

		AnimationManager::RegisterEvent("GTS_Trample_Finisher_L", "Trample", GTS_Trample_Finisher_L);
		AnimationManager::RegisterEvent("GTS_Trample_Finisher_R", "Trample", GTS_Trample_Finisher_R);
	}

	void AnimationFootTrample::RegisterTriggers() {
		AnimationManager::RegisterTrigger("TrampleL", "Trample", "GTSBeh_Trample_L");
		AnimationManager::RegisterTrigger("TrampleR", "Trample", "GTSBeh_Trample_R");

		AnimationManager::RegisterTrigger("TrampleStartL", "Trample", "GTSBEH_Trample_Start_L");
		AnimationManager::RegisterTrigger("TrampleStartR", "Trample", "GTSBEH_Trample_Start_R");
	}
}