#pragma once
// Module that handles the Camera
#include "events.hpp"
#include "spring.hpp"
#include "managers/cameras/tp/alt.hpp"
#include "managers/cameras/tp/normal.hpp"
#include "managers/cameras/tp/foot.hpp"
#include "managers/cameras/tp/footL.hpp"
#include "managers/cameras/tp/footR.hpp"

#include "managers/cameras/fp/normal.hpp"
#include "managers/cameras/fp/combat.hpp"
#include "managers/cameras/fp/loot.hpp"

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
			Alt altState;
			Foot footState;
			FootR footRState;
			FootL footLState;

			FirstPerson fpState;
			FirstPersonCombat fpCombatState;
			FirstPersonLoot fpLootState;

			NiPoint3 manualEdit;
			Spring smoothScale = Spring(0.33, 1000.0);
			Spring3 smoothOffset = Spring3(NiPoint3(0.33, 0.33, 0.33), 1000.0);
			Spring3 smoothPlayerOffset = Spring3(NiPoint3(0.33, 0.33, 0.33), 1000.0);
	};
}
