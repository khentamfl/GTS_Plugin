#pragma once
// Module that handles footsteps

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	struct RbData {
		hkpRigidBody* rb;
		RbData(hkpRigidBody* shape);
		RbData(RbData&& old) : rb(std::move(old.rb)) {
		};
		~RbData();

		void ApplyPose(const hkVector4& origin, const float& new_scale);
	};
}
