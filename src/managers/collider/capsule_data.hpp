#pragma once
#include "managers/collider/common.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	hkpCapsuleShape* MakeCapsule();

	class CapsuleData : public ShapeData {
		public:
			hkpCapsuleShape* capsule;
			hkVector4 start;
			hkVector4 end;
			float radius;
			CapsuleData(hkpCapsuleShape* orig_capsule);
			CapsuleData(CapsuleData&& old) : capsule(std::move(old.capsule)), start(std::move(old.start)), end(std::move(old.end)), radius(std::move(old.radius)) {
			};
			~CapsuleData();

			void ApplyScale(const float& new_scale, const hkVector4& vecScale) override;
			void SetOriginMinZ() override;
	};
}
