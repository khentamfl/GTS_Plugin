#pragma once
#include "data/runtime.hpp"
#include "UI/DebugAPI.hpp"
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
	bool HugAttach(T& anyGiant, U& anyTiny, float additionalScale) {
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
		auto giantPos = giant->GetPosition();
		NiPoint3 target = bone->world.translate;
    NiPoint3 giantLocal = (target - giantPos);

    // TWEAK THESE
    const float M_FORWARD = 1.25;
    const float M_UP = 2.0;

    // y = m*x + c
    // @1x scale no change
    // 1 = m*1+c
    //  Therefore c=?
    // c = 1-m

    log::info("Scale ratio: {}", additionalScale); 
    float scale_forward = M_FORWARD * additionalScale + (1.0 - M_FORWARD);
    float scale_up = M_UP * additionalScale + (1.0 - M_UP);
    log::info("  - scale_forward: {}", scale_forward);
    log::info("  - scale_up: {}", scale_up);
    // scale_forward = std::clamp(scale_forward, 0.5f, 2.0f);
    // scale_up = std::clamp(scale_up, 0.5f, 2.0f);
    log::info("  - local delta: {}", Vector2Str(giantLocal));
    log::info("  - local forwards: {}", NiPoint3(giantLocal.x, giantLocal.y, 0.0).Length());
    log::info("  - local up: {}", giantLocal.z);
    giantLocal.x *= scale_forward;
    giantLocal.y *= scale_forward;
    giantLocal.z *= scale_up;
    log::info("  - local delta scaled: {}", Vector2Str(giantLocal));
    log::info("  - local forwards scaled: {}", NiPoint3(giantLocal.x, giantLocal.y, 0.0).Length());
    log::info("  - local up scaled: {}", giantLocal.z);
		target = giantLocal + giantPos;
		return AttachTo(anyGiant, anyTiny, target);
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

		tiny->data.angle.x = giant->data.angle.x;
		tiny->data.angle.y = giant->data.angle.y;
		tiny->data.angle.z = giant->data.angle.z;
		if (Runtime::GetBool("EnableDebugOverlay")) {
			DebugAPI::DrawSphere(glm::vec3(clevagePos.x, clevagePos.y, clevagePos.z), 2.0, 10, {1.0, 0.0, 0.0, 1.0});
		}

		return AttachTo(anyGiant, anyTiny, clevagePos);
	}
}
