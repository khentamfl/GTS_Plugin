#pragma once
#include "node.hpp"
#include "UI/DebugAPI.hpp"

using namespace RE;

namespace Gts {

	Actor* GetActorPtr(Actor* actor);

	Actor* GetActorPtr(Actor& actor);

	Actor* GetActorPtr(ActorHandle& actor);

	Actor* GetActorPtr(const ActorHandle& actor);

	Actor* GetActorPtr(FormID formId);


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
	bool AttachToCleavage(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}

		auto breastLNode = find_node(giant, "L Breast02");
		if (!breastLNode) {
			return false;
		}
		auto breastL = breastLNode->world.translate;
    DebugAPI::DrawSphere(glm::vec3(breastL.x, breastL.y, breastL.z), 20.0, 10, {1.0, 1.0, 1.0 , 1.0});

		auto breastRNode = find_node(giant, "R Breast02");
		if (!breastRNode) {
			return false;
		}
		auto breastR = breastRNode->world.translate;
    DebugAPI::DrawSphere(glm::vec3(breastR.x, breastR.y, breastR.z), 20.0, 10, {1.0, 1.0, 1.0 , 1.0});

		auto spine2Node = find_node(giant, "NPC Spine2 [Spn2]");
		if (!spine2Node) {
			return false;
		}
		auto spine2 = spine2Node->world.translate;


		float forwardAmount = 1.0;

		auto breastForward = ((breastL - spine2) + (breastR - spine2))  * forwardAmount / 2 + spine2;
    DebugAPI::DrawSphere(glm::vec3(breastForward.x, breastForward.y, breastForward.z), 20.0, 10, {1.0, 0.0, 0.0 , 1.0});

		return AttachTo(anyGiant, anyTiny, breastForward);
	}
}
