#pragma once
// Module that handles footsteps

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct ListData {
		hkpListShape* list;
		hkVector4 aabbHalfExtents;
		hkVector4 aabbCenter;
		ListData(hkpListShape* shape);
		ListData(ListData&& old) : list(std::move(old.list)), aabbHalfExtents(std::move(old.aabbHalfExtents)), aabbCenter(std::move(old.aabbCenter)) {
		};
		~ListData();

		void ApplyScale(const float& new_scale, const hkVector4& vecScale);
	};
}
