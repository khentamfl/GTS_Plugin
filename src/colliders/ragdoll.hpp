#pragma once
// Takes a ragdoll and extracts its collision objects
#include "colliders/common.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class RagdollData: public ColliderData {
		public:
			RagdollData(hkaRagdollInstance* ragdoll);
	};
}
