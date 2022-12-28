#pragma once
#include "managers/cameras/fpState.hpp"

using namespace RE;

namespace Gts {
	class FirstPersonCombat : public FirstPersonCameraState {
		public:
			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPos) override;
	};
}
