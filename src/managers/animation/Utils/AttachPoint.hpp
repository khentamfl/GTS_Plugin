#pragma once
#include "node.hpp"
#include "UI/DebugAPI.hpp"

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

	bool HugAttach(T& anyGiant, U& anyTiny) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}
		Actor* tiny = GetActorPtr(anyTiny);
		if (!tiny) {
			return false;
		}
		auto bone = find_node(giant, "AnimObjectA");
		if (!bone) {
			return false;
		}
		NiPoint3 Coordinates = bone->world.translate;
		float offset = (70 + get_visual_scale(giant)/get_visual_scale(tiny)) - 70.0;
		Coordinates.y += offset;
		Coordinates.y += offset;
		Coordinates.z += offset;
		return AttachTo(anyGiant, anyTiny, offset);
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
		DebugAPI::DrawSphere(glm::vec3(bone->world.translate.x, bone->world.translate.y, bone->world.translate.z), 2.0, 10, {1.0, 1.0, 1.0 , 1.0});
		clevagePos += (bone->world * NiPoint3()) * (1.0/bone_count);
		}

		tiny->data.angle.x = giant->data.angle.x;
		tiny->data.angle.y = giant->data.angle.y;
		tiny->data.angle.z = giant->data.angle.z;

		DebugAPI::DrawSphere(glm::vec3(clevagePos.x, clevagePos.y, clevagePos.z), 2.0, 10, {1.0, 0.0, 0.0 , 1.0});

		return AttachTo(anyGiant, anyTiny, clevagePos);
	}
}
