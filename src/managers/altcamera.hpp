#pragma once
// Module that handles the Camera
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class CameraManager : public EventListener {
		public:
			[[nodiscard]] static CameraManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
			virtual void Start() override;

			void AdjustUpDown(float amt);
			void ResetUpDown();

			void AdjustLeftRight(float amt);
			void ResetLeftRight();

			void UpdateFirstPerson(bool ImProne);

			void ApplyCameraSettings(float size, float x, float y, float AltX, float AltY, float MinDistance, float MaxDistance, float usingAutoDistance, bool ImProne, bool IsWeaponDrawn);
			void ApplyFeetCameraSettings(float size, float x, float y, float AltX, float AltY, float MinDistance, float MaxDistance, float usingAutoDistance, bool ImProne, bool IsWeaponDrawn);
			void UpdateCamera(bool ImProne);

		private:
			inline static float deltaX = 0.0;
			inline static float deltaZ = 0.0;
			inline static float CameraX = 0.0;
			inline static float CameraZ = 0.0;
			inline static float CameraMin = 0.0;
			inline static float CameraMax = 0.0;

	};
}
