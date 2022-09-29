#include "managers/collider/common.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	void ShapeData::ApplyScale(const float& new_scale, const hkVector4& vecScale) {

	}
	void ShapeData::ApplyPose(const hkVector4& origin, const float& new_scale) {

	}

	void ShapeData::SetOrigin(const float& x, const float& y, const float& z) {
		this->origin = NiPoint3(x, y, z);
	}

	void ShapeData::SetOrigin(const hkVector4& point) {
		this->SetOrigin(point.quad.m128_f32[0], point.quad.m128_f32[1], point.quad.m128_f32[2]);
	}

	void ShapeData::SetOrigin(const NiPoint3& point) {
		this->origin = point;
	}
	void ShapeData::SetOriginMinZ() {
		// Nothing here, just an empty default so I can be lazy and skip it
	}
}
