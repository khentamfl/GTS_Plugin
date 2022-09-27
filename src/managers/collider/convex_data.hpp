#pragma once
#include "hooks/RE.hpp"
#include "managers/collider/common.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct RotatedVerts {
		hkVector4 x;
		hkVector4 y;
		hkVector4 z;
	};

	class ConvexData : public ShapeData {
		public:
			hkpConvexVerticesShape* convex;
			float radius;
			hkVector4 aabbHalfExtents;
			hkVector4 aabbCenter;
			std::uint32_t numVertices;
			std::vector<RotatedVerts> rotated_verts;

			ConvexData(hkpConvexVerticesShape* orig_capsule);
			ConvexData(ConvexData&& old) : convex(std::move(old.convex)), radius(std::move(old.radius)), aabbHalfExtents(std::move(old.aabbHalfExtents)), aabbCenter(std::move(old.aabbCenter)), rotated_verts(std::move(old.rotated_verts)) {
			};
			~ConvexData();

			void ApplyScale(const float& scale, const hkVector4& vecScale)  override;
			void SetOriginMinZ() override;
	};
}
