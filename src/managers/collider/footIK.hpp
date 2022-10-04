#pragma once
// Module that handles footsteps
#include "hooks/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class FootIkData {
		public:
			FootIkData();
			FootIkData(hkbFootIkDriver* ik);
			~FootIkData();
			void ChangeIk(hkbFootIkDriver* ik);

			void ApplyScale(const float& new_scale, const hkVector4& vecScale);
			void ApplyPose(const hkVector4& origin, const float& new_scale);

			hkbFootIkDriver* ik = nullptr;
		private:
			mutable std::mutex _lock;
	};
}
