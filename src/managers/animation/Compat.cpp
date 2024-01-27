// Animation: Compatibility
// Notes: Made avaliable for other generic anim mods
//  - Stages
//    - "GTScrush_caster",          //[0] The gainer.
//    - "GTScrush_victim",          //[1] The one to crush
// Notes: Modern Combat Overhaul compatibility
// - Stages
//   - "MCO_SecondDodge",           // enables GTS sounds and footstep effects
//   - "SoundPlay.MCO_DodgeSound",

#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/CrawlUtils.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/Compat.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	const std::vector<std::string_view> BODY_NODES = {
		"NPC R Thigh [RThg]",
		"NPC L Thigh [LThg]",
		"NPC R Butt",
		"NPC L Butt",
		"NPC Spine [Spn0]",
		"NPC Spine1 [Spn1]",
		"NPC Spine2 [Spn2]",
	};

	void TriggerKillZone(Actor* giant) {
		if (!giant) {
			return;
		}
		float BASE_CHECK_DISTANCE = 90.0;
		float SCALE_RATIO = 3.0;
		if (HasSMT(giant)) {
			SCALE_RATIO = 0.8;
		}
		float giantScale = get_visual_scale(giant);
		NiPoint3 giantLocation = giant->GetPosition();
		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				if (otherActor->IsInKillMove()) {
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();
						if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale * 3) {
							PrintDeathSource(giant, otherActor, DamageSource::Booty);
							CrushManager::Crush(giant, otherActor);
						}
					}
				}
			}
		}
	}

	void StopBodyDamage_Slide(Actor* giant) {
		std::string name = std::format("BodyDOT_Slide_{}", giant->formID);
		TaskManager::Cancel(name);
	}

	void StopBodyDamage_DOT(Actor* giant) {
		std::string name = std::format("BodyDOT_{}", giant->formID);
		TaskManager::Cancel(name);
	}

	void StartBodyDamage_DOT(Actor* giant) {
		float damage = 2.0 * TimeScale();
		auto gianthandle = giant->CreateRefHandle();
		std::string name = std::format("BodyDOT_{}", giant->formID);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();

			auto BreastL = find_node(giantref, "NPC L Breast");
			auto BreastR = find_node(giantref, "NPC R Breast");
			auto BreastL03 = find_node(giantref, "L Breast03");
			auto BreastR03 = find_node(giantref, "R Breast03");

			if (!IsProning(giantref)) {
				return false;
			}

			for (auto Nodes: BODY_NODES) {
				auto Node = find_node(giantref, Nodes);
				if (Node) {
					DoDamageAtPoint(giant, 15, 0.20 * damage, Node, 400, 0.10, 1.33, DamageSource::BodyCrush);
				}
			}

			if (BreastL03 && BreastR03) {
				DoDamageAtPoint(giant, 14, 0.2 * damage, BreastL03, 400, 0.10, 1.33, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, 14, 0.2 * damage, BreastR03, 400, 0.10, 1.33, DamageSource::BreastImpact);
				return true;
			} else if (BreastL && BreastR) {
				DoDamageAtPoint(giant, 14, 0.2 * damage, BreastL, 400, 0.10, 1.33, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, 14, 0.2 * damage, BreastR, 400, 0.10, 1.33, DamageSource::BreastImpact);
				return true;
			}
			return false;
		});
	}

	void StartBodyDamage_Slide(Actor* giant) {
		float damage = 24.0 * TimeScale();
		auto gianthandle = giant->CreateRefHandle();
		std::string name = std::format("BodyDOT_Slide_{}", giant->formID);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();

			auto BreastL = find_node(giantref, "NPC L Breast");
			auto BreastR = find_node(giantref, "NPC R Breast");
			auto BreastL03 = find_node(giantref, "L Breast03");
			auto BreastR03 = find_node(giantref, "R Breast03");

			if (!IsProning(giantref)) {
				return false;
			}

			for (auto Nodes: BODY_NODES) {
				auto Node = find_node(giantref, Nodes);
				if (Node) {
					std::string rumbleName = std::format("Node: {}", Nodes);
					DoDamageAtPoint(giant, 15, 0.45 * damage, Node, 200, 0.10, 2.0, DamageSource::BodyCrush);
					GRumble::Once(rumbleName, giant, 0.25, 0.02, Nodes);
				}
			}

			if (BreastL03 && BreastR03) {
				GRumble::Once("BreastDot_L", giantref, 0.25, 0.025, "L Breast03");
				GRumble::Once("BreastDot_R", giantref, 0.25, 0.025, "R Breast03");
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastL03, 200, 0.10, 2.5, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastR03, 200, 0.10, 2.5, DamageSource::BreastImpact);
				return true;
			} else if (BreastL && BreastR) {
				GRumble::Once("BreastDot_L", giantref, 0.25, 0.025, "NPC L Breast");
				GRumble::Once("BreastDot_R", giantref, 0.25, 0.025, "NPC R Breast");
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastL, 200, 0.10, 2.5, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastR, 200, 0.10, 2.5, DamageSource::BreastImpact);
				return true;
			}
			return false;
		});
	}

	

	void GTScrush_caster(AnimationEventData& data) { 
		// Compatibility with Thick Thighs Take Lives mod, this compatibility probably needs a revision.
		// Mainly just need to call damage similar to how we do it with DoDamageAtPoint() function
		// 21.01.2024
		//data.stage = 0;
		TriggerKillZone(&data.giant);
	}

	void GTScrush_victim(AnimationEventData& data) { // Compatibility with Thick Thighs Take Lives mod
		//data.stage = 0;
		if (data.giant.formID != 0x14) {
			TriggerKillZone(PlayerCharacter::GetSingleton());
		}
	}

	void MCO_SecondDodge(AnimationEventData& data) {
		data.stage = 0;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20;
		DoDamageEffect(&data.giant, 1.20, 1.4, 10, 0.20, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoDamageEffect(&data.giant, 1.20, 1.4, 10, 0.20, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, 1.0, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.90, 1.35, FootEvent::Right);
		DoLaunch(&data.giant, 0.90, 1.35, FootEvent::Left);
	}
	void MCO_DodgeSound(AnimationEventData& data) {
		data.stage = 0;
		float scale = get_visual_scale(&data.giant);
		float volume = scale * 0.20;
		DoDamageEffect(&data.giant, 1.20, 1.4, 10, 0.20, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoDamageEffect(&data.giant, 1.20, 1.4, 10, 0.20, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, 1.0, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.90, 1.35, FootEvent::Right);
		DoLaunch(&data.giant, 0.90, 1.35, FootEvent::Left);
	}

	void GTS_DiveSlide_ON(AnimationEventData& data) {
		auto giant = &data.giant;
		StartBodyDamage_Slide(giant);
	}
	void GTS_DiveSlide_OFF(AnimationEventData& data) {
		auto giant = &data.giant;
		StopBodyDamage_Slide(giant);
	}
	void GTS_BodyDamage_ON(AnimationEventData& data) {
		auto giant = &data.giant;
		SetProneState(giant, true);
		StartBodyDamage_DOT(giant);
	}
	void GTS_BodyDamage_Off(AnimationEventData& data) {
		auto giant = &data.giant;
		SetProneState(giant, false);
		StopBodyDamage_DOT(giant);
	}

	void SBOProneOnEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("SBO_ProneOn", player);
	}

	void SBOProneOffEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (player->IsSneaking()) {
			AnimationManager::StartAnim("SBO_ProneOff", player);
		}
	}

	void SBODiveEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (player->IsSneaking()) {
			AnimationManager::StartAnim("SBO_Dive", player);
		}
	}
}

