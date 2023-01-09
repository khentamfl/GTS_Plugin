#pragma once

using namespace RE;

namespace Gts {
	class CameraState {
		public:
			virtual void EnterState();
			virtual void ExitState();

			virtual float GetScale();
			virtual NiPoint3 GetOffset(const NiPoint3& cameraPosLocal);
			virtual NiPoint3 GetOffset(const NiPoint3& cameraPosLocal, bool isProne);
			virtual NiPoint3 GetOffsetProne(const NiPoint3& cameraPosLocal);

			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPosLocal);
			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPosLocal, bool isProne);
			virtual NiPoint3 GetCombatOffsetProne(const NiPoint3& cameraPosLocal);

			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPosLocal);
			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPosLocal, bool isProne);
			virtual NiPoint3 GetPlayerLocalOffsetProne(const NiPoint3& cameraPosLocal);

			virtual NiPoint3 GetPlayerLocalOffsetInstant(const NiPoint3& cameraPos);

			virtual bool PermitManualEdit();
	};
}
