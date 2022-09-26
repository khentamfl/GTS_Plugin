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
		hkVector4 wless = hkVector4(scale, scale, scale, 0.0);

		this->list->aabbHalfExtents = this->aabbHalfExtents * wless * hkVector4(2.0);
		this->list->aabbCenter = this->aabbCenter * wless;
		log::info("List: aabbCenter: {}", Vector2Str(this->list->aabbCenter));
		log::info("List: aabbHalfExtents: {}", Vector2Str(this->list->aabbHalfExtents));
		this->list->flags = 1;
	}
}
