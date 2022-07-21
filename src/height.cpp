#include "height.h"
#include "util.h"
#include "GtsManager.h"
#include "persistent.h"
#include "transient.h"

using namespace Gts;

namespace Gts {
	void set_target_height(Actor* actor, float height) {
		if (!actor) {
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return;
		}
		float scale = height / temp_actor_data->base_height;
		if (actor_data) {
			actor_data->target_scale = scale;
		}
	}

	float get_target_height(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return 0.0;
		}
		if (actor_data) {
			float scale = actor_data->target_scale;
			return scale * temp_actor_data->base_height;
		}
		return 0.0;
	}

	void mod_target_height(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return;
		}
		if (!actor_data) {
			return;
		}
		float current_scale = actor_data->target_scale;
		float current_height = current_scale * temp_actor_data->base_height;
		float target_height = (current_height + amt);
		float target_scale = target_height / temp_actor_data->base_height;
		float scale_delta = target_scale - current_scale;

		actor_data->target_scale += scale_delta;
	}

	void set_max_height(Actor* actor, float height) {
		if (!actor) {
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return;
		}
		float scale = height / temp_actor_data->base_height;
		if (actor_data) {
			actor_data->max_scale = scale;
		}
	}

	float get_max_height(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return 0.0;
		}
		if (actor_data) {
			float scale = actor_data->max_scale;
			return scale * temp_actor_data->base_height;
		}
		return 0.0;
	}
	void mod_max_height(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return;
		}
		if (actor_data) {
			float current_scale = actor_data->max_scale;
			float current_height = current_scale * temp_actor_data->base_height;
			float target_height = (current_height + amt);
			float target_scale = target_height / temp_actor_data->base_height;
			float scale_delta = target_scale - current_scale;

			actor_data->max_scale += scale_delta;
		}
	}

	float get_visual_height(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return 0.0;
		}
		if (actor_data) {
			float scale = actor_data->visual_scale;
			return scale * temp_actor_data->base_height;
		}
		return 0.0;
	}
}
