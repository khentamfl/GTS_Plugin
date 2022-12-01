#pragma once
// Module that handles the Camera
#include "events.hpp"
#include "spring.hpp"
#include "managers/cameras/alt.hpp"
#include "managers/cameras/altProne.hpp"
#include "managers/cameras/normal.hpp"
#include "managers/cameras/prone.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class CameraManager : public EventListener {
		public:
			[[nodiscard]] static CameraManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Start() override;

			static void UpdateCamera();
			void ApplyCamera();

			CameraState* GetCameraState();

			void AdjustUpDown(float amt);
			void ResetUpDown();

			void AdjustLeftRight(float amt);
			void ResetLeftRight();

		private:
			Normal normalState;
			Prone proneState;
			Alt altState;
			AltProne altProneState;

			Spring deltaX = Spring(0.0, 0.0002);
			Spring deltaZ = Spring(0.0, 0.0002);
	};
}
