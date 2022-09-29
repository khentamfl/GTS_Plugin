#include "managers/collider/convex_data.hpp"
#include "util.hpp"
#include "hooks/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	ConvexData::ConvexData(hkpConvexVerticesShape* shape) {
		this->convex = shape;
		this->convex->AddReference();

		this->radius = shape->radius;
		this->aabbHalfExtents = shape->aabbHalfExtents;
		this->aabbCenter = shape->aabbCenter;
		this->numVertices = shape->numVertices;

		std::size_t numVertices = shape->numVertices;
		std::size_t numRotatedVerticies = numVertices / 4;
		if (numVertices % 4 != 0) {
			numRotatedVerticies += 1;
		}

		this->rotated_verts = {};
		for (std::size_t i = 0; i< numRotatedVerticies; i++) {
			this->rotated_verts.push_back(
				RotatedVerts {
				.x = shape->rotatedVertices[i].vertices[0],
				.y = shape->rotatedVertices[i].vertices[1],
				.z = shape->rotatedVertices[i].vertices[2]}
				);
		}

		// this->SetOriginMinZ();
	}

	ConvexData::~ConvexData () {
		this->convex->RemoveReference();
	}

	void ConvexData::ApplyScale(const float& scale, const hkVector4& vecScale) {
		auto shape = this->convex;
		if (shape->numVertices != this->numVertices) {
			log::error("Vertext count mismatch");
		}
		// shape->radius = this->radius * scale;
		hkVector4 origin = hkVector4(this->origin.x, this->origin.y, this->origin.z, 0.0);
		// log::info("origin: {}", Vector2Str(origin));

		shape->aabbHalfExtents = this->aabbHalfExtents * vecScale * hkVector4(2.0);
		shape->aabbCenter = (this->aabbCenter - origin) * vecScale + origin;

		// log::info("ConvVerts: aabbCenter: {}", Vector2Str(shape->aabbCenter));
		// log::info("ConvVerts: aabbHalfExtents: {}", Vector2Str(shape->aabbHalfExtents));

		std::size_t numVertices = shape->numVertices;
		std::size_t numRotatedVerticies = numVertices / 4;
		if (numVertices % 4 != 0) {
			numRotatedVerticies += 1;
		}
		hkVector4 x_origin = hkVector4(this->origin.x);
		hkVector4 y_origin = hkVector4(this->origin.y);
		hkVector4 z_origin = hkVector4(this->origin.z);
		for (std::size_t i = 0; i< numRotatedVerticies; i++) {
			shape->rotatedVertices[i].vertices[0] = (this->rotated_verts[i].x - x_origin) * vecScale + x_origin;
			shape->rotatedVertices[i].vertices[1] = (this->rotated_verts[i].y - y_origin) * vecScale + y_origin;
			shape->rotatedVertices[i].vertices[2] = (this->rotated_verts[i].z - z_origin) * vecScale + z_origin;
		}
	}

	void ConvexData::SetOriginMinZ() {
		if (this->rotated_verts.size() == 0) {
			return;
		}
		float lowest_z = this->rotated_verts[0].z.quad.m128_f32[0];
		NiPoint3 lowest = NiPoint3(
			this->rotated_verts[0].x.quad.m128_f32[0],
			this->rotated_verts[0].y.quad.m128_f32[0],
			this->rotated_verts[0].z.quad.m128_f32[0]
			);
		for (auto& rotated_vert: this->rotated_verts) {
			for (std::size_t j = 0; j<4; j++) {
				if (rotated_vert.z.quad.m128_f32[j] < lowest_z) {
					lowest_z = rotated_vert.z.quad.m128_f32[j];
					NiPoint3 lowest = NiPoint3(
						rotated_vert.x.quad.m128_f32[j],
						rotated_vert.y.quad.m128_f32[j],
						rotated_vert.z.quad.m128_f32[j]
						);
				}
			}
		}
		log::info("Setting convverts, lowest point: {}", Vector2Str(lowest));
		this->SetOrigin(lowest);
	}
}
