#include "scale/height.hpp"
#include "scale/scale.hpp"
#include "managers/GtsManager.hpp"
#include "data/transient.hpp"

using namespace Gts;

namespace {
	float height_to_scale(Actor* actor, float height) {
		if (!actor) {
			return -1.0;
		}
		auto temp_actor_data = Transient::GetSingleton().GetData(actor);
		if (!temp_actor_data) {
			return -1.0;
		}
		return height / temp_actor_data->base_height;
	}

	float scale_to_height(Actor* actor, float scale) {
		if (!actor) {
			return -1.0;
		}
		auto temp_actor_data = Transient::GetSingleton().GetData(actor);
		if (!temp_actor_data) {
			return -1.0;
		}
		return scale * temp_actor_data->base_height;
	}

}

namespace Gts {
	void set_target_height(Actor* actor, float height) {
		float scale = height_to_scale(actor, height);
		set_target_scale(actor, scale);
	}

	float get_target_height(Actor* actor) {
		float scale = get_target_scale(actor);
		return scale_to_height(actor, scale);
	}

	void mod_target_height(Actor* actor, float amt) {
		float current_scale = get_target_scale(actor);
		float current_height = scale_to_height(actor, current_scale);
		float target_height = (current_height + amt);
		float target_scale = height_to_scale(actor, target_height);
		float scale_delta = target_scale - current_scale;
		update_target_scale(actor, scale_delta);
	}

	void set_max_height(Actor* actor, float height) {
		float scale = height_to_scale(actor, height);
		set_max_scale(actor, scale);
	}

	float get_max_height(Actor* actor) {
		float scale = get_max_scale(actor);
		return scale_to_height(actor, scale);
	}
	void mod_max_height(Actor* actor, float amt) {
		float current_scale = get_max_scale(actor);
		float current_height = scale_to_height(actor, current_scale);
		float target_height = (current_height + amt);
		float target_scale = height_to_scale(actor, target_height);
		float scale_delta = target_scale - current_scale;
		mod_max_scale(actor, scale_delta);
	}

	float get_visual_height(Actor* actor) {
		float scale = get_visual_scale(actor);
		return scale_to_height(actor, scale);
	}

	float get_base_height(Actor* actor) {
		auto temp_actor_data = Transient::GetSingleton().GetData(actor);
		if (!temp_actor_data) {
			return -1.0;
		}
		return temp_actor_data->base_height;
	}
}
