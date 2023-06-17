#pragma once
#include "node.hpp"
#include <math>

using namespace RE;

namespace Gts {

	template<typename U>
	bool FaceTowardsPoint(U& anyTiny, NiPoint3 point) {
		Actor* tiny =  GetActorPtr(anyTiny);

		if (!tiny) {
			return false;
		}

		auto tinyPos = tiny->GetPosition();

    NiPoint forwards = NiPoint3(0, 1, 0);
    NiPoint3 direction = point - tinyPos;
    direction.z = 0;
    direction = direction / direction.Length();
    float newAngle = acos(forwards.Dot(direction));

		tiny->data.angle.z = newAngle;

		return true;
	}
	template<typename T, typename U>
	bool FaceTowards(T& anyGiant, U& anyTiny, std::string_view bone_name) {
		Actor* giant = GetActorPtr(anyGiant);
		if (!giant) {
			return false;
		}
		auto bone = find_node(giant, bone_name);
		if (!bone) {
			return false;
		}
		return FaceTowardsPoint(anyTiny, bone->world.translate);
	}

	template<typename T, typename U>
	bool FaceTowards(T& anyGiant, U& anyTiny) {
		return FaceTowardsPoint(anyTiny, anyGiant->GetPosition());
	}

}
