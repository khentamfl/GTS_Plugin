#pragma once

using namespace RE;

namespace Gts {
	class CameraState {
		public:
			virtual float GetScale();
			virtual NiPoint3 GetOffset(const NiPoint3& cameraPosLocal);
			virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPosLocal);
			virtual NiPoint3 GetPlayerLocalOffset(const NiPoint3& cameraPosLocal);
			virtual bool PermitManualEdit();
	};
}
