#pragma once
#include "managers/cameras/state.hpp"

using namespace RE;

namespace Gts {
	class AltProne : public CameraState {
		public:
			virtual NiPoint3 GetOffset(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPos) override;
	};
}
