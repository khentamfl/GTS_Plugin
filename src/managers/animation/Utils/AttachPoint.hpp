#pragma once
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace RE;

namespace Gts {

	template<typename T, typename U>
	bool AttachTo(T& anyGiant, U& anyTiny, NiPoint3 point) {
		Actor* giant =  GetActorPtr(anyGiant);
		Actor* tiny =  GetActorPtr(anyTiny);

		if (!giant) {
			return false;
		}
		if (!tiny) {
			return false;
		}

		tiny->SetPosition(point, true);

		auto charcont = tiny->GetCharController();
		if (charcont) {
			charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
		}

		return true;
	}
	template<typename T, typename U>
	bool AttachTo(T& anyGiant, U& anyTiny, std::string_view bone_name) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}
		auto bone = find_node(giant, bone_name);
		if (!bone) {
			return false;
		}
		return AttachTo(anyGiant, anyTiny, bone->world.translate);
	}

	template<typename T, typename U>
	bool AttachToObjectA(T& anyGiant, U& anyTiny) {
		return AttachTo(anyGiant, anyTiny, "AnimObjectA");
	}

	template<typename T, typename U>
	bool AttachToObjectB(T& anyGiant, U& anyTiny) {
		return AttachTo(anyGiant, anyTiny, "AnimObjectB");
	}

	template<typename T, typename U>
	bool AttachToHand(T& anyGiant, U& anyTiny) {
		return AttachTo(anyGiant, anyTiny, "NPC L Finger02 [LF02]");
	}

	template<typename T, typename U>
	bool HugAttach(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}
		Actor* tiny = GetActorPtr(anyTiny);
		if (!tiny) {
			return false;
		}
		auto targetRootA = find_node(giant, "AnimObjectA");
		if (!targetRootA) {
			return false;
		}
    auto targetA = targetRootA->world.translate;

    float scaleFactor = get_visual_scale(tiny) / get_visual_scale(giant);

    NiPoint3 targetB = NiPoint3();
    std::vector<std::string_view> bone_names = {
      "NPC L Finger02 [LF02]",
      "NPC R Finger02 [RF02]",
      "L Breast02",
      "R Breast02"
    };
    std::uint32_t bone_count = bone_names.size();
    for (auto bone_name: bone_names) {
			auto bone = find_node(giant, bone_name);
			if (!bone) {
				return false;
			}
			targetB += (bone->world * NiPoint3()) * (1.0/bone_count);
		}

    // scaleFactor = std::clamp(scaleFactor, 0.0f, 1.0f);
    auto targetPoint = targetA*(scaleFactor) + targetB*(1.0 - scaleFactor);
    if (Runtime::GetBool("EnableDebugOverlay")) {
      		DebugAPI::DrawSphere(glm::vec3(targetA.x, targetA.y, targetA.z), 2.0, 40, {1.0, 0.0, 0.0, 1.0});
      		DebugAPI::DrawSphere(glm::vec3(targetB.x, targetB.y, targetB.z), 2.0, 40, {0.0, 1.0, 0.0, 1.0});
			DebugAPI::DrawSphere(glm::vec3(targetPoint.x, targetPoint.y, targetPoint.z), 2.0, 40, {0.0, 0.0, 1.0, 1.0});
		}

		return AttachTo(anyGiant, anyTiny, targetPoint);
	}

	template<typename T, typename U>
	bool AttachToCleavage(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}
		Actor* tiny = GetActorPtr(anyTiny);
		if (!tiny) {
			return false;
		}

		std::vector<std::string_view> bone_names = {
			"L Breast02",
			"R Breast02"
		};

		NiPoint3 clevagePos = NiPoint3();
		
		float LPosX = 0.0f;
		float LPosY = 0.0f;
		float LPosZ = 0.0f;

		float RPosX = 0.0f;
		float RPosY = 0.0f;
		float RPosZ = 0.0f;

		auto NPC = find_node(tiny, "NPC Root [Root]");
		if (!NPC) {
			return false;
		}
		auto BreastL = find_node(giant, "L Breast02");
		auto BreastR = find_node(giant, "R Breast02");
		if (!BreastL) {
			return false;
		} if (!BreastR) {
			return false;
		}

		NiMatrix3 LeftBreastRotation = BreastL->world.rotate;
		NiMatrix3 RightBreastRotation = BreastR->world.rotate;

		LeftBreastRotation.ToEulerAnglesXYZ(LPosX, LPosY, LPosZ);
		RightBreastRotation.ToEulerAnglesXYZ(RPosX, RPosY, RPosZ);

		NiMatrix3 NPCROT = NPC->world.rotate;


		auto NewRot = NiPoint3(((LPosX + RPosX) * 70) / 2, 0, 0);
		NPCROT.SetEulerAnglesXYZ(NewRot);

		log::info("Angle of L breast: x: {}, y: {}, z: {}", LPosX, LPosY, LPosZ);
		log::info("Angle of R breast: x: {}, y: {}, z: {}", RPosX, RPosY, RPosZ);
		

		std::uint32_t bone_count = bone_names.size();
		for (auto bone_name: bone_names) {
			auto bone = find_node(giant, bone_name);
			if (!bone) {
				return false;
			}
			if (Runtime::GetBool("EnableDebugOverlay")) {
				DebugAPI::DrawSphere(glm::vec3(bone->world.translate.x, bone->world.translate.y, bone->world.translate.z), 2.0, 10, {1.0, 1.0, 1.0, 1.0});
			}
			clevagePos += (bone->world * NiPoint3()) * (1.0/bone_count);
		}

		//clevagePos.x *= bonus;
		//clevagePos.z *= bonus;

		//tiny->data.angle.x = giant->data.angle.x;//((RPosX + LPosX) * 70) / 2;//
		//tiny->data.angle.x = ((RPosX + LPosX) * 70) / 2;
		//log::info("Tiny Angle X: {}", tiny->data.angle.x);
		//tiny->data.angle.y = giant->data.angle.y;
		tiny->data.angle.z = giant->data.angle.z;
		
		if (Runtime::GetBool("EnableDebugOverlay")) {
			DebugAPI::DrawSphere(glm::vec3(clevagePos.x, clevagePos.y, clevagePos.z), 2.0, 10, {1.0, 0.0, 0.0, 1.0});
		}

		return AttachTo(anyGiant, anyTiny, clevagePos);
	}
}
