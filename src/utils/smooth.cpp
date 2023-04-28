#include "utils/smooth.hpp"
#include "spring.hpp"

using namespace RE;
using namespace SKSE;

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
	void critically_damped(
		float& x,
		float& v,
		float x_goal,
		float halflife,
		float dt)
	{
		float y = halflife_to_damping(halflife) / 2.0f;
		float j0 = x - x_goal;
		float j1 = v + j0*y;
		float eydt = fast_negexp(y*dt);

		x = eydt*(j0 + j1*dt) + x_goal;
		v = eydt*(v - j1*y*dt);
	}


	// https://www.desmos.com/calculator/pqgliwxzi2
	float soft_power(const float x, const float k,const float n,const float s,const float o,const float a) {
		return pow(1.0+pow(k*(x),n*s),1.0/s)/pow(1.0+pow(k*o,n*s), 1.0/s) + a;
	}
	// https://www.desmos.com/calculator/vyofjrqmrn
	float soft_core(const float x, const float k, const float n, const float s, const float o, const float a) {
		return 1.0/soft_power(x, k, n, s, o, 0.0) + a;
	}
	float soft_core(const float x, const SoftPotential& soft_potential) {
		return soft_core(x, soft_potential.k, soft_potential.n, soft_potential.s, soft_potential.o, soft_potential.a);
	}
	float soft_power(const float x, const SoftPotential& soft_potential) {
		return soft_power(x, soft_potential.k, soft_potential.n, soft_potential.s, soft_potential.o, soft_potential.a);
	}

	// https://en.wikipedia.org/wiki/Smoothstep
	float clamp(float lowerlimit, float upperlimit, float x) {
		if (x < lowerlimit) {
			x = lowerlimit;
		}
		if (x > upperlimit) {
			x = upperlimit;
		}
		return x;
	}
	float smootherstep(float edge0, float edge1, float x) {
		// Scale, and clamp x to 0..1 range
		x = clamp(0.0, 1.0, (x - edge0) / (edge1 - edge0));
		// Evaluate polynomial
		return x * x * x * (x * (x * 6 - 15) + 10);
	}
	float smoothstep (float edge0, float edge1, float x) {
		// Scale/bias into [0..1] range
		x = clamp(0.0, 1.0, (x - edge0) / (edge1 - edge0));

		return x * x * (3 - 2 * x);
	}
}
