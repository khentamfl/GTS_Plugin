#pragma once
#include "managers/cameras/state.hpp"

using namespace RE;

namespace Gts {
	class FirstPersonCameraState : public CameraState {
		public:
			virtual bool PermitManualEdit() override;
			virtual bool PermitTransition() override;

			virtual float ProneAdjustment();
	};
}
