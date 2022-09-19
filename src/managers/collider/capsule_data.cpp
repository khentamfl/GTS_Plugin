#include "managers/collider/capsule_data.hpp"
#include "util.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	CapsuleData::CapsuleData(hkpCapsuleShape* orig_capsule) {
		// this->capsule->AddReference();
		this->capsule = orig_capsule;

		this->start = orig_capsule->vertexA;
		this->end = orig_capsule->vertexB;
		this->radius = orig_capsule->radius;
	}

	CapsuleData::~CapsuleData () {
		// this->capsule->RemoveReference();
	}

	void CapsuleData::ApplyScale(const float& scale, const hkVector4& vecScale) {
		// log::info("Scaling capsule: {}", reinterpret_cast<std::uintptr_t>(this->capsule));
		// log::info("  - Scale by: {}", scale);
		this->capsule->vertexA = this->start * vecScale;
		// log::info("  - VertexA: {} -> {}", Vector2Str(this->start), Vector2Str(this->capsule->vertexA));
		this->capsule->vertexB = this->end * vecScale;
		// log::info("  - VertexB: {} -> {}", Vector2Str(this->end), Vector2Str(this->capsule->vertexB));
		this->capsule->radius = this->radius * scale;
		// log::info("  - Radius: {} -> {}", this->radius, this->capsule->radius);
	}
}
