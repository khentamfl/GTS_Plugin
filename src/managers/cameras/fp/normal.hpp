#pragma once
#include "managers/cameras/fpState.hpp"

using namespace RE;

namespace Gts {
	class FirstPerson : public FirstPersonCameraState {
		public:
			virtual NiPoint3 GetPlayerLocalOffsetProne(const NiPoint3& cameraPos) override;
	};
}
