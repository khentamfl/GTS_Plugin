#pragma once
#include "managers/collider/common.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ListData : public ShapeData {
		public:
			hkpListShape* list;
			hkVector4 aabbHalfExtents;
			hkVector4 aabbCenter;
			ListData(hkpListShape* shape);
			ListData(ListData&& old) : list(std::move(old.list)), aabbHalfExtents(std::move(old.aabbHalfExtents)), aabbCenter(std::move(old.aabbCenter)) {
			};
			~ListData();

			void ApplyScale(const float& new_scale, const hkVector4& vecScale) override;
			void SetOriginMinZ() override;
	};
}
