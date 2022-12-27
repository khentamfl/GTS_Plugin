#pragma once
#include "managers/cameras/state.hpp"
#include "spring.hpp"

using namespace RE;

namespace Gts {
	class Foot : public CameraState {
		public:
			virtual NiPoint3 GetOffset(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPos) override;

			inline std::string Vector2Str(const NiPoint3& vector) {
				return std::format("{:.2f},{:.2f},{:.2f}", vector.x, vector.y, vector.z);
			}

			inline std::string Vector2Str(const NiPoint3* vector) {
			if (vector) {
				return std::format("{:.2f},{:.2f},{:.2f}", vector->x, vector->y, vector->z);
			} else {
				return "";
			}
	}

		private:
			NiPoint3 GetFootPos();

			Spring3 smoothFootPos = Spring3(NiPoint3(0.0, 0.0, 0.0), 200.0);
	};
}
