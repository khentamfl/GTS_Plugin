#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/GrabAnimationController.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/Attributes.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"

#include <random>

using namespace RE;
using namespace REL;
using namespace Gts;
using namespace std;

///GTS_GrabbedTiny MUST BE 1 when we have someone in hands

/*Event used in the behaviours to transition between most behaviour states
   Grab Events
        GTSBEH_GrabStart
        GTSBEH_GrabVore
        GTSBEH_GrabAttack
        GTSBEH_GrabThrow
        GTSBEH_GrabRelease
   More Grab things we don't need to do anything with in the DLL
        GTSBeh_MT
        GTSBeh_1hm
        GTSBeh_Mag
        GTSBeh_Set
        GTSBeh_GrabVore_LegTrans
   Used to leave the grab state
        GTSBeh_GrabExit
   Grab Event to go back to vanilla
        GTSBEH_AbortGrab
 */


namespace {

	const std::vector<std::string_view> RHAND_RUMBLE_NODES = { // used for hand rumble
		"NPC R UpperarmTwist1 [RUt1]",
		"NPC R UpperarmTwist2 [RUt2]",
		"NPC R Forearm [RLar]",
		"NPC R ForearmTwist2 [RLt2]",
		"NPC R ForearmTwist1 [RLt1]",
		"NPC R Hand [RHnd]",
	};

	const std::vector<std::string_view> LHAND_RUMBLE_NODES = { // used for hand rumble
		"NPC L UpperarmTwist1 [LUt1]",
		"NPC L UpperarmTwist2 [LUt2]",
		"NPC L Forearm [LLar]",
		"NPC L ForearmTwist2 [LLt2]",
		"NPC L ForearmTwist1 [LLt1]",
		"NPC L Hand [LHnd]",
	};

	const std::string_view RNode = "NPC R Foot [Rft ]";
	const std::string_view LNode = "NPC L Foot [Lft ]";

	void StartRHandRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: RHAND_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StartLHandRumble(std::string_view tag, Actor& actor, float power, float halflife) {
		for (auto& node_name: LHAND_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Start(rumbleName, &actor, power,  halflife, node_name);
		}
	}

