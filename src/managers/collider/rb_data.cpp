#include "managers/collider/rb_data.hpp"
#include "util.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace {
	void scale_relative_byref(hkVector4& input, const hkVector4& origin, const float& scale) {
		input = (input - origin)*scale + origin;
	}
}

namespace Gts {
	RbData::RbData(hkpRigidBody* rb) {
		this->rb = rb;
		this->rb->AddReference();
	}

	RbData::~RbData () {
		this->rb->RemoveReference();
	}

	void RbData::ApplyPose(const hkVector4& origin, const float& new_scale) {
		// auto rigidBody = this->rb;
		// // Translation
		// scale_relative_byref(rigidBody->motion.motionState.transform.translation, origin, new_scale);
		//
		// // COM 0
		// scale_relative_byref(rigidBody->motion.motionState.sweptTransform.centerOfMass0, origin, new_scale);
		//
		// // COM 1
		// scale_relative_byref(rigidBody->motion.motionState.sweptTransform.centerOfMass1, origin, new_scale);
	}
}
