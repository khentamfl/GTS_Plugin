#include "managers/collider/capsule_data.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	CapsuleData::CapsuleData(hkpCapsuleShape* orig_capsule) {
		this->capsule = orig_capsule;

		this->start = orig_capsule->vertexA;
		this->capsule->AddReference();
		this->end = orig_capsule->vertexB;
		this->radius = orig_capsule->radius;
	}

	CapsuleData::~CapsuleData () {
		this->capsule->RemoveReference();
	}

	void CapsuleData::ApplyScale(const float& scale, const hkVector4& vecScale) {
		this->capsule->vertexA = this->start * vecScale;
		this->capsule->vertexB = this->end * vecScale;
		this->capsule->radius = this->radius * scale;
	}
}
