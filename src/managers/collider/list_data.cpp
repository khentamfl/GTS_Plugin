#include "managers/collider/list_data.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	ListData::ListData(hkpListShape* list) {
		this->list->AddReference();
		this->list = list;

		this->aabbHalfExtents = list->aabbHalfExtents;
		this->aabbCenter = list->aabbCenter;
	}

	ListData::~ListData () {
		this->list->AddReference();
	}

	void ListData::ApplyScale(const float& scale, const hkVector4& vecScale) {
		this->list->aabbHalfExtents = this->aabbHalfExtents * vecScale;
		this->list->aabbCenter = this->aabbCenter * vecScale;
	}
}
