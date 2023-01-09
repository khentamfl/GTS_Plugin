#pragma once
#include "managers/cameras/state.hpp"
#include "spring.hpp"

using namespace RE;

namespace Gts {
	class Foot : public CameraState {
		public:
			virtual void EnterState() override;

			virtual NiPoint3 GetOffset(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPos) override;

			virtual NiPoint3 GetPlayerLocalOffsetInstant() override;
		protected:
			virtual NiPoint3 GetFootPos();

			Spring3 smoothFootPos = Spring3(NiPoint3(0.0, 0.0, 0.0), 0.01);

			Spring smoothScale = Spring(1.0, 0.5);
	};
}
