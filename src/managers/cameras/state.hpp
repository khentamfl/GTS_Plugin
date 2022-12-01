#pragma once

namespace Gts {
	class CameraState {
		virtual float GetScale();
		virtual NiPoint3 GetOffset(const NiPoint3& cameraPosLocal);
		virtual NiPoint3 GetCombatOffset(const NiPoint3& cameraPosLocal);
	};
}
