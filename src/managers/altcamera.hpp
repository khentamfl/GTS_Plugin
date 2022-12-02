#pragma once
// Module that handles the Camera
#include "events.hpp"
#include "spring.hpp"
#include "managers/cameras/alt.hpp"
#include "managers/cameras/altProne.hpp"
#include "managers/cameras/normal.hpp"
#include "managers/cameras/prone.hpp"
#include "managers/cameras/foot.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class CameraManager : public EventListener {
		public:
			[[nodiscard]] static CameraManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Start() override;

			virtual void CameraUpdate() override;

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
			Foot footState;

			NiPoint3 manualEdit;
			Spring smoothScale = Spring(1.0, 0.2);
			Spring3 smoothOffset = Spring3(NiPoint3(0.0, 0.0, 0.0), 0.2);
	};
}
