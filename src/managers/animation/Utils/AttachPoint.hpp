#pragma once
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace RE;
namespace {
	const std::string_view leftFootLookup = "NPC L Foot [Lft ]";
	const std::string_view rightFootLookup = "NPC R Foot [Rft ]";
	const std::string_view leftCalfLookup = "NPC L Calf [LClf]";
	const std::string_view rightCalfLookup = "NPC R Calf [RClf]";
	const std::string_view leftToeLookup = "AnimObjectA";
	const std::string_view rightToeLookup = "AnimObjectA";
	const std::string_view bodyLookup = "NPC Spine1 [Spn1]";
}

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

		ForceRagdoll(tiny, false);

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
	bool AttachToUnderFoot_Left(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant); 
		if (!giant) {
			return false;
		}

		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(giant);

		auto leftFoot = find_node(giant, leftFootLookup);
		auto leftCalf = find_node(giant, leftCalfLookup);
		auto leftToe = find_node(giant, leftToeLookup);
		if (!leftFoot) {
			return false;
		} if (!leftCalf) {
			return false;
		} if (!leftToe) {
			return false;
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

		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0, hh/70, -0.75 + (-hh * 0.65)), // Offset it forward
		};
		std::tuple<NiAVObject*, NiMatrix3> left(leftFoot, leftRotMat);

		for (const auto& [foot, rotMat]: {left}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
				NiPoint3 coords = foot->world*(rotMat*point);
				log::info("FootLeft Coords: {}", Vector2Str(coords));
				coords.z -= hh * 0.65;
				return AttachTo(anyGiant, anyTiny, coords);
			}
		}
		return false;
		
	}

	template<typename T, typename U>
	bool AttachToUnderFoot_Right(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant); 
		if (!giant) {
			return false;
		}

		NiPoint3 hhOffsetbase = HighHeelManager::GetBaseHHOffset(giant);

		auto rightFoot = find_node(giant, rightFootLookup);
		auto rightCalf = find_node(giant, rightCalfLookup);
		auto rightToe = find_node(giant, rightToeLookup);


		if (!rightFoot) {
			return false;
		}
		if (!rightCalf) {
			return false;
		}
		if (!rightToe) {
			return false;
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

		float hh = hhOffsetbase[2];
		// Make a list of points to check
		std::vector<NiPoint3> points = {
			NiPoint3(0, hh/70, -0.75 + (-hh * 0.65)), // Offset it forward
		};
		std::tuple<NiAVObject*, NiMatrix3> right(rightFoot, rightRotMat);

		for (const auto& [foot, rotMat]: {right}) {
			std::vector<NiPoint3> footPoints = {};
			for (NiPoint3 point: points) {
				footPoints.push_back(foot->world*(rotMat*point));
				NiPoint3 coords = foot->world*(rotMat*point);
				log::info("FootRight Coords: {}", Vector2Str(coords));
				return AttachTo(anyGiant, anyTiny, coords);
			}
		}

		return false;
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
		//tiny->data.angle.y = giant->data.angle.y;
		tiny->data.angle.z = giant->data.angle.z;
		
		if (Runtime::GetBool("EnableDebugOverlay")) {
			DebugAPI::DrawSphere(glm::vec3(clevagePos.x, clevagePos.y, clevagePos.z), 2.0, 10, {1.0, 0.0, 0.0, 1.0});
		}

		return AttachTo(anyGiant, anyTiny, clevagePos);
	}
}
