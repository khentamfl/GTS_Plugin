#pragma once
// Module that handles the Camera
#include "events.hpp"
#include "timer.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class CameraManager : public EventListener {
		public:
			[[nodiscard]] static CameraManager& GetSingleton() noexcept;

			void SetfOverShoulderPosX(float value);
			float GetfOverShoulderPosX();
			void SetfOverShoulderPosY(float value);
			float GetfOverShoulderPosY();
			void SetfOverShoulderPosZ(float value);
			float GetfOverShoulderPosZ();

			void SetfOverShoulderCombatPosX(float value);
			float GetfOverShoulderCombatPosX();
			void SetfOverShoulderCombatPosY(float value);
			float GetfOverShoulderCombatPosY();
			void SetfOverShoulderCombatPosZ(float value);
			float GetfOverShoulderCombatPosZ();

			void SetfVanityModeMaxDist(float value);
			float GetfVanityModeMaxDist();
			void SetfVanityModeMinDist(float value);
			float GetfVanityModeMinDist();

			void SetfMouseWheelZoomIncrement(float value);
			float GetfMouseWheelZoomIncrement();
			void SetfMouseWheelZoomSpeed(float value);
			float GetfMouseWheelZoomSpeed();

			virtual std::string DebugName() override;
			virtual void Update() override;

			void AdjustSide(bool Reset, bool Right, bool Left);
			void AdjustUpDown(bool Reset, bool Up, bool Down);

			//void OnScaleChanged(float new_scale, float old_scale);

			void ApplyCameraSettings(float size, float x, float y, float AltX, float AltY, float MinDistance, float MaxDistance, float usingAutoDistance, bool ImProne);
			void ApplyFeetCameraSettings(float size, float x, float y, float AltX, float AltY, float MinDistance, float MaxDistance, float usingAutoDistance, bool ImProne);
			void UpdateFirstPerson(bool ImProne, float scale);
		private:
			float last_scale = -1.0;
			inline static float UpDown = 1.0;  //< -- Inline static float is a MUST. Else it doesn't work, for some reason.
			inline static float Side = 1.0;
			Timer CameraTimer = Timer(0.10);
	};
}
