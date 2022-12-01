#pragma once
#include "managers/cameras/state.hpp"

namespace Gts {
	class Normal : public CameraState {
		virtual NiPoint3 GetOffset(const NiPoint3& cameraPos) override {
			return NiPoint3(
				Runtime::GetFloat("cameraX"),
				0.0,
				Runtime::GetFloat("cameraY")
				);
		}

		virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPos) override {
			return NiPoint3(
				Runtime::GetFloat("combatCameraX"),
				0.0,
				Runtime::GetFloat("combatCameraY")
				);
		}
	};
}
