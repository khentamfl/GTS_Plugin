#pragma once
#include <RE/B/bhkRigidBody.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class RigidBody : public bhkRigidBody {
		public:
			void SetAngularImpulse(const hkVector4& a_impulse);
		    void SetAngularVelocity(const hkVector4& a_newVel);
		    void SetLinearImpulse(const hkVector4& a_impulse);
		    void SetLinearVelocity(const hkVector4& a_newVel);
	};
}