#pragma once
#include "managers/cameras/state.hpp"

namespace Gts {
	class Alt : public CameraState {
		virtual NiPoint3 GetOffset(const NiPoint3& cameraPos) override;

		virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPos) override;
	};
}
