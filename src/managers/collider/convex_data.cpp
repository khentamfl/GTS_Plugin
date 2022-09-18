#include "managers/collider/convex_data.hpp"
#include "util.hpp"
#include "hooks/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	ConvexData::ConvexData(hkpConvexVerticesShape* shape) {
		// this->convex->AddReference();
		this->convex = shape;

		this->radius = shape->radius;
		this->aabbHalfExtents = shape->aabbHalfExtents;
		this->aabbCenter = shape->aabbCenter;

		std::size_t numVertices = shape->numVertices;
		std::size_t numRotatedVerticies = numVertices / 3;
		if (numVertices % 3 != 0) {
			numRotatedVerticies += 1;
		}

		this->rotated_verts = {};
		for (std::size_t i = 0; i< numRotatedVerticies; i++) {
			this->rotated_verts.push_back(shape->rotatedVertices[i].vertices[0]);
			this->rotated_verts.push_back(shape->rotatedVertices[i].vertices[1]);
			this->rotated_verts.push_back(shape->rotatedVertices[i].vertices[2]);
		}
	}

	ConvexData::~ConvexData () {
		// this->convex->RemoveReference();
	}

	void ConvexData::ApplyScale(const float& scale, const hkVector4& vecScale) {
		log::info("- Scaling ConvexVerts: {}", reinterpret_cast<std::uintptr_t>(this->convex));
		log::info("  - Scale: {}", scale);
		auto shape = this->convex;
		// shape->radius = this->radius * scale;
		// log::info("  - Radius: {} -> {}", this->radius, shape->radius);
		// shape->aabbHalfExtents = this->aabbHalfExtents * vecScale;
		// log::info("  - aabbHalfExtents: {} -> {}", Vector2Str(this->aabbHalfExtents), Vector2Str(shape->aabbHalfExtents));
		// shape->aabbCenter = this->aabbCenter * vecScale;
		// log::info("  - aabbCenter: {} -> {}", Vector2Str(this->aabbCenter), Vector2Str(shape->aabbCenter));

		std::size_t numRotatedVerticies = this->rotated_verts.size();
		for (std::size_t i = 0; i < numRotatedVerticies; i++) {
			std::size_t j = i / 3;
			std::size_t k = i % 3;
			log::info("  - shape->rotatedVertices[{}].vertices[{}] = {}", j, k, Vector2Str(shape->rotatedVertices[j].vertices[k]));
			shape->rotatedVertices[j].vertices[k] = this->rotated_verts[i] * vecScale;
			log::info("  - rotatedVertices[{}]: {} -> {}",i, Vector2Str(this->rotated_verts[i]), Vector2Str(shape->rotatedVertices[j].vertices[k]));

		}
	}
}
