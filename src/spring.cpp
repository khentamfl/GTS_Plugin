#include "spring.hpp"

namespace {
	// Spring code from https://theorangeduck.com/page/spring-roll-call
	float halflife_to_damping(float halflife, float eps = 1e-5f)
	{
		return (4.0f * 0.69314718056f) / (halflife + eps);
	}

	float damping_to_halflife(float damping, float eps = 1e-5f)
	{
		return (4.0f * 0.69314718056f) / (damping + eps);
	}
	float fast_negexp(float x)
	{
		return 1.0f / (1.0f + x + 0.48f*x*x + 0.235f*x*x*x);
	}
}

namespace Gts {

	void Spring::Update(float dt) {
		float y = halflife_to_damping(this->halflife) / 2.0f;
		float j0 = this->value - this->target;
		float j1 = this->velocity + j0*y;
		float eydt = fast_negexp(y*dt);

		this->value = eydt*(j0 + j1*dt) + this->target;
		this->velocity = eydt*(this->velocity - j1*y*dt);
	}

	Spring::Spring(float initial, float halflife) : value(initial), target(initial), halflife(halflife) {
	}
}
