#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/highheel.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/DeathReport.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "managers/tremor.hpp"
#include "data/transient.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {

	const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
	const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
	const std::string_view leftCalfLookup = "NPC L Calf [LClf]";
	const std::string_view rightCalfLookup = "NPC R Calf [RClf]";
	const std::string_view leftToeLookup = "NPC L Toe0 [LToe]";
	const std::string_view rightToeLookup = "NPC R Toe0 [RToe]";
	const std::string_view bodyLookup = "NPC Spine1 [Spn1]";

	void BlockFirstPerson(Actor* actor, bool block) { // Credits to ArranzCNL for this function. Forces Third Person because we don't have FP working yet.
		auto playerControls = RE::PlayerControls::GetSingleton();
		auto camera = RE::PlayerCamera::GetSingleton();
		auto controlMap = RE::ControlMap::GetSingleton();
		if (block) {
			controlMap->enabledControls.reset(RE::UserEvents::USER_EVENT_FLAG::kPOVSwitch); // Block POV Switching
			camera->ForceThirdPerson();
			return;
		}
		//playerControls->data.povScriptMode = block;
		controlMap->enabledControls.set(RE::UserEvents::USER_EVENT_FLAG::kPOVSwitch); // Allow POV Switching
	}

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

	void DoLaunch(Actor* giant, float radius, float power, FootEvent kind) {
		float smt_power = 1.0;
		float smt_radius = 1.0;
		if (HasSMT(giant)) {
			smt_power *= 2.0;
			smt_radius *= 1.25;
		}
		LaunchActor::GetSingleton().ApplyLaunch(giant, radius * smt_radius, power * smt_power, kind);
	}

	void DoLaunch(Actor* giant, float radius, float power, NiAVObject* node) {
		float smt_power = 1.0;
		float smt_radius = 1.0;
		if (HasSMT(giant)) {
			smt_power *= 2.0;
			smt_radius *= 1.25;
		}
		LaunchActor::GetSingleton().ApplyLaunch(giant, radius * smt_radius, power * smt_power, node);
	}

	void GrabStaminaDrain(Actor* giant, Actor* tiny, float sizedifference) {
		float WasteMult = 1.0;
		if (Runtime::HasPerkTeam(giant, "DestructionBasics")) {
			WasteMult *= 0.65;
		}
		if (Runtime::HasPerkTeam(giant, "SkilledGTS")) {
			float level = std::clamp(GetGtsSkillLevel() * 0.0035f, 0.0f, 0.35f);
			WasteMult -= level;
		}

		float WasteStamina = (1.00 * WasteMult)/sizedifference * TimeScale();
		DamageAV(giant, ActorValue::kStamina, WasteStamina);
	}

	void DrainStamina(Actor* giant, std::string_view TaskName, std::string_view perk, bool decide, float waste, float power) {
		float WasteMult = 1.0;
		if (Runtime::HasPerkTeam(giant, perk)) {
			WasteMult -= 0.35;
		}
		if (Runtime::HasPerkTeam(giant, "SkilledGTS")) {
			float level = std::clamp(GetGtsSkillLevel() * 0.0035f, 0.0f, 0.35f);
			WasteMult -= level;
		}
		std::string name = std::format("StaminaDrain_{}_{}", TaskName, giant->formID);
		if (decide) {
			TaskManager::Run(name, [=](auto& progressData) {
				ActorHandle casterhandle = giant->CreateRefHandle();
				if (!casterhandle) {
					return false;
				}
				float stamina = GetAV(giant, ActorValue::kStamina);
				if (stamina <= 1.0) {
					return false; // Abort if we don't have stamina so it won't drain it forever. Just to make sure.
				}
				float multiplier = AnimationManager::GetAnimSpeed(giant);
				float WasteStamina = 0.50 * power * multiplier;
				DamageAV(giant, ActorValue::kStamina, WasteStamina * WasteMult * TimeScale());
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

	float GetWasteMult(Actor* giant) {
		float WasteMult = 1.0;
		if (Runtime::HasPerk(giant, "DestructionBasics")) {
			WasteMult *= 0.65;
		}
		if (Runtime::HasPerkTeam(giant, "SkilledGTS")) {
			WasteMult -= GetGtsSkillLevel() * 0.0035;
		}
		return WasteMult;
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

	void DoFootTrample_Left(Actor* giant, Actor* tiny, bool SMT) {
		auto gianthandle = giant->CreateRefHandle();
		auto tinyhandle = tiny->CreateRefHandle();

		if (SMT) {
			ShrinkUntil(giant, tiny, 3.6);
		}

		std::string name = std::format("FootTrample_{}", tiny->formID);
		auto FrameA = Time::FramesElapsed();
		auto coordinates = AttachToUnderFoot_Left(giant, tiny);
		if (coordinates == NiPoint3(0,0,0)) {
			return;
		}
		SetBeingGrinded(tiny, true);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}

			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			auto FrameB = Time::FramesElapsed() - FrameA;
			if (FrameB <= 4.0) {
				return true;
			}
			AttachTo(giantref, tinyref, coordinates);
			if (!isTrampling(giantref)) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			if (tinyref->IsDead()) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			return true;
		});
		
	}
	void DoFootTrample_Right(Actor* giant, Actor* tiny, bool SMT) {
		auto gianthandle = giant->CreateRefHandle();
		auto tinyhandle = tiny->CreateRefHandle();

		if (SMT) {
			ShrinkUntil(giant, tiny, 3.6);
		}

		std::string name = std::format("FootTrample_{}", tiny->formID);
		auto FrameA = Time::FramesElapsed();
		auto coordinates = AttachToUnderFoot_Right(giant, tiny);
		if (coordinates == NiPoint3(0,0,0)) {
			return;
		}
		SetBeingGrinded(tiny, true);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}

			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			auto FrameB = Time::FramesElapsed() - FrameA;
			if (FrameB <= 4.0) {
				return true;
			}
			AttachTo(giantref, tinyref, coordinates);
			if (!isTrampling(giantref)) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			if (tinyref->IsDead()) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			return true;
		});
	}

	void DoFootGrind_Left(Actor* giant, Actor* tiny, bool SMT) {
		auto gianthandle = giant->CreateRefHandle();
		auto tinyhandle = tiny->CreateRefHandle();

		if (SMT) {
			ShrinkUntil(giant, tiny, 3.6);
		}

		std::string name = std::format("FootGrind_{}", tiny->formID);
		auto FrameA = Time::FramesElapsed();
		auto coordinates = AttachToUnderFoot_Left(giant, tiny);
		if (coordinates == NiPoint3(0,0,0)) {
			return;
		}
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}

			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			auto FrameB = Time::FramesElapsed() - FrameA;
			if (FrameB <= 4.0) {
				return true;
			}

			AttachTo(giantref, tinyref, coordinates);
			if (!IsFootGrinding(giantref)) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			if (tinyref->IsDead()) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			return true;
		});
	}

	void DoFootGrind_Right(Actor* giant, Actor* tiny, bool SMT) {
		auto gianthandle = giant->CreateRefHandle();
		auto tinyhandle = tiny->CreateRefHandle();

		if (SMT) {
			ShrinkUntil(giant, tiny, 3.6);
		}

		std::string name = std::format("FootGrind_{}", tiny->formID);
		auto FrameA = Time::FramesElapsed();
		auto coordinates = AttachToUnderFoot_Right(giant, tiny);
		if (coordinates == NiPoint3(0,0,0)) {
			return;
		}
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();
			auto FrameB = Time::FramesElapsed() - FrameA;
			if (FrameB <= 4.0) {
				return true;
			}

			AttachTo(giantref, tinyref, coordinates);
			if (!IsFootGrinding(giantref)) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			if (tinyref->IsDead()) {
				SetBeingGrinded(tinyref, false);
				return false;
			}
			return true;
		});
	}

	void FootGrindCheck_Left(Actor* actor, float radius, bool strong) {  // Check if we hit someone with stomp. Yes = Start foot grind. Left Foot.
		if (!actor) {
			return;
		}

		float giantScale = get_visual_scale(actor);
		const float BASE_CHECK_DISTANCE = 90.0;
		const float BASE_DISTANCE = 6.0;
		const float SCALE_RATIO = 3.0;

		bool SMT = false;

		if (HasSMT(actor)) {
			giantScale += 3.5;
			SMT = true;
		}

		// Get world HH offset
		NiPoint3 hhOffset = HighHeelManager::GetHHOffset(actor);
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(actor);

		auto leftFoot = find_node(actor, leftFootLookup);
		auto leftCalf = find_node(actor, leftCalfLookup);
		auto leftToe = find_node(actor, leftToeLookup);
		if (!leftFoot) {
			return;
		}
		if (!leftCalf) {
			return;
		}
		if (!leftToe) {
			return;
		}
		NiMatrix3 leftRotMat;
		{
			NiAVObject* foot = leftFoot;
			NiAVObject* calf = leftCalf;
			NiAVObject* toe = leftToe;
			NiTransform inverseFoot = foot->world.Invert();
			NiPoint3 forward = inverseFoot*toe->world.translate;
			forward = forward / forward.Length();

			NiPoint3 up = inverseFoot*calf->world.translate;
			up = up / up.Length();

			NiPoint3 right = forward.UnitCross(up);
			forward = up.UnitCross(right); // Reorthonalize

			leftRotMat = NiMatrix3(right, forward, up);
		}

		float maxFootDistance = BASE_DISTANCE * radius * giantScale;
		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(-1.6, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
		};
		std::tuple<NiAVObject*, NiMatrix3> left(leftFoot, leftRotMat);

		for (const auto& [foot, rotMat]: {left}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
			}
			if (IsDebugEnabled() && (actor->formID == 0x14 || actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction"))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance, 800, {0.0, 1.0, 0.0, 1.0});
				}
			}

			NiPoint3 giantLocation = actor->GetPosition();
			for (auto otherActor: find_actors()) {
				if (otherActor != actor) {
					float tinyScale = get_visual_scale(otherActor) * GetScaleAdjustment(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();

						if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
							// Check the tiny's nodes against the giant's foot points
							int nodeCollisions = 0;
							float force = 0.0;

							auto model = otherActor->GetCurrent3D();

							if (model) {
								for (auto point: footPoints) {
									VisitNodes(model, [&nodeCollisions, &force, point, maxFootDistance](NiAVObject& a_obj) {
										float distance = (point - a_obj.world.translate).Length();
										if (distance < maxFootDistance) {
											nodeCollisions += 1;
											force = 1.0 - distance / maxFootDistance;//force += 1.0 - distance / maxFootDistance;
										}
										return true;
									});
								}
							}
							if (nodeCollisions > 0) {
								float aveForce = std::clamp(force, 0.00f, 0.70f);
								if (aveForce >= 0.00 && !otherActor->IsDead()) {
									if (!strong) {
										DoFootGrind_Left(actor, otherActor, SMT);
										SetBeingGrinded(otherActor, true);
										AnimationManager::StartAnim("GrindLeft", actor);
									} else {
										AnimationManager::StartAnim("TrampleStartL", actor);
										DoFootTrample_Left(actor, otherActor, SMT);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	void FootGrindCheck_Right(Actor* actor, float radius, bool strong) {  // Check if we hit someone with stomp. Yes = Start foot grind. Right Foot.
		if (!actor) {
			return;
		}

		float giantScale = get_visual_scale(actor);
		const float BASE_CHECK_DISTANCE = 90.0;
		const float BASE_DISTANCE = 6.0;
		const float SCALE_RATIO = 3.0;

		bool SMT = false;

		if (HasSMT(actor)) {
			giantScale += 3.60;
			SMT = true;
		}

		// Get world HH offset
		NiPoint3 hhOffset = HighHeelManager::GetHHOffset(actor);
		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(actor);

		auto rightFoot = find_node(actor, rightFootLookup);
		auto rightCalf = find_node(actor, rightCalfLookup);
		auto rightToe = find_node(actor, rightToeLookup);

		if (!rightFoot) {
			return;
		}
		if (!rightCalf) {
			return;
		}
		if (!rightToe) {
			return;
		}
		NiMatrix3 rightRotMat;
		{
			NiAVObject* foot = rightFoot;
			NiAVObject* calf = rightCalf;
			NiAVObject* toe = rightToe;

			NiTransform inverseFoot = foot->world.Invert();
			NiPoint3 forward = inverseFoot*toe->world.translate;
			forward = forward / forward.Length();

			NiPoint3 up = inverseFoot*calf->world.translate;
			up = up / up.Length();

			NiPoint3 right = up.UnitCross(forward);
			forward = right.UnitCross(up); // Reorthonalize

			rightRotMat = NiMatrix3(right, forward, up);
		}

		float maxFootDistance = BASE_DISTANCE * radius * giantScale;
		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0.0, hh*0.08, -0.25 +(-hh * 0.25)), // The standard at the foot position
			NiPoint3(-1.6, 7.7 + (hh/70), -0.75 + (-hh * 1.15)), // Offset it forward
			NiPoint3(0.0, (hh/50), -0.25 + (-hh * 1.15)), // Offset for HH
		};
		std::tuple<NiAVObject*, NiMatrix3> right(rightFoot, rightRotMat);

		for (const auto& [foot, rotMat]: {right}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
			}
			if (IsDebugEnabled() && (actor->formID == 0x14 || IsTeammate(actor))) {
				for (auto point: footPoints) {
					DebugAPI::DrawSphere(glm::vec3(point.x, point.y, point.z), maxFootDistance, 800, {0.0, 1.0, 0.0, 1.0});
				}
			}

			NiPoint3 giantLocation = actor->GetPosition();
			for (auto otherActor: find_actors()) {
				if (otherActor != actor) {
					float tinyScale = get_visual_scale(otherActor) * GetScaleAdjustment(otherActor);
					if (giantScale / tinyScale > SCALE_RATIO) {
						NiPoint3 actorLocation = otherActor->GetPosition();

						if ((actorLocation-giantLocation).Length() < BASE_CHECK_DISTANCE*giantScale) {
							// Check the tiny's nodes against the giant's foot points
							int nodeCollisions = 0;
							float force = 0.0;

							auto model = otherActor->GetCurrent3D();

							if (model) {
								for (auto point: footPoints) {
									VisitNodes(model, [&nodeCollisions, &force, point, maxFootDistance](NiAVObject& a_obj) {
										float distance = (point - a_obj.world.translate).Length();
										if (distance < maxFootDistance) {
											nodeCollisions += 1;
											force = 1.0 - distance / maxFootDistance;
										}
										return true;
									});
								}
							}
							if (nodeCollisions > 0) {
								float aveForce = std::clamp(force, 0.00f, 0.70f);
								if (aveForce >= 0.00 && !otherActor->IsDead()) {
									if (!strong) {
										DoFootGrind_Right(actor, otherActor, SMT);
										SetBeingGrinded(otherActor, true);
										AnimationManager::StartAnim("GrindRight", actor);
									} else {
										AnimationManager::StartAnim("TrampleStartR", actor); // Do Trample instead
										DoFootTrample_Right(actor, otherActor, SMT);
									}
								}
							}
						}
					}
				}
			}
		}
	}

	void DoDamageAtPoint_Cooldown(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult, float crushmult, float pushpower, DamageSource Cause) { // Apply crawl damage to each bone individually
		auto profiler = Profilers::Profile("Other: CrawlDamage");
		if (!node) {
			return;
		} if (!giant) {
			return;
		}
		auto& sizemanager = SizeManager::GetSingleton();
		float giantScale = get_visual_scale(giant);

		float SCALE_RATIO = 1.0;
		bool SMT = false;
		

		if (HasSMT(giant)) {
			giantScale += 1.75; // enough to push giants around, but not mammoths/dragons
			SMT = true; // set SMT to true
		}

		NiPoint3 NodePosition = node->world.translate;
		float maxDistance = radius * giantScale;

		if (IsDebugEnabled() && (giant->formID == 0x14 || giant->IsPlayerTeammate() || Runtime::InFaction(giant, "FollowerFaction"))) {
			DebugAPI::DrawSphere(glm::vec3(NodePosition.x, NodePosition.y, NodePosition.z), maxDistance);
		}

		NiPoint3 giantLocation = giant->GetPosition();

		for (auto otherActor: find_actors()) {
			if (otherActor != giant) {
				float tinyScale = get_visual_scale(otherActor);
				NiPoint3 actorLocation = otherActor->GetPosition();
				if ((actorLocation - giantLocation).Length() < maxDistance * 6.0) {
					tinyScale *= GetScaleAdjustment(otherActor); // take Giant/Dragon scale into account
					int nodeCollisions = 0;
					float force = 0.0;

					auto model = otherActor->GetCurrent3D();

					if (model) {
						VisitNodes(model, [&nodeCollisions, &force, NodePosition, maxDistance](NiAVObject& a_obj) {
							float distance = (NodePosition - a_obj.world.translate).Length();
							if (distance < maxDistance) {
								nodeCollisions += 1;
								force = 1.0 - distance / maxDistance;
								return false;
							}
							return true;
						});
					}
					if (nodeCollisions > 0) {
						bool allow = sizemanager.IsHandDamaging(otherActor);
						if (!allow) {
							float aveForce = std::clamp(force, 0.15f, 0.70f);
							float pushForce = std::clamp(force, 0.04f, 0.10f);
							float audio = 1.0;
							if (SMT) {
								pushForce *= 1.5;
								damage *= 2.25;
								audio = 3.0;
							}
							AccurateDamage::GetSingleton().ApplySizeEffect(giant, otherActor, aveForce * damage, random, bbmult, crushmult, Cause);
							if (giantScale / tinyScale > 2.25) {
								PushTowards(giant, otherActor, node, pushForce * pushpower, true);
							}
							float Volume = clamp(0.25, 1.0, (giantScale/tinyScale)*pushForce);

							auto node = find_node(giant, GetDeathNodeName(Cause));
							if (node) {
								Runtime::PlaySoundAtNode("SwingImpact", giant, Volume, 1.0, node); // play swing impact sound
							}

							ApplyShakeAtPoint(giant, 3.0 * pushpower * audio, node->world.translate, 1.5);
							sizemanager.GetDamageData(otherActor).lastHandDamageTime = Time::WorldTimeElapsed();
						}
					}
				}
			}
		}
	}
}
