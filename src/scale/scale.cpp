#include "scale/height.hpp"
#include "util.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"

using namespace Gts;

namespace Gts {
	void set_target_scale(Actor* actor, float scale) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				actor_data->target_scale = scale;
				actor_data->target_scale_v = 0.0;
			}
		}
	}

	float get_target_scale(Actor* actor) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				return actor_data->target_scale;
			}
		}
		return -1.0;
	}

	void mod_target_scale(Actor* actor, float amt) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				actor_data->target_scale += amt;
				actor_data->target_scale_v = 0.0;
			}
		}
	}

	void set_max_scale(Actor* actor, float scale) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				actor_data->max_scale = scale;
			}
		}
	}

	float get_max_scale(Actor* actor) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				return actor_data->max_scale;
			}
		}
		return -1.0;
	}
	void mod_max_scale(Actor* actor, float amt) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				actor_data->max_scale += amt;
			}
		}
	}

	float get_visual_scale(Actor* actor) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				return actor_data->visual_scale;
			}
		}
		return -1.0;
	}

	float get_natural_scale(Actor* actor) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				return actor_data->native_scale;
			}
		}
		return -1.0;
	}

	float get_effective_scale(Actor* actor) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				return actor_data->visual_scale * actor_data->effective_multi;
			}
		}
		return -1.0;
	}
}