	void StopRHandRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: RHAND_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Stop(rumbleName, &actor);
		}
	}
	void StopLHandRumble(std::string_view tag, Actor& actor) {
		for (auto& node_name: RHAND_RUMBLE_NODES) {
			std::string rumbleName = std::format("{}{}", tag, node_name);
			GRumble::Stop(rumbleName, &actor);
		}
	}

	bool Escaped(Actor* giant, Actor* tiny, float strength) {
		float tiny_chance = ((rand() % 100000) / 100000.0f) * get_visual_scale(tiny);
		float giant_chance = ((rand() % 100000) / 100000.0f) * strength * get_visual_scale(giant);
		return (tiny_chance > giant_chance);
	}

	void RotateActorTask(Actor* giant, Actor* tiny) {
		std::string name = std::format("RotateActor_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			auto TinyRef = skyrim_cast<TESObjectREFR*>(tinyref);
			if (!TinyRef) {
				return false;
			}

			float LPosX = 0.0f;
			float LPosY = 0.0f;
			float LPosZ = 0.0f;

			float RPosX = 0.0f;
			float RPosY = 0.0f;
			float RPosZ = 0.0f;

			float TinyX = 0.0f;
			float TinyY = 0.0f;
			float TinyZ = 0.0f;

			auto NPC = find_node(tiny, "CME LBody [LBody]");
			auto BreastL = find_node(giant, "L Breast02");
			auto BreastR = find_node(giant, "R Breast02");
			if (!NPC) {
				return false;
			}
			if (!BreastL) {
				return false;
			}
			if (!BreastR) {
				return false;
			}

			NiMatrix3 LeftBreastRotation = BreastL->world.rotate;
			NiMatrix3 RightBreastRotation = BreastR->world.rotate;

			LeftBreastRotation.ToEulerAnglesXYZ(LPosX, LPosY, LPosZ);
			RightBreastRotation.ToEulerAnglesXYZ(RPosX, RPosY, RPosZ);

			NiMatrix3 NPCROT = NPC->world.rotate;

			//NPCROT.SetEulerAnglesXYZ(NewRot); = NiPoint3((LPosX + RPosX) / 2, 0, giantref->data.angle.z);

			auto NewRot = NiPoint3(((LPosX + RPosX) * 70) / 2, 0, giantref->data.angle.z);
			auto Reset = NiPoint3(0, 0, 0);

			NPCROT.SetEulerAnglesXYZ(NewRot);
			update_node(NPC);

			auto RotationResult = NPCROT.ToEulerAnglesXYZ(TinyX, TinyY, TinyZ);

			//log::info("Angle of L breast: x: {}, y: {}, z: {}", LPosX, LPosY, LPosZ);
			//log::info("Angle of R breast: x: {}, y: {}, z: {}", RPosX, RPosY, RPosZ);

			// All good try another frame
			if (!IsBetweenBreasts(giantref)) {
				NPCROT.SetEulerAnglesXYZ(Reset);
				update_node(NPC);
				return false; // Abort it
			}
			return true;
		});
		TaskManager::ChangeUpdate(name, UpdateKind::Camera);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////G R A B
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GTSGrab_Catch_Start(AnimationEventData& data) {
		ManageCamera(&data.giant, true, 7.0);
		auto grabbedActor = Grab::GetHeldActor(&data.giant);
		if (grabbedActor) {
			DisableCollisions(grabbedActor, &data.giant);
			SetBeingHeld(grabbedActor, true);
		}
		StartLHandRumble("GrabL", data.giant, 0.5, 0.10);
	}

	void GTSGrab_Catch_Actor(AnimationEventData& data) {
		auto giant = &data.giant;
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 1);
		auto grabbedActor = Grab::GetHeldActor(&data.giant);
		if (grabbedActor) {
			Grab::AttachActorTask(giant, grabbedActor);
			DisableCollisions(grabbedActor, &data.giant); // Just to be sure
			if (!IsTeammate(grabbedActor)) {
				Attacked(grabbedActor, giant);
			}
		}
		GRumble::Once("GrabCatch", giant, 2.0, 0.15);
	}

	void GTSGrab_Catch_End(AnimationEventData& data) {
		ManageCamera(&data.giant, false, 7.0);
		StopLHandRumble("GrabL", data.giant);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////A T T A C K
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GTSGrab_Attack_MoveStart(AnimationEventData& data) {
		auto giant = &data.giant;
		DrainStamina(giant, "GrabAttack", "DestructionBasics", true, 0.75);
		ManageCamera(giant, true, 7.0);
		StartLHandRumble("GrabMoveL", data.giant, 0.5, 0.10);
	}

	void GTSGrab_Attack_Damage(AnimationEventData& data) {
		auto& sizemanager = SizeManager::GetSingleton();
		float bonus = 1.0;
		auto giant = &data.giant;
		auto grabbedActor = Grab::GetHeldActor(giant);

		if (grabbedActor) {
			Attacked(grabbedActor, giant); // force combat
			float sizeDiff = get_visual_scale(giant)/get_visual_scale(grabbedActor);
			float power = std::clamp(sizemanager.GetSizeAttribute(giant, 0), 1.0f, 999999.0f);
			float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(grabbedActor);
			float damage = (1.600 * sizeDiff) * power * additionaldamage * additionaldamage;
			float experience = std::clamp(damage/800, 0.0f, 0.06f);
			if (HasSMT(giant)) {
				damage *= 1.60;
				bonus = 3.0;
			}

			InflictSizeDamage(giant, grabbedActor, damage);

			GRumble::Once("GrabAttack", giant, 5.0 * bonus, 0.05, "NPC L Hand [LHnd]");

			SizeHitEffects::GetSingleton().BreakBones(giant, grabbedActor, 0, 1); // don't do damage and just add flat debuff
			SizeHitEffects::GetSingleton().BreakBones(giant, grabbedActor, 0, 1); // do it twice

			ModSizeExperience(experience, giant);
			AddSMTDuration(giant, 1.0);


			std::string taskname = std::format("GrabCrush_{}", grabbedActor->formID);
			auto tinyref = grabbedActor->CreateRefHandle();
			auto giantref = giant->CreateRefHandle();
			TaskManager::RunOnce(taskname, [=](auto& update) {
				if (!tinyref || !giantref) {
					return;
				}
				auto tiny = tinyref.get().get();
				auto giantess = giantref.get().get();
				if (GetAV(tiny, ActorValue::kHealth) <= 1.0 || tiny->IsDead()) {
					CrushManager::Crush(giantess, tiny);
					ModSizeExperience(0.14, giantess);
					SetBeingHeld(tiny, false);
					GRumble::Once("GrabAttackKill", giantess, 14.0 * bonus, 0.15, "NPC L Hand [LHnd]");
					if (!LessGore()) {
						Runtime::PlaySoundAtNode("CrunchImpactSound", giantess, 1.0, 0.0, "NPC L Hand [LHnd]");
						Runtime::PlaySoundAtNode("CrunchImpactSound", giantess, 1.0, 0.0, "NPC L Hand [LHnd]");
						Runtime::PlaySoundAtNode("CrunchImpactSound", giantess, 1.0, 0.0, "NPC L Hand [LHnd]");
					} else {
						Runtime::PlaySoundAtNode("SoftHandAttack", giantess, 1.0, 0.0, "NPC L Hand [LHnd]");
					}
					Runtime::PlaySoundAtNode("GtsCrushSound", giantess, 1.0, 1.0, "NPC L Hand [LHnd]");
					SetBetweenBreasts(giantess, false);
					AdjustSizeReserve(giantess, get_visual_scale(tiny)/10);
					AdvanceQuestProgression(giantess, tiny, 5, 1.0, false);
					SpawnHurtParticles(giantess, tiny, 3.0, 1.6);
					SpawnHurtParticles(giantess, tiny, 3.0, 1.6);
					PrintDeathSource(giantess, tiny, DamageSource::HandCrushed);
					Grab::DetachActorTask(giantess);
					Grab::Release(giantess);
				} else {
					if (!LessGore()) {
						Runtime::PlaySoundAtNode("CrunchImpactSound", giantess, 1.0, 0.0, "NPC L Hand [LHnd]");
						SpawnHurtParticles(giantess, tiny, 1.0, 1.0);
					} else {
						Runtime::PlaySoundAtNode("SoftHandAttack", giantess, 1.0, 0.0, "NPC L Hand [LHnd]");
					}
					StaggerActor(tiny, 0.25f);
				}
			});
		}
	}

	void GTSGrab_Attack_MoveStop(AnimationEventData& data) {
		auto giant = &data.giant;
		auto& sizemanager = SizeManager::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(giant);
		ManageCamera(giant, false, 7.0);
		DrainStamina(giant, "GrabAttack", "DestructionBasics", false, 0.75);
		StopLHandRumble("GrabMoveL", data.giant);
		if (!grabbedActor) {
			giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
			giant->SetGraphVariableInt("GTS_Grab_State", 0);
			AnimationManager::StartAnim("GrabAbort", giant);
			AnimationManager::StartAnim("TinyDied", giant);
			Grab::DetachActorTask(giant);
			Grab::Release(giant);
			return;
		}
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////V O R E
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GTSGrab_Eat_Start(AnimationEventData& data) {
		ToggleEmotionEdit(&data.giant, true);
		auto otherActor = Grab::GetHeldActor(&data.giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		ManageCamera(&data.giant, true, 7.0);
		if (otherActor) {
			VoreData.AddTiny(otherActor);
		}
		StartLHandRumble("GrabVoreL", data.giant, 0.5, 0.10);
	}

	void GTSGrab_Eat_OpenMouth(AnimationEventData& data) {
		auto giant = &data.giant;
		auto otherActor = Grab::GetHeldActor(giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(giant);
		if (otherActor) {
			SetBeingEaten(otherActor, true);
		}
		AdjustFacialExpression(giant, 0, 1.0, "phenome"); // Start opening mouth
		AdjustFacialExpression(giant, 1, 0.5, "phenome"); // Open it wider

		AdjustFacialExpression(giant, 0, 0.80, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.80, "modifier"); // blink R

		AdjustFacialExpression(&data.giant, 3, 0.8, "phenome"); // Smile a bit (Mouth)
		StopLHandRumble("GrabVoreL", data.giant);
	}

	void GTSGrab_Eat_Eat(AnimationEventData& data) {
		auto otherActor = Grab::GetHeldActor(&data.giant);
		auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
		if (otherActor) {
			for (auto& tiny: VoreData.GetVories()) {
				if (!AllowDevourment()) {
					VoreData.Swallow();
					if (IsCrawling(&data.giant)) {
						otherActor->SetAlpha(0.0); // Hide Actor
					}
				} else {
					CallDevourment(&data.giant, otherActor);
				}
			}
		}
	}

	void GTSGrab_Eat_CloseMouth(AnimationEventData& data) {
		auto giant = &data.giant;
		AdjustFacialExpression(giant, 0, 0.0, "phenome"); // Close mouth
		AdjustFacialExpression(giant, 1, 0.0, "phenome"); // Close it

		AdjustFacialExpression(giant, 0, 0.0, "modifier"); // blink L
		AdjustFacialExpression(giant, 1, 0.0, "modifier"); // blink R

		AdjustFacialExpression(&data.giant, 3, 0.0, "phenome"); // Smile a bit (Mouth)
	}

	void GTSGrab_Eat_Swallow(AnimationEventData& data) {
		auto giant = &data.giant;
		auto otherActor = Grab::GetHeldActor(&data.giant);
		if (otherActor) {
			SetBeingEaten(otherActor, false);
			auto& VoreData = Vore::GetSingleton().GetVoreData(&data.giant);
			for (auto& tiny: VoreData.GetVories()) {
				VoreData.KillAll();
			}
			giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
			giant->SetGraphVariableInt("GTS_Grab_State", 0);
			Runtime::PlaySoundAtNode("VoreSwallow", &data.giant, 1.0, 1.0, "NPC Head [Head]"); // Play sound
			AnimationManager::StartAnim("TinyDied", giant);
			//BlockFirstPerson(giant, false);
			ManageCamera(&data.giant, false, 7.0);
			SetBeingHeld(otherActor, false);
			Grab::DetachActorTask(giant);
			Grab::Release(giant);
		}
		ToggleEmotionEdit(giant, false);
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////T H R O W
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GTSGrab_Throw_MoveStart(AnimationEventData& data) {
		auto giant = &data.giant;
		DrainStamina(giant, "GrabThrow", "DestructionBasics", true, 1.25);
		ManageCamera(giant, true, 7.0);
		StartLHandRumble("GrabThrowL", data.giant, 0.5, 0.10);
	}

	void GTSGrab_Throw_FS_R(AnimationEventData& data) {
		if (IsUsingThighAnimations(&data.giant) || IsCrawling(&data.giant)) {
			return; // Needed to not apply it during animation blending for thigh/crawling animations
		}
		float shake = 1.0;
		float launch = 1.0;
		float dust = 0.9;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.5;
			dust = 1.25;
		}
		GRumble::Once("StompR", &data.giant, 1.50 * shake, 0.0, RNode);
		DoDamageEffect(&data.giant, 1.1 * launch * data.animSpeed * perk, 1.0 * launch * data.animSpeed, 10, 0.20, FootEvent::Right, 1.0, DamageSource::CrushedRight);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Right, RNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Right, RNode);
		DoLaunch(&data.giant, 0.75 * perk, 1.25, FootEvent::Right);
	}

	void GTSGrab_Throw_FS_L(AnimationEventData& data) {
		if (IsUsingThighAnimations(&data.giant) || IsCrawling(&data.giant)) {
			return; // Needed to not apply it during animation blending for thigh/crawling animations
		}
		float shake = 1.0;
		float launch = 1.0;
		float dust = 0.9;
		float perk = GetPerkBonus_Basics(&data.giant);
		if (HasSMT(&data.giant)) {
			shake = 4.0;
			launch = 1.5;
			dust = 1.25;
		}
		GRumble::Once("StompL", &data.giant, 1.50 * shake, 0.0, LNode);
		DoDamageEffect(&data.giant, 1.1 * launch * data.animSpeed * perk, 1.0 * launch * data.animSpeed, 10, 0.20, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
		DoFootstepSound(&data.giant, 1.0, FootEvent::Left, LNode);
		DoDustExplosion(&data.giant, dust, FootEvent::Left, LNode);
		DoLaunch(&data.giant, 0.75 * perk, 1.25, FootEvent::Left);
	}

	void GTSGrab_Throw_Throw_Pre(AnimationEventData& data) {// Throw frame 0
		auto giant = &data.giant;
		auto otherActor = Grab::GetHeldActor(&data.giant);

		NiPoint3 startThrow = otherActor->GetPosition();
		double startTime = Time::WorldTimeElapsed();
		ActorHandle tinyHandle = otherActor->CreateRefHandle();
		ActorHandle gianthandle = giant->CreateRefHandle();

		Grab::DetachActorTask(giant);
		Grab::Release(giant);

		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		giant->SetGraphVariableInt("GTS_Grab_State", 0);

		auto charcont = otherActor->GetCharController();
		if (charcont) {
			charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
		}

		// Do this next frame (or rather until some world time has elapsed)
		TaskManager::Run([=](auto& update){
			Actor* giant = gianthandle.get().get();
			Actor* tiny = tinyHandle.get().get();
			if (!giant) {
				return false;
			}
			if (!tiny) {
				return false;
			}
			// Wait for 3D to be ready
			if (!giant->Is3DLoaded()) {
				return true;
			}
			if (!giant->GetCurrent3D()) {
				return true;
			}
			if (!tiny->Is3DLoaded()) {
				return true;
			}
			if (!tiny->GetCurrent3D()) {
				return true;
			}

			NiPoint3 endThrow = tiny->GetPosition();
			double endTime = Time::WorldTimeElapsed();

			if ((endTime - startTime) > 1e-4) {
				// Time has elapsed
				SetBeingHeld(tiny, false);
				EnableCollisions(tiny);

				NiPoint3 vector = endThrow - startThrow;
				float distanceTravelled = vector.Length();
				float timeTaken = endTime - startTime;
				float speed = distanceTravelled / timeTaken;
				if (!IsCrawling(giant)) {
					log::info("Not Crawling");
				}
				// NiPoint3 direction = vector / vector.Length();

				// Angles in degrees
				// Sermit: Please just adjust these



				float angle_x = 60;//Runtime::GetFloat("cameraAlternateX"); // 60
				float angle_y = 10; //Runtime::GetFloat("cameraAlternateY");//10.0;
				float angle_z = 0;//::GetFloat("combatCameraAlternateX"); // 0

				// Conversion to radians
				const float PI = 3.141592653589793;
				float angle_x_rad = angle_x * 180.0 / PI;
				float angle_y_rad = angle_y * 180.0 / PI;
				float angle_z_rad = angle_z * 180.0 / PI;

				// Work out direction from angles and an initial (forward) vector;
				//
				// If all angles are zero then it goes forward
				// angle_x is pitch
				// angle_y is yaw
				// angle_z is roll
				//
				// The order of operation is pitch > yaw > roll
				NiMatrix3 customRot = NiMatrix3(angle_x_rad, angle_y_rad, angle_z_rad);
				NiPoint3 forward = NiPoint3(0.0, 0.0, 1.0);
				NiPoint3 customDirection = customRot * forward;

				// Convert to giant local space
				// Only use rotation not translaion or scale since those will mess everything up
				NiMatrix3 giantRot = giant->GetCurrent3D()->world.rotate;
				NiPoint3 direction = giantRot * (customDirection / customDirection.Length());
				//log::info("forward : {}", Vector2Str(forward));
				//log::info("customDirection : {}", Vector2Str(customDirection));
				//log::info("Direction : {}", Vector2Str(direction));
				//log::info("Speed: {}", Runtime::GetFloat("cameraAlternateX") * 100);

				//PushActorAway(giant, tiny, direction, speed * 100);
				PushActorAway(giant, tiny, 1);
				//ApplyHavokImpulse(tiny, direction.x, direction.y, direction.z, Runtime::GetFloat("cameraAlternateX") * 100);//speed * 100);
				return false;
			} else {
				return true;
			}
		});
	}

	void GTSGrab_Throw_ThrowActor(AnimationEventData& data) { // Throw frame 1
		auto giant = &data.giant;
		auto otherActor = Grab::GetHeldActor(&data.giant);

		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		giant->SetGraphVariableInt("GTS_Grab_State", 0);
		ManageCamera(giant, false, 7.0);
		GRumble::Once("ThrowFoe", &data.giant, 2.50, 0.10, "NPC L Hand [LHnd]");
		AnimationManager::StartAnim("TinyDied", giant);
		//BlockFirstPerson(giant, false);
		Grab::DetachActorTask(giant);
		Grab::Release(giant);
	}

	void GTSGrab_Throw_Throw_Post(AnimationEventData& data) { // Throw frame 2
	}

	void GTSGrab_Throw_MoveStop(AnimationEventData& data) { // Throw Frame 3
		auto giant = &data.giant;
		DrainStamina(giant, "GrabThrow", "DestructionBasics", false, 1.25);
		StopLHandRumble("GrabThrowL", data.giant);
	}


	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////R E L E A S E
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GTSGrab_Release_FreeActor(AnimationEventData& data) {
		auto giant = &data.giant;
		SetBetweenBreasts(giant, false);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		giant->SetGraphVariableInt("GTS_Storing_Tiny", 0);
		giant->SetGraphVariableInt("GTS_Grab_State", 0);
		auto grabbedActor = Grab::GetHeldActor(giant);
		ManageCamera(&data.giant, false, 7.0);
		AnimationManager::StartAnim("TinyDied", giant);
		//BlockFirstPerson(giant, false);
		if (grabbedActor) {
			PushActorAway(giant, grabbedActor, 1.0);
			EnableCollisions(grabbedActor);
			SetBeingHeld(grabbedActor, false);
		}
		Grab::DetachActorTask(giant);
		Grab::Release(giant);
	}

	void GTSBEH_GrabExit(AnimationEventData& data) {
		auto giant = &data.giant;
		auto grabbedActor = Grab::GetHeldActor(giant);
		if (grabbedActor) {
			EnableCollisions(grabbedActor);
		}
		SetBetweenBreasts(giant, false);

		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		giant->SetGraphVariableInt("GTS_Storing_Tiny", 0);
		giant->SetGraphVariableInt("GTS_Grab_State", 0);
		AnimationManager::StartAnim("TinyDied", giant);
		DrainStamina(giant, "GrabAttack", "DestructionBasics", false, 0.75);
		DrainStamina(giant, "GrabThrow", "DestructionBasics", false, 1.25);
		ManageCamera(&data.giant, false, 7.0);
		Grab::DetachActorTask(giant);
		Grab::Release(giant);
	}

	void GTSBEH_AbortGrab(AnimationEventData& data) {
		auto giant = &data.giant;
		auto grabbedActor = Grab::GetHeldActor(giant);
		if (grabbedActor) {
			EnableCollisions(grabbedActor);
			SetBeingHeld(grabbedActor, false);
		}
		SetBetweenBreasts(giant, false);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		giant->SetGraphVariableInt("GTS_Storing_Tiny", 0);
		giant->SetGraphVariableInt("GTS_Grab_State", 0);

		AnimationManager::StartAnim("TinyDied", giant);
		DrainStamina(giant, "GrabAttack", "DestructionBasics", false, 0.75);
		DrainStamina(giant, "GrabThrow", "DestructionBasics", false, 1.25);
		ManageCamera(&data.giant, false, 7.0);
		Grab::DetachActorTask(giant);
		Grab::Release(giant);
	}

	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////B R E A S T S
	////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GTSGrab_Breast_MoveStart(AnimationEventData& data) {
		ManageCamera(&data.giant, true, 7.0);
	}

	void GTSGrab_Breast_PutActor(AnimationEventData& data) { // Places actor between breasts
		auto giant = &data.giant;
		SetBetweenBreasts(giant, true);
		Runtime::PlaySoundAtNode("BreastImpact", giant, 1.0, 0.0, "NPC L Hand [LHnd]");
		giant->SetGraphVariableInt("GTS_Storing_Tiny", 1);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0);
		auto otherActor = Grab::GetHeldActor(giant);
		if (otherActor) {
			otherActor->SetGraphVariableBool("GTSBEH_T_InStorage", true);
			if (IsHostile(giant, otherActor)) {
				AnimationManager::StartAnim("Breasts_Idle_Unwilling", otherActor);
			} else {
				AnimationManager::StartAnim("Breasts_Idle_Willing", otherActor);
			}
		}
	}

	void GTSGrab_Breast_TakeActor(AnimationEventData& data) { // Removes Actor
		auto giant = &data.giant;
		SetBetweenBreasts(giant, false);
		giant->SetGraphVariableInt("GTS_Storing_Tiny", 0);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 1);
		auto otherActor = Grab::GetHeldActor(giant);
		if (otherActor) {
			otherActor->SetGraphVariableBool("GTSBEH_T_InStorage", false);
			//BlockFirstPerson(giant, true);
			AnimationManager::StartAnim("Breasts_FreeOther", otherActor);
		}
	}

	void GTSGrab_Breast_MoveEnd(AnimationEventData& data) {
		ManageCamera(&data.giant, false, 7.0);
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	/////////////////////////////E V E N T S
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	void GrabOtherEvent(const InputEventData& data) { // Grab other actor
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (grabbedActor) { //If we have actor, don't pick anyone up.
			return;
		}
		if (!CanPerformAnimation(player, 2)) {
			return;
		}
		if (IsGtsBusy(player) || IsEquipBusy(player) || IsTransitioning(player)) {
			return; // Disallow Grabbing if Behavior is busy doing other stuff.
		}
		auto& Grabbing = GrabAnimationController::GetSingleton();
		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerkTeam(player, "MassVorePerk")) {
			numberOfPrey = 1 + (get_visual_scale(player)/3);
		}
		std::vector<Actor*> preys = Grabbing.GetGrabTargetsInFront(player, numberOfPrey);
		for (auto prey: preys) {
			Grabbing.StartGrab(player, prey);
		}
	}

	void GrabAttackEvent(const InputEventData& data) { // Attack everyone in your hand
		auto player = PlayerCharacter::GetSingleton();
		if (IsGtsBusy(player)) {
			return;
		}
		if (!IsStomping(player) && !IsTransitioning(player)) {
			auto grabbedActor = Grab::GetHeldActor(player);
			if (!grabbedActor) {
				return;
			}
			float WasteStamina = 20.0;
			if (Runtime::HasPerk(player, "DestructionBasics")) {
				WasteStamina *= 0.65;
			}
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				AnimationManager::StartAnim("GrabDamageAttack", player);
			} else {
				TiredSound(player, "You're too tired to perform hand attack");
			}
		}
	}

	void GrabVoreEvent(const InputEventData& data) { // Eat everyone in hand
		auto player = PlayerCharacter::GetSingleton();
		if (!CanPerformAnimation(player, 3)) {
			return;
		}
		if (!IsGtsBusy(player) && !IsTransitioning(player)) {
			auto grabbedActor = Grab::GetHeldActor(player);
			if (!grabbedActor) {
				return;
			}
			if (IsInsect(grabbedActor, true) || IsBlacklisted(grabbedActor) || IsUndead(grabbedActor)) {
				return; // Same rules as with Vore
			}
			AnimationManager::StartAnim("GrabEatSomeone", player);
		}
	}

	void GrabThrowEvent(const InputEventData& data) { // Throw everyone away
		auto player = PlayerCharacter::GetSingleton();
		if (!IsGtsBusy(player) && !IsTransitioning(player)) { // Only allow outside of GtsBusy and when not transitioning
			auto grabbedActor = Grab::GetHeldActor(player);
			if (!grabbedActor) {
				return;
			}
			float WasteStamina = 40.0;
			if (Runtime::HasPerk(player, "DestructionBasics")) {
				WasteStamina *= 0.65;
			}
			if (GetAV(player, ActorValue::kStamina) > WasteStamina) {
				AnimationManager::StartAnim("GrabThrowSomeone", player);
			} else {
				TiredSound(player, "You're too tired to throw that actor");
			}
		}
	}

	void GrabReleaseEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor) {
			return;
		}
		if (IsGtsBusy(player) || IsTransitioning(player)) {
			return;
		}
		AnimationManager::StartAnim("GrabReleasePunies", player);
	}

	void BreastsPutEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor || IsTransitioning(player)) {
			return;
		}
		AnimationManager::StartAnim("Breasts_Put", player);
	}
	void BreastsRemoveEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player);
		if (!grabbedActor || IsTransitioning(player)) {
			return;
		}
		AnimationManager::StartAnim("Breasts_Pull", player);
	}
}