namespace Gts
{
	void AnimationCompat::RegisterEvents() {
		InputManager::RegisterInputEvent("SBO_ToggleProne", SBOProneOnEvent);
		InputManager::RegisterInputEvent("SBO_DisableProne", SBOProneOffEvent);
		InputManager::RegisterInputEvent("SBO_ToggleDive", SBODiveEvent);

		AnimationManager::RegisterEvent("GTScrush_caster", "Compat1", GTScrush_caster);
		AnimationManager::RegisterEvent("GTScrush_victim", "Compat2", GTScrush_victim);
		AnimationManager::RegisterEvent("MCO_SecondDodge", "MCOCompat1", MCO_SecondDodge);
		AnimationManager::RegisterEvent("SoundPlay.MCO_DodgeSound", "MCOCompat2", MCO_DodgeSound);
		AnimationManager::RegisterEvent("GTS_DiveSlide_ON", "Body", GTS_DiveSlide_ON);
		AnimationManager::RegisterEvent("GTS_DiveSlide_OFF", "Body", GTS_DiveSlide_OFF);
		AnimationManager::RegisterEvent("GTS_BodyDamage_ON", "Body", GTS_BodyDamage_ON);
		AnimationManager::RegisterEvent("GTS_BodyDamage_Off", "Body", GTS_BodyDamage_Off);

		
	}

	void AnimationCompat::RegisterTriggers() {
		AnimationManager::RegisterTrigger("SBO_ProneOn", "Crawl", "GTSBeh_ProneStart");
		AnimationManager::RegisterTrigger("SBO_ProneOff", "Crawl", "GTSBeh_ProneStop");
		AnimationManager::RegisterTrigger("SBO_Dive", "Crawl", "GTSBeh_ProneStart_Dive");
	}
}
