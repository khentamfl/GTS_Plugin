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
			virtual void Start() override;

			static void UpdateCamera();
			void ApplyCamera();

			void AdjustUpDown(float amt);
			void ResetUpDown();

			void AdjustLeftRight(float amt);
			void ResetLeftRight();

		private:

			float deltaX = 0.0;
			float deltaZ = 0.0;
	};
}
