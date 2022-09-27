#include "managers/collider/list_data.hpp"
#include "util.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	ListData::ListData(hkpListShape* list) {
		this->list = list;
		this->list->AddReference();

		this->aabbHalfExtents = list->aabbHalfExtents;
		this->aabbCenter = list->aabbCenter;
	}

	ListData::~ListData () {
		this->list->RemoveReference();
	}

	void ListData::ApplyScale(const float& scale, const hkVector4& vecScale) {
		hkVector4 origin = hkVector4(this->origin.x, this->origin.y, this->origin.z, 0.0);
		this->list->aabbHalfExtents = this->aabbHalfExtents * vecScale;
		this->list->aabbCenter = (this->aabbCenter - origin) * vecScale + origin;
	}

	void ListData::SetOriginMinZ() {
		hkVector4 lowest = this->aabbCenter - hkVector4(0.0, 0.0, this->aabbHalfExtents.quad.m128_f32[2], 0.0);
		this->SetOrigin(lowest);
	}
}
