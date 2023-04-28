#pragma once
#include "magic/magic.hpp"
// Module that tracks the target


using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class TrackSize : public Magic {
		public:
			using Magic::Magic;

			virtual void OnUpdate() override;

			virtual std::string GetName() override;
	};
}
