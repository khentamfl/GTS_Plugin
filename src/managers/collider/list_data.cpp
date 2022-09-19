#include "managers/collider/list_data.hpp"
#include "util.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	ListData::ListData(hkpListShape* list) {
		// this->list->AddReference();
		this->list = list;

		this->aabbHalfExtents = list->aabbHalfExtents;
		this->aabbCenter = list->aabbCenter;
	}

	ListData::~ListData () {
		// this->list->AddReference();
	}

	void ListData::ApplyScale(const float& scale, const hkVector4& vecScale) {
		// log::info("- Scaling list: {}", reinterpret_cast<std::uintptr_t>(this->list));
		// log::info("  - Scale by: {}", scale);
		this->list->aabbHalfExtents = this->aabbHalfExtents * vecScale;
		// log::info("  - aabbHalfExtents: {} -> {}", Vector2Str(this->aabbHalfExtents), Vector2Str(this->list->aabbHalfExtents));
		this->list->aabbCenter = this->aabbCenter * vecScale;
		// log::info("  - aabbCenter: {} -> {}", Vector2Str(this->aabbCenter), Vector2Str(this->list->aabbCenter));
	}
}
