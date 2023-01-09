#pragma once
#include "managers/cameras/state.hpp"

using namespace RE;

namespace Gts {
	class FirstPersonCameraState : public CameraState {
		public:
			virtual NiPoint3 GetPlayerLocalOffsetInstant(const NiPoint3& cameraPos) override;
			virtual bool PermitManualEdit() override;

			virtual NiPoint3 ProneAdjustment(const NiPoint3& cameraPosLocal);
	};
}
