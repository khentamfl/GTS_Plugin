#pragma once
#include "managers/cameras/state.hpp"

using namespace RE;

namespace Gts {
	class ThirdPersonCameraState : public CameraState {
		public:
			virtual NiPoint3 GetPlayerLocalOffsetProne(const NiPoint3& cameraPosLocal) override;
			virtual NiPoint3 ProneAdjustment(const NiPoint3& cameraPosLocal);
	};
}
