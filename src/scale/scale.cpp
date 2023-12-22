#include "scale/scale.hpp"
#include "managers/GtsManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/height.hpp"

using namespace Gts;

namespace {
	const float EPS = 1e-4;

	float GetShrinkPenalty(float size) {
		// https://www.desmos.com/calculator/pqgliwxzi2
		SoftPotential cut {
			.k = 1.08,
			.n = 0.90,
			.s = 3.00,
			.a = 0.0,
		};
		float power = soft_power(size, cut);
		if (SizeManager::GetSingleton().BalancedMode() >= 2.0) {
			return std::clamp(power, 1.0f, 99999.0f); // So it never reports values below 1.0. Just to make sure.
		} else {
			return 1.0;
		}
	}
}

namespace Gts {

	void set_target_scale(Actor& actor, float scale) {
		auto actor_data = Persistent::GetSingleton().GetData(&actor);
		if (actor_data) {
			if (scale < (actor_data->max_scale + EPS)) {
				// If new value is below max: allow it
				actor_data->target_scale = scale;
			} else if (actor_data->target_scale < (actor_data->max_scale - EPS)) {
				// If we are below max currently and we are trying to scale over max: make it max
				actor_data->target_scale = actor_data->max_scale;
			} else {
				// If we are over max: forbid it
			}
		}
	}
	void set_target_scale(Actor* actor, float scale) {
		if (actor) {
			Actor& a = *actor;
			set_target_scale(a, scale);
		}
	}

	float get_target_scale(Actor& actor) {
		auto actor_data = Persistent::GetSingleton().GetData(&actor);
		if (actor_data) {
			return actor_data->target_scale;
		} else {
			return -1.0;
		}
	}
	float get_target_scale(Actor* actor) {
		if (actor) {
			Actor& a = *actor;
			return get_target_scale(a);
		} else {
			return -1.0;
		}
	}

	void mod_target_scale(Actor& actor, float amt) {
		auto profiler = Profilers::Profile("Scale: ModTargetScale");
		auto actor_data = Persistent::GetSingleton().GetData(&actor);
		// TODO: Fix this
		//
		// Semit, these functions should be RAW and make actual unscaled adjustments
		// some of the math will do odd things if not.
		//
		// In this case it means that a setscale and a modscale can produce
		// different scales
		//
		// If you want to adjust these I receommend we instead create a more
		// general function called Grow() and Shrink() (as is done in the magic)
		// that handles these modifier effects
		//
		// This way we have to sets of function Grow which we accept can be modify and may not
		// actually mod by the request amount and mod_target_scale which
		// represents real final absolute adjustments
		//
		// By altering this we now have NO way to make an absolute adjustment
		//
		// At this point in the CODE amt should NOT be altered, ALTER IT BEFORE THIS FUNCTION
		//
		if (amt > 0 && (actor.formID == 0x14 || IsTeammate(&actor))) {
			float scale = actor_data->visual_scale; // Enabled if BalanceMode is True. Decreases Grow Efficiency.
			if (scale >= 1.0) {
				amt /= GetShrinkPenalty(scale);
			}
		}
		if (Runtime::HasPerkTeam(&actor, "OnTheEdge")) {
			float GetHP = clamp(0.5, 1.0, GetHealthPercentage(&actor) + 0.4); // Bonus Size Gain if Actor has perk
			// How it should work: when health is < 60%, empower growth by up to 50%. Max value at 10% health.
			if (amt > 0) {
				amt /= GetHP;
			} else if (amt < 0) {
				amt *= GetHP;
			}
		}
		if (actor_data) {
			if (amt - EPS < 0.0) {
				// If neative change always: allow
				float scale = actor_data->visual_scale;
				DistributeStolenAttributes(&actor, -amt * GetShrinkPenalty(scale)); // Adjust max attributes
				actor_data->target_scale += amt;
			} else if (actor_data->target_scale + amt < (actor_data->max_scale + EPS)) {
				// If change results is below max: allow it
				actor_data->target_scale += amt;
			} else if (actor_data->target_scale < (actor_data->max_scale - EPS)) {
				// If we are currently below max and we are scaling above max: make it max
				actor_data->target_scale = actor_data->max_scale;
			} else {
				// if we are over max then don't allow it
			}
		}
	}
	void mod_target_scale(Actor* actor, float amt) {
		if (actor) {
			mod_target_scale(*actor, amt);
		}
	}

	void set_max_scale(Actor& actor, float scale) {
		auto actor_data = Persistent::GetSingleton().GetData(&actor);
		if (actor_data) {
			actor_data->max_scale = scale;
		}
	}
	void set_max_scale(Actor* actor, float scale) {
		if (actor) {
			set_max_scale(*actor, scale);
		}
	}

	float get_max_scale(Actor& actor) {
		auto actor_data = Persistent::GetSingleton().GetData(&actor);
		if (actor_data) {
			return actor_data->max_scale;
		}
		return -1.0;
	}
	float get_max_scale(Actor* actor) {
		if (actor) {
			return get_max_scale(*actor);
		}
		return -1.0;
	}

	void mod_max_scale(Actor& actor, float amt) {
		auto actor_data = Persistent::GetSingleton().GetData(&actor);
		if (actor_data) {
			actor_data->max_scale += amt;
		}
	}
	void mod_max_scale(Actor* actor, float amt) {
		if (actor) {
			mod_max_scale(*actor, amt);
		}
	}

	float get_visual_scale(Actor& actor) {
		auto actor_data = Persistent::GetSingleton().GetData(&actor);
		if (actor_data) {
			return actor_data->visual_scale * get_natural_scale(actor);
		}
		return -1.0;
	}
	float get_visual_scale(Actor* actor) {
		if (actor) {
			return get_visual_scale(*actor);
		}
		return -1.0;
	}

	float get_natural_scale(Actor& actor) {
		auto actor_data = Transient::GetSingleton().GetData(&actor);
		if (actor_data) {
      if (actor_data->initialScale > 0.0) {
        return actor_data->otherScales * actor_data->initialScale;
      } else {
        // initialScale has not be found yet (actor not loaded)
        return actor_data->otherScales;
      }
		}
		return 1.0;
	}

	float get_natural_scale(Actor* actor) {
		if (actor) {
			return get_natural_scale(*actor);
		}
		return 1.0;
	}

	float get_neutral_scale(Actor* actor) {
		return 1.0;
	}

	float get_giantess_scale(Actor& actor) {
		auto actor_data = Persistent::GetSingleton().GetData(&actor);
		if (actor_data) {
			return actor_data->visual_scale;
		}
		return 1.0;
	}
	float get_giantess_scale(Actor* actor) {
		if (actor) {
			return get_giantess_scale(*actor);
		}
		return 1.0;
	}

}
