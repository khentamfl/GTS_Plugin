#include "managers/damage/bhktest.hpp"

namespace Gts
{
	void RigidBody::SetAngularImpulse(const hkVector4& a_impulse)
	{
		using func_t = decltype(&RigidBody::SetAngularImpulse);
		REL::Relocation<func_t> func{ RELOCATION_ID(76262, 78092) };
		return func(this, a_impulse);
	}

	void RigidBody::SetAngularVelocity(const hkVector4& a_newVel)
	{
		using func_t = decltype(&RigidBody::SetAngularVelocity);
		REL::Relocation<func_t> func{ RELOCATION_ID(76260, 78090) };
		return func(this, a_newVel);
	}

	void RigidBody::SetLinearImpulse(const hkVector4& a_impulse)
	{
		using func_t = decltype(&RigidBody::SetLinearImpulse);
		REL::Relocation<func_t> func{ RELOCATION_ID(76261, 78091) };
		return func(this, a_impulse);
	}

	void RigidBody::SetLinearVelocity(const hkVector4& a_newVel)
	{
		using func_t = decltype(&RigidBody::SetLinearVelocity);
		REL::Relocation<func_t> func{ RELOCATION_ID(76259, 78089) };
		return func(this, a_newVel);
	}
}