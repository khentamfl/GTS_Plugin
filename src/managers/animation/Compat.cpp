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
#include "managers/damage/AccurateDamage.hpp"
#include "managers/animation/Compat.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
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
		float SCALE_RATIO = 0.0;
		float giantScale = get_visual_scale(giant);
		NiPoint3 giantLocation = giant->GetPosition();
		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				if (otherActor->IsInKillMove()) {
					float tinyScale = get_visual_scale(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();
						if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
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
					DoDamageAtPoint(giant, 15, 0.45 * damage, Node, 400, 0.10, 1.33, DamageSource::BodyCrush);
				}
			}

			if (BreastL03 && BreastR03) {
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastL03, 400, 0.10, 1.33, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastR03, 400, 0.10, 1.33, DamageSource::BreastImpact);
				return true;
			} else if (BreastL && BreastR) {
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastL, 400, 0.10, 1.33, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastR, 400, 0.10, 1.33, DamageSource::BreastImpact);
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
					Rumble::Once(rumbleName, giant, 0.25, 0.02, Nodes);
				}
			}

			if (BreastL03 && BreastR03) {
				Rumble::Once("BreastDot_L", giantref, 0.25, 0.025, "L Breast03");
				Rumble::Once("BreastDot_R", giantref, 0.25, 0.025, "R Breast03");
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastL03, 200, 0.10, 2.5, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastR03, 200, 0.10, 2.5, DamageSource::BreastImpact);
				return true;
			} else if (BreastL && BreastR) {
				Rumble::Once("BreastDot_L", giantref, 0.25, 0.025, "NPC L Breast");
				Rumble::Once("BreastDot_R", giantref, 0.25, 0.025, "NPC R Breast");
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastL, 200, 0.10, 2.5, DamageSource::BreastImpact);
				DoDamageAtPoint(giant, 14, 0.6 * damage, BreastR, 200, 0.10, 2.5, DamageSource::BreastImpact);
				return true;
			}
			return false;
		});
	}


	void GTScrush_caster(AnimationEventData& data) {
		//data.stage = 0;
		log::info("GTScrush_caster");
		TriggerKillZone(&data.giant);
	}

	void GTScrush_victim(AnimationEventData& data) {
		//data.stage = 0;
		if (data.giant.formID != 0x14) {
			auto player = PlayerCharacter::GetSingleton();
			float giantscale = get_visual_scale(player);
			float tinyscale = get_visual_scale(&data.giant);
			float sizedifference = giantscale/tinyscale;
			if (sizedifference >= 0.0) {
				CrushManager::Crush(player, &data.giant);
				PrintDeathSource(player, &data.giant, DamageSource::CrushedLeft);
			}
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
		DoLaunch(&data.giant, 0.75, 1.75, 1.4, FootEvent::Right, 0.85);
		DoLaunch(&data.giant, 0.75, 1.75, 1.4, FootEvent::Left, 0.85);
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
		DoLaunch(&data.giant, 0.75, 1.75, 1.4, FootEvent::Right, 0.85);
		DoLaunch(&data.giant, 0.75, 1.75, 1.4, FootEvent::Left, 0.85);
	}

	void JumpDown(AnimationEventData& data) {
		auto giant = &data.giant;
		float perk = GetPerkBonus_Basics(giant);
		auto& sizemanager = SizeManager::GetSingleton();
		float damage = sizemanager.GetSizeAttribute(giant, 2);
		DoDamageEffect(&data.giant, 0.9 * damage, 6.0, 20, 0.25, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoDamageEffect(&data.giant, 0.9 * damage, 6.0, 20, 0.25, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoLaunch(&data.giant, 1.05 * perk, 1.60, 1.2, FootEvent::Left, 1.15);
		DoLaunch(&data.giant, 1.05 * perk, 1.60, 1.2, FootEvent::Right, 1.15);
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
		StartBodyDamage_DOT(giant);
	}
	void GTS_BodyDamage_OFF(AnimationEventData& data) {
		auto giant = &data.giant;
		StopBodyDamage_DOT(giant);
	}

	void SBOProneOnEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("SBO_ProneOn", player);
	}

	void SBOProneOffEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("SBO_ProneOff", player);
	}

	void SBODiveEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		AnimationManager::StartAnim("SBO_Dive", player);
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
		AnimationManager::RegisterEvent("GTS_BodyDamage_OFF", "Body", GTS_BodyDamage_OFF);
		AnimationManager::RegisterEvent("JumpDown", "JumpCompat1", JumpDown);
	}

	void AnimationCompat::RegisterTriggers() {
		AnimationManager::RegisterTrigger("SBO_ProneOn", "Crawl", "GTSBeh_ProneStart");
		AnimationManager::RegisterTrigger("SBO_ProneOff", "Crawl", "GTSBeh_ProneStop");
		AnimationManager::RegisterTrigger("SBO_Dive", "Crawl", "GTSBeh_ProneStart_Dive");
	}
}
