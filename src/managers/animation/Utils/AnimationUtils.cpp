#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/damage/CollisionDamage.hpp"
#include "managers/animation/HugShrink.hpp"
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
#include "utils/looting.hpp"
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

	void Hugs_FixAnimationDesync(Actor* giant, Actor* tiny, bool reset) {
		auto transient = Transient::GetSingleton().GetData(tiny);
		if (transient) {
			float& animspeed = transient->Hug_AnimSpeed;
			if (!reset) {
				animspeed = AnimationManager::GetAnimSpeed(giant);
			} else {
				animspeed = 1.0; // 1.0 makes dll use GetAnimSpeed of tiny
			}
			// Fix hug anim de-sync
		}
	}

	void Vore_AttachToRightHandTask(Actor* giant, Actor* tiny) {
		std::string name = std::format("CrawlVore_{}_{}", giant->formID, tiny->formID);
		ActorHandle giantHandle = giant->CreateRefHandle();
		ActorHandle tinyHandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!giantHandle) {
				return false;
			} 
			if (!tinyHandle) {
				return false;
			}
			auto giantref = giantHandle.get().get();
			auto tinyref = tinyHandle.get().get();

			auto FingerA = find_node(giant, "NPC R Finger02 [RF02]");
			if (!FingerA) {
				Notify("R Finger 02 node not found");
				return false;
			}
			auto FingerB = find_node(giant, "NPC R Finger30 [RF30]");
			if (!FingerB) {
				Notify("R Finger 30 node not found");
				return false;
			}
			NiPoint3 coords = (FingerA->world.translate + FingerB->world.translate) / 2.0;
			coords.z -= 3.0;

			if (tinyref->IsDead()) {
				Notify("Vore Task ended");
				return false;
			}

			return AttachTo(giantref, tinyref, coords);
		});
	}
	
	bool Vore_ShouldAttachToRHand(Actor* giant, Actor* tiny) {
		if (IsTransferingTiny(giant)) {
			Vore_AttachToRightHandTask(giant, tiny);
			return true;
		} else {
			return false;
		}
	}

	void UpdateFriendlyHugs(Actor* giant, Actor* tiny, bool force) {
		bool perk = Runtime::HasPerkTeam(giant, "HugCrush_LovingEmbrace");
		bool hostile = IsHostile(tiny, giant);
		bool teammate = IsTeammate(tiny);

		if (perk && !hostile && teammate && !force) {
			tiny->SetGraphVariableBool("GTS_IsFollower", true);
			giant->SetGraphVariableBool("GTS_HuggingTeammate", true);
		} else {
			tiny->SetGraphVariableBool("GTS_IsFollower", false);
			giant->SetGraphVariableBool("GTS_HuggingTeammate", false);
		}
		// This function determines the following:
		// Should the Tiny play "willing" or "Unwilling" hug idle?
	}

	void HugCrushOther(Actor* giant, Actor* tiny) {
		Attacked(tiny, giant);
		if (giant->formID == 0x14 && IsDragon(tiny)) {
			CompleteDragonQuest(tiny, false, tiny->IsDead());
		}
		float currentSize = get_visual_scale(tiny);

		ModSizeExperience(giant, 0.24); // Adjust Size Matter skill
		KillActor(giant, tiny);

		if (!IsLiving(tiny)) {
			SpawnDustParticle(tiny, tiny, "NPC Root [Root]", 3.6);
		} else {
			if (!LessGore()) {
				auto root = find_node(tiny, "NPC Root [Root]");
				if (root) {
					SpawnParticle(tiny, 0.20, "GTS/Damage/Explode.nif", NiMatrix3(), root->world.translate, 2.0, 7, root);
					SpawnParticle(tiny, 0.20, "GTS/Damage/Explode.nif", NiMatrix3(), root->world.translate, 2.0, 7, root);
					SpawnParticle(tiny, 0.20, "GTS/Damage/Explode.nif", NiMatrix3(), root->world.translate, 2.0, 7, root);
					SpawnParticle(tiny, 1.20, "GTS/Damage/ShrinkOrCrush.nif", NiMatrix3(), root->world.translate, get_visual_scale(tiny) * 10, 7, root);
				}
				Runtime::CreateExplosion(tiny, get_visual_scale(tiny)/4, "BloodExplosion");
				Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC Root [Root]", NiPoint3{0, 0, -1}, 512, false, true);
			} else {
				Runtime::PlaySound("BloodGushSound", tiny, 1.0, 0.5);
			}
		}

		AddSMTDuration(giant, 5.0);

		ApplyShakeAtNode(tiny, 20, "NPC Root [Root]", 20.0);

		ActorHandle giantHandle = giant->CreateRefHandle();
		ActorHandle tinyHandle = tiny->CreateRefHandle();
		std::string taskname = std::format("HugCrush {}", tiny->formID);

		TaskManager::RunOnce(taskname, [=](auto& update){
			if (!tinyHandle) {
				return;
			}
			if (!giantHandle) {
				return;
			}
			auto giant = giantHandle.get().get();
			auto tiny = tinyHandle.get().get();
			float scale = get_visual_scale(tiny);
			TransferInventory(tiny, giant, scale, false, true, DamageSource::Crushed, true);
		});
		if (tiny->formID != 0x14) {
			Disintegrate(tiny, true); // Set critical stage 4 on actor
		} else {
			TriggerScreenBlood(50);
			tiny->SetAlpha(0.0); // Player can't be disintegrated, so we make player Invisible
		}
		auto Node = find_node(giant, "NPC Spine2 [Spn2]"); 
		if (!Node) {
			Notify("Error: Spine2 [Spn2] node not found");
			return;
		}
		Runtime::PlaySoundAtNode("ShrinkToNothingSound", giant, 1.0, 1.0, "NPC Spine2 [Spn2]");
	}

	// Cancels all hug-related things
	void AbortHugAnimation(Actor* giant, Actor* tiny) {
		AnimationManager::StartAnim("Huggies_Spare", giant); // Start "Release" animation on Giant

		AdjustFacialExpression(giant, 0, 0.0, "phenome");
		AdjustFacialExpression(giant, 0, 0.0, "modifier");
		AdjustFacialExpression(giant, 1, 0.0, "modifier");

		bool Friendly;
		giant->GetGraphVariableBool("GTS_HuggingTeammate", Friendly);

		if (Friendly) { // If friendly, we don't want to push/release actor
			return; // GTS_Hug_Release event handles that in this case.
		}

		if (tiny) {
			EnableCollisions(tiny);
			SetBeingHeld(tiny, false);
			PushActorAway(giant, tiny, 1.0);
			UpdateFriendlyHugs(giant, tiny, true); // set GTS_IsFollower (tiny) and GTS_HuggingTeammate (GTS) bools to false
			Hugs_FixAnimationDesync(giant, tiny, true); // reset anim speed override so .dll won't use it
		}
		HugShrink::Release(giant);
		
	}

	void Utils_UpdateHugBehaviors(Actor* giant, Actor* tiny) { // blend between two anims: send value to behaviors
        float tinySize = get_visual_scale(tiny);
        float giantSize = get_visual_scale(giant);
        float size_difference = std::clamp(giantSize/tinySize, 1.0f, 3.0f);

		float OldMin = 1.0;
		float OldMax = 3.0;

		float NewMin = 0.0;
		float NewMax = 1.0;

		float OldValue = size_difference;
		float NewValue = (((OldValue - OldMin) * (NewMax - NewMin)) / (OldMax - OldMin)) + NewMin;

		tiny->SetGraphVariableFloat("GTS_SizeDifference", NewValue); // pass Tiny / Giant size diff POV to Tiny
		giant->SetGraphVariableFloat("GTS_SizeDifference", NewValue); // pass Tiny / Giant size diff POV to GTS
    }

	void StartHealingAnimation(Actor* giant, Actor* tiny) {
		UpdateFriendlyHugs(giant, tiny, false);
		AnimationManager::StartAnim("Huggies_Heal", giant);

		if (IsFemale(tiny)) {
			AnimationManager::StartAnim("Huggies_Heal_Victim_F", tiny);
		} else {
			AnimationManager::StartAnim("Huggies_Heal_Victim_M", tiny);
		}
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
		if (giant->formID == 0x14) {
			if (AllowFeetTracking()) {
				auto& sizemanager = SizeManager::GetSingleton();
				sizemanager.SetActionBool(giant, enable, type);
			}
		}
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
		WasteMult *= Perk_GetCostReduction(giant);

		float WasteStamina = (1.40 * WasteMult)/sizedifference * TimeScale();
		DamageAV(giant, ActorValue::kStamina, WasteStamina);
	}

	void DrainStamina(Actor* giant, std::string_view TaskName, std::string_view perk, bool decide, float power) {
		float WasteMult = 1.0;
		if (Runtime::HasPerkTeam(giant, perk)) {
			WasteMult -= 0.35;
		}
		WasteMult *= Perk_GetCostReduction(giant);

		std::string name = std::format("StaminaDrain_{}_{}", TaskName, giant->formID);
		if (decide) {
			ActorHandle GiantHandle = giant->CreateRefHandle();
			TaskManager::Run(name, [=](auto& progressData) {
				if (!GiantHandle) {
					return false;
				}
				auto GiantRef = GiantHandle.get().get();
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
		WasteMult *= Perk_GetCostReduction(giant);
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
								ActorHandle giantHandle = actor->CreateRefHandle();
								ActorHandle tinyHandle = otherActor->CreateRefHandle();
								std::string taskname = std::format("GrindCheckL_{}_{}", actor->formID, otherActor->formID);
								TaskManager::RunOnce(taskname, [=](auto& update){
									if (!tinyHandle) {
										return;
									}
									if (!giantHandle) {
										return;
									}
									
									auto giant = giantHandle.get().get();
									auto tiny = tinyHandle.get().get();

									if (aveForce >= 0.00 && !tiny->IsDead()) {
										if (!strong) {
											DoFootGrind_Left(giant, tiny, SMT);
											SetBeingGrinded(tiny, true);
											AnimationManager::StartAnim("GrindLeft", giant);
										} else {
											AnimationManager::StartAnim("TrampleStartL", giant);
											DoFootTrample_Left(giant, tiny, SMT);
										}
									}
								});
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
			if (IsDebugEnabled() && (actor->formID == 0x14 || IsTeammate(actor) || EffectsForEveryone(actor))) {
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
								ActorHandle giantHandle = actor->CreateRefHandle();
								ActorHandle tinyHandle = otherActor->CreateRefHandle();
								std::string taskname = std::format("GrindCheckR_{}_{}", actor->formID, otherActor->formID);
								TaskManager::RunOnce(taskname, [=](auto& update){
									if (!tinyHandle) {
										return;
									}
									if (!giantHandle) {
										return;
									}
									
									auto giant = giantHandle.get().get();
									auto tiny = tinyHandle.get().get();

									if (aveForce >= 0.00 && !tiny->IsDead()) {
										if (!strong) {
											DoFootGrind_Right(giant, tiny, SMT);
											SetBeingGrinded(tiny, true);
											AnimationManager::StartAnim("GrindRight", giant);
										} else {
											AnimationManager::StartAnim("TrampleStartR", giant);
											DoFootTrample_Right(giant, tiny, SMT);
										}
									}
								});
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
		}
		if (!giant) {
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
							float aveForce = std::clamp(force, 0.06f, 0.70f);
							float pushForce = std::clamp(force, 0.04f, 0.10f);
							float audio = 1.0;
							if (SMT) {
								pushForce *= 1.5;
								damage *= 2.25;
								audio = 3.0;
							}
							if (otherActor->IsDead()) {
								tinyScale *= 0.6;
							}
							
							if (giantScale / tinyScale > 2.25) {
								PushTowards(giant, otherActor, node, pushForce * pushpower, true);
							} else {
								Utils_PushCheck(giant, otherActor, aveForce);
							}
							float Volume = clamp(0.25, 1.0, (giantScale/tinyScale)*pushForce);

							auto node = find_node(giant, GetDeathNodeName(Cause));
							if (node) {
								Runtime::PlaySoundAtNode("SwingImpact", giant, Volume, 1.0, node); // play swing impact sound
							}

							
							ApplyShakeAtPoint(giant, 3.0 * pushpower * audio, node->world.translate, 1.5);
							sizemanager.GetDamageData(otherActor).lastHandDamageTime = Time::WorldTimeElapsed();
							CollisionDamage::GetSingleton().ApplySizeEffect(giant, otherActor, aveForce * damage, random, bbmult, crushmult, Cause);
						}
					}
				}
			}
		}
	}

	NiPoint3 GetHeartPosition(Actor* giant, Actor* tiny) { // used to spawn Heart Particles during healing hugs

		NiPoint3 TargetA = NiPoint3();
		NiPoint3 TargetB = NiPoint3();
		std::vector<std::string_view> bone_names = {
			"L Breast03",
			"R Breast03"
		};
		std::uint32_t bone_count = bone_names.size();
		for (auto bone_name_A: bone_names) {
			auto bone = find_node(giant, bone_name_A);
			if (!bone) {
				Notify("Error: Breast Nodes could not be found.");
				Notify("Actor without nodes: {}", giant->GetDisplayFullName());
				Notify("Suggestion: install XP32 skeleton.");
				return NiPoint3();
			}
			TargetA += (bone->world.translate) * (1.0/bone_count);
		}
		for (auto bone_name_B: bone_names) {
			auto bone = find_node(tiny, bone_name_B);
			if (!bone) {
				Notify("Error: Breast Nodes could not be found.");
				Notify("Actor without nodes: {}", tiny->GetDisplayFullName());
				Notify("Suggestion: install XP32 skeleton.");
				return NiPoint3();
			}
			TargetB += (bone->world.translate) * (1.0/bone_count);
		}

		auto targetPoint = (TargetA + TargetB) / 2;
		targetPoint.z += 45.0 * get_visual_scale(giant);
		return targetPoint;
	}


	float GetHugStealRate(Actor* actor) {
		float steal = 0.18;
		if (Runtime::HasPerkTeam(actor, "HugCrush_ToughGrip")) {
			steal += 0.072;
		}
		if (Runtime::HasPerkTeam(actor, "HugCrush")) {
			steal *= 1.35;
		}
		return steal;
	}

	float GetHugShrinkThreshold(Actor* actor) {
		float threshold = 2.5;
		float bonus = 1.0;
		if (Runtime::HasPerk(actor, "HugCrush")) {
			bonus += 0.25;
		}
		if (Runtime::HasPerk(actor, "HugCrush_Greed")) {
			bonus += 0.35;
		}
		if (HasGrowthSpurt(actor)) {
			bonus *= 2.0;
		}
		return threshold * bonus;
	}

	float GetHugCrushThreshold(Actor* actor) {
		float hp = 0.20;
		if (Runtime::HasPerkTeam(actor, "HugCrush_MightyCuddles")) {
			hp += 0.10; // 0.30
		}
		if (Runtime::HasPerkTeam(actor, "HugCrush_HugsOfDeath")) {
			hp += 0.20; // 0.50
		}
		return hp;
	}
}
