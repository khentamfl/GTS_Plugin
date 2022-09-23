#pragma once
// Module that handles footsteps
#include "hooks/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct RotatedVerts {
		hkVector4 x;
		hkVector4 y;
		hkVector4 z;
	};

	struct ConvexData {
		hkpConvexVerticesShape* convex;
		float radius;
		hkVector4 aabbHalfExtents;
		hkVector4 aabbCenter;
		std::uint32_t numVertices;
		std::vector<RotatedVerts> rotated_verts;
		NiPoint3 origin = NiPoint3(0.0, 0.0, 0.0);

		ConvexData(hkpConvexVerticesShape* orig_capsule);
		ConvexData(ConvexData&& old) : convex(std::move(old.convex)), radius(std::move(old.radius)), aabbHalfExtents(std::move(old.aabbHalfExtents)), aabbCenter(std::move(old.aabbCenter)), rotated_verts(std::move(old.rotated_verts)) {
		};
		~ConvexData();

		void ApplyScale(const float& scale, const hkVector4& vecScale);
		void SetOrigin(const float& x, const float& y, const float& z);
		void SetOrigin(const NiPoint3& point);
		void SetOriginMinZ();
	};
}
