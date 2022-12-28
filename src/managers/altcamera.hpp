#pragma once
// Module that handles the Camera
#include "events.hpp"
#include "spring.hpp"
#include "managers/cameras/alt.hpp"
#include "managers/cameras/altProne.hpp"
#include "managers/cameras/normal.hpp"
#include "managers/cameras/prone.hpp"
#include "managers/cameras/foot.hpp"
#include "managers/cameras/footL.hpp"
#include "managers/cameras/footR.hpp"

#include "managers/cameras/fp.hpp"
#include "managers/cameras/fpProne.hpp"
#include "managers/cameras/fpCombat.hpp"
#include "managers/cameras/fpCombatProne.hpp"
#include "managers/cameras/fpLoot.hpp"
#include "managers/cameras/fpLootProne.hpp"

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
			FootR footRState;
			FootL footLState;

			FirstPerson fpState;
			FirstPersonProne fpProneState;
			FirstPersonCombat fpCombatState;
			FirstPersonCombatProne fpCombatProneState;
			FirstPersonLoot fpLootState;
			FirstPersonLootProne fpLootProneState;

			NiPoint3 manualEdit;
			Spring smoothScale = Spring(1.0, 3.0);
			Spring3 smoothOffset = Spring3(NiPoint3(0.0, 0.0, 0.0), 3.0);
			Spring3 smoothPlayerOffset = Spring3(NiPoint3(0.0, 0.0, 0.0), 3.0);
	};
}
