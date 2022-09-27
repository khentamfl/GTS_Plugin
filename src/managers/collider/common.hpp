#pragma once
// Module that handles footsteps

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ShapeData {
		public:
			virtual void ApplyScale(const float& new_scale, const hkVector4& vecScale);
			virtual void ApplyPose(const hkVector4& origin, const float& new_scale);

			virtual void SetOrigin(const float& x, const float& y, const float& z);
			virtual void SetOrigin(const NiPoint3& point);
			virtual void SetOrigin(const hkVector4& point);
			virtual void SetOriginMinZ();
		protected:
			NiPoint3 origin = NiPoint3(0.0, 0.0, 0.0);
	};
}
