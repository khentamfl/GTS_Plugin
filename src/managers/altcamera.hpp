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

		private:
			inline static float deltaX = 0.0;
			inline static float deltaZ = 0.0;
	};
}
