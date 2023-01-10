#pragma once
#include "managers/cameras/state.hpp"
#include "spring.hpp"

using namespace RE;

namespace Gts {
	class ThirdPersonCameraState : public CameraState {
		public:
			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPos) override;
			virtual NiPoint3 GetPlayerLocalOffsetProne(const NiPoint3& cameraPos) override;
			virtual std::vector<std::string> GetBoneTargets();
			virtual NiPoint3 ProneAdjustment(const NiPoint3& cameraPosLocal);

		private:
			Spring smoothScale = Spring(1.0, 0.5);
			Spring3 smoothedBonePos = Spring3(NiPoint3(0.0, 0.0, 0.0), 0.5);
	};
}
