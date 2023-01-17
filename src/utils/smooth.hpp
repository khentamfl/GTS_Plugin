#pragma once

using namespace RE;
using namespace SKSE;

namespace Gts {
	void critically_damped(
		float& x,
		float& v,
		float x_goal,
		float halflife,
		float dt);


	struct SoftPotential {
		float k;
		float n;
		float s;
		float o;
		float a;
	};
	// https://www.desmos.com/calculator/pqgliwxzi2
	float soft_power(const float x, const float k,const float n,const float s,const float o,const float a);
	// https://www.desmos.com/calculator/vyofjrqmrn
	float soft_core(const float x, const float k, const float n, const float s, const float o, const float a);
	float soft_core(const float x, const SoftPotential& soft_potential);
	float soft_power(const float x, const SoftPotential& soft_potential);

	// https://en.wikipedia.org/wiki/Smoothstep
	float clamp(float lowerlimit, float upperlimit, float x);
	float smootherstep(float edge0, float edge1, float x);
	float smoothstep (float edge0, float edge1, float x);
}
