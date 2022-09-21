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
		shape->aabbHalfExtents = this->aabbHalfExtents * vecScale;
		shape->aabbCenter = this->aabbCenter * vecScale;

		std::size_t numVertices = shape->numVertices;
		std::size_t numRotatedVerticies = numVertices / 4;
		if (numVertices % 4 != 0) {
			numRotatedVerticies += 1;
		}
		for (std::size_t i = 0; i< numRotatedVerticies; i++) {
			shape->rotatedVertices[i].vertices[0] = this->rotated_verts[i].x * vecScale;
			shape->rotatedVertices[i].vertices[1] = this->rotated_verts[i].y * vecScale;
			shape->rotatedVertices[i].vertices[2] = this->rotated_verts[i].z * vecScale;
		}
	}
}