namespace Gts {
	Grab& Grab::GetSingleton() noexcept {
		static Grab instance;
		return instance;
	}

	std::string Grab::DebugName() {
		return "Grab";
	}

	void Grab::DetachActorTask(Actor* giant) {
		std::string name = std::format("GrabAttach_{}", giant->formID);
		giant->SetGraphVariableInt("GTS_GrabbedTiny", 0); // Tell behaviors 'we have nothing in our hands'. A must.
		giant->SetGraphVariableInt("GTS_Grab_State", 0);
		giant->SetGraphVariableInt("GTS_Storing_Tiny", 0);
		SetBetweenBreasts(giant, false);
		TaskManager::Cancel(name);
	}

	void Grab::AttachActorTask(Actor* giant, Actor* tiny) {
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		std::string name = std::format("GrabAttach_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			if (!tinyref) {
				return false; // end task in that case
			}

			// Exit on death
			float sizedifference = get_target_scale(giantref)/get_target_scale(tinyref);

			ForceRagdoll(tinyref, false);

			ShutUp(tinyref);

			if (giantref->IsDead() || tinyref->IsDead() || GetAV(tinyref, ActorValue::kHealth) <= 0.0 || sizedifference < 6.0 || GetAV(giantref, ActorValue::kStamina) < 2.0) {
				PushActorAway(giantref, tinyref, 1.0);
				tinyref->SetGraphVariableBool("GTSBEH_T_InStorage", false);
				SetBetweenBreasts(giantref, false);
				SetBeingHeld(tinyref, false);
				giantref->SetGraphVariableInt("GTS_GrabbedTiny", 0); // Tell behaviors 'we have nothing in our hands'. A must.
				giantref->SetGraphVariableInt("GTS_Grab_State", 0);
				giantref->SetGraphVariableInt("GTS_Storing_Tiny", 0);
				DrainStamina(giant, "GrabAttack", "DestructionBasics", false, 0.75);
				AnimationManager::StartAnim("GrabAbort", giantref); // Abort Grab animation
				AnimationManager::StartAnim("TinyDied", giantref);
				ManageCamera(giantref, false, 7.0); // Disable any camera edits
				Grab::Release(giantref);
				return false;
			}

			if (IsBeingEaten(tinyref)) {
				if (!AttachToObjectA(gianthandle, tinyhandle)) {
					// Unable to attach
					return false;
				}
			} else if (IsBetweenBreasts(giantref)) {
				bool hostile = IsHostile(giantref, tinyref);
				float restore = 0.04 * TimeScale();
				if (!hostile) {
					tinyref->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, restore);
					tinyref->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kStamina, restore);
				}
				if (hostile) {
					DamageAV(tinyref, ActorValue::kStamina, restore * 2);
				}
				if (!AttachToCleavage(gianthandle, tinyhandle)) {
					// Unable to attach
					Grab::Release(giantref);
					return false;
				}
			} else if (AttachToHand(gianthandle, tinyhandle)) {
				GrabStaminaDrain(giantref, tinyref, sizedifference);
				return true;
			} else {
				if (!AttachToHand(gianthandle, tinyhandle)) {
					// Unable to attach
					return false;
				}
			}

			// All good try another frame
			return true;
		});
		TaskManager::ChangeUpdate(name, UpdateKind::Camera);
	}


	void Grab::GrabActor(Actor* giant, TESObjectREFR* tiny, float strength) {
		Grab::GetSingleton().data.try_emplace(giant, tiny, strength);
	}
	void Grab::GrabActor(Actor* giant, TESObjectREFR* tiny) {
		// Default strength 1.0: normal grab for actor of their size
		//
		Grab::GrabActor(giant, tiny, 1.0);
	}

	void Grab::Reset() {
		this->data.clear();
	}

	void Grab::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	void Grab::Release(Actor* giant) {
		Grab::GetSingleton().data.erase(giant);
	}

	TESObjectREFR* Grab::GetHeldObj(Actor* giant) {
		try {
			auto& me = Grab::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}

	}
	Actor* Grab::GetHeldActor(Actor* giant) {
		auto obj = Grab::GetHeldObj(giant);
		Actor* actor = skyrim_cast<Actor*>(obj);
		if (actor) {
			return actor;
		} else {
			return nullptr;
		}
	}

	void Grab::RegisterEvents() {
		InputManager::RegisterInputEvent("GrabOther", GrabOtherEvent);
		InputManager::RegisterInputEvent("GrabAttack", GrabAttackEvent);
		InputManager::RegisterInputEvent("GrabVore", GrabVoreEvent);
		InputManager::RegisterInputEvent("GrabThrow", GrabThrowEvent);
		InputManager::RegisterInputEvent("GrabRelease", GrabReleaseEvent);
		InputManager::RegisterInputEvent("BreastsPut", BreastsPutEvent);
		InputManager::RegisterInputEvent("BreastsRemove", BreastsRemoveEvent);

		AnimationManager::RegisterEvent("GTSGrab_Catch_Start", "Grabbing", GTSGrab_Catch_Start);
		AnimationManager::RegisterEvent("GTSGrab_Catch_Actor", "Grabbing", GTSGrab_Catch_Actor);
		AnimationManager::RegisterEvent("GTSGrab_Catch_End", "Grabbing", GTSGrab_Catch_End);

		AnimationManager::RegisterEvent("GTSGrab_Attack_MoveStart", "Grabbing", GTSGrab_Attack_MoveStart);
		AnimationManager::RegisterEvent("GTSGrab_Attack_Damage", "Grabbing", GTSGrab_Attack_Damage);
		AnimationManager::RegisterEvent("GTSGrab_Attack_MoveStop", "Grabbing", GTSGrab_Attack_MoveStop);

		AnimationManager::RegisterEvent("GTSGrab_Eat_Start", "Grabbing", GTSGrab_Eat_Start);
		AnimationManager::RegisterEvent("GTSGrab_Eat_OpenMouth", "Grabbing", GTSGrab_Eat_OpenMouth);
		AnimationManager::RegisterEvent("GTSGrab_Eat_Eat", "Grabbing", GTSGrab_Eat_Eat);
		AnimationManager::RegisterEvent("GTSGrab_Eat_CloseMouth", "Grabbing", GTSGrab_Eat_CloseMouth);
		AnimationManager::RegisterEvent("GTSGrab_Eat_Swallow", "Grabbing", GTSGrab_Eat_Swallow);

		AnimationManager::RegisterEvent("GTSGrab_Throw_MoveStart", "Grabbing", GTSGrab_Throw_MoveStart);
		AnimationManager::RegisterEvent("GTSGrab_Throw_FS_R", "Grabbing", GTSGrab_Throw_FS_R);
		AnimationManager::RegisterEvent("GTSGrab_Throw_FS_L", "Grabbing", GTSGrab_Throw_FS_L);
		AnimationManager::RegisterEvent("GTSGrab_Throw_Throw_Pre", "Grabbing", GTSGrab_Throw_Throw_Pre);
		AnimationManager::RegisterEvent("GTSGrab_Throw_ThrowActor", "Grabbing", GTSGrab_Throw_ThrowActor);
		AnimationManager::RegisterEvent("GTSGrab_Throw_Throw_Post", "Grabbing", GTSGrab_Throw_Throw_Post);
		AnimationManager::RegisterEvent("GTSGrab_Throw_MoveStop", "Grabbing", GTSGrab_Throw_MoveStop);

		AnimationManager::RegisterEvent("GTSGrab_Breast_MoveStart", "Grabbing", GTSGrab_Breast_MoveStart);
		AnimationManager::RegisterEvent("GTSGrab_Breast_PutActor", "Grabbing", GTSGrab_Breast_PutActor);
		AnimationManager::RegisterEvent("GTSGrab_Breast_TakeActor", "Grabbing", GTSGrab_Breast_TakeActor);
		AnimationManager::RegisterEvent("GTSGrab_Breast_MoveEnd", "Grabbing", GTSGrab_Breast_MoveEnd);

		AnimationManager::RegisterEvent("GTSGrab_Release_FreeActor", "Grabbing", GTSGrab_Release_FreeActor);

		AnimationManager::RegisterEvent("GTSBEH_GrabExit", "Grabbing", GTSBEH_GrabExit);
		AnimationManager::RegisterEvent("GTSBEH_AbortGrab", "Grabbing", GTSBEH_AbortGrab);
	}

	void Grab::RegisterTriggers() {
		AnimationManager::RegisterTrigger("GrabSomeone", "Grabbing", "GTSBEH_GrabStart");
		AnimationManager::RegisterTrigger("GrabEatSomeone", "Grabbing", "GTSBEH_GrabVore");
		AnimationManager::RegisterTrigger("GrabDamageAttack", "Grabbing", "GTSBEH_GrabAttack");
		AnimationManager::RegisterTrigger("GrabThrowSomeone", "Grabbing", "GTSBEH_GrabThrow");
		AnimationManager::RegisterTrigger("GrabReleasePunies", "Grabbing", "GTSBEH_GrabRelease");
		AnimationManager::RegisterTrigger("GrabExit", "Grabbing", "GTSBEH_GrabExit");
		AnimationManager::RegisterTrigger("GrabAbort", "Grabbing", "GTSBEH_AbortGrab");
		AnimationManager::RegisterTrigger("TinyDied", "Grabbing", "GTSBEH_TinyDied");
		AnimationManager::RegisterTrigger("Breasts_Put", "Grabbing", "GTSBEH_BreastsAdd");
		AnimationManager::RegisterTrigger("Breasts_Pull", "Grabbing", "GTSBEH_BreastsRemove");
		AnimationManager::RegisterTrigger("Breasts_Idle_Unwilling", "Grabbing", "GTSBEH_T_Storage_Enemy");
		AnimationManager::RegisterTrigger("Breasts_Idle_Willing", "Grabbing", "GTSBEH_T_Storage_Ally");
		AnimationManager::RegisterTrigger("Breasts_FreeOther", "Grabbing", "GTSBEH_T_Remove");

	}

	GrabData::GrabData(TESObjectREFR* tiny, float strength) : tiny(tiny), strength(strength) {
	}
}


//Beh's:
/*
        GTSBEH_GrabStart
        GTSBEH_GrabVore
        GTSBEH_GrabAttack
        GTSBEH_GrabThrow
        GTSBEH_GrabRelease

        GTSBeh_GrabExit
        GTSBEH_AbortGrab (Similar to GTSBEH_Exit but Grab only)
 */
