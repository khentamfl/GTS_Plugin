#include <SKSE/SKSE.h>
#include <Config.h>
#include <managers/GtsManager.h>
#include <data/persistent.h>
#include <data/transient.h>
#include <data/runtime.h>
#include <managers/highheel.h>
#include "util.h"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	void update_height(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data) {
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}
		if (!trans_actor_data) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		float target_scale = min(persi_actor_data->target_scale, persi_actor_data->max_scale);
		if (fabs(target_scale - persi_actor_data->visual_scale) < 1e-5) {
			return;
		}
		float minimum_scale_delta = 0.005; // 0.5%
		if (fabs(target_scale - persi_actor_data->visual_scale) < minimum_scale_delta) {
			persi_actor_data->visual_scale = target_scale;
			persi_actor_data->visual_scale_v = 0.0;
		} else {
			critically_damped(
				persi_actor_data->visual_scale,
				persi_actor_data->visual_scale_v,
				target_scale,
				persi_actor_data->half_life,
				*g_delta_time
				);
		}
	}
	void apply_height(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data, bool force = false) {
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}
		if (!trans_actor_data) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		float scale = get_scale(actor);
		if (scale < 0.0) {
			return;
		}
		float visual_scale = persi_actor_data->visual_scale;

		// Is scale correct already?
		if (fabs(visual_scale - scale) <= 1e-5 && !force) {
			return;
		}

		// Is scale too small
		if (visual_scale <= 1e-5) {
			return;
		}

		// log::trace("Scale changed from {} to {}. Updating",scale, visual_scale);
		set_scale(actor, visual_scale);
	}

	void apply_speed(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data, bool force = false) {
		if (!Persistent::GetSingleton().is_speed_adjusted) {
			return;
		}
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}
		if (!trans_actor_data) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}

		float scale = persi_actor_data->visual_scale;
		if (scale < 1e-5) {
			return;
		}
		SoftPotential& speed_adjustment = Persistent::GetSingleton().speed_adjustment;
		float speed_mult = soft_core(scale, speed_adjustment);
		persi_actor_data->anim_speed = speed_mult;
		actor->SetActorValue(ActorValue::kSpeedMult, trans_actor_data->base_walkspeedmult / speed_mult);

		// Experiement
		if (false) {
			auto& rot_speed = actor->currentProcess->middleHigh->rotationSpeed;
			if (fabs(rot_speed.x) > 1e-5 || fabs(rot_speed.y) > 1e-5 || fabs(rot_speed.z) > 1e-5) {
				log::info("{} rotationSpeed: {},{},{}", actor_name(actor), rot_speed.x,rot_speed.y,rot_speed.z);
				actor->currentProcess->middleHigh->rotationSpeed.x *= speed_mult;
				actor->currentProcess->middleHigh->rotationSpeed.y *= speed_mult;
				actor->currentProcess->middleHigh->rotationSpeed.z *= speed_mult;
			}
			auto& animationDelta = actor->currentProcess->high->animationDelta;
			if (fabs(animationDelta.x) > 1e-5 || fabs(animationDelta.y) > 1e-5 || fabs(animationDelta.z) > 1e-5) {
				log::info("{} animationDelta: {},{},{}", actor_name(actor), animationDelta.x,animationDelta.y,animationDelta.z);
			}
			auto& animationAngleMod = actor->currentProcess->high->animationAngleMod;
			if (fabs(animationAngleMod.x) > 1e-5 || fabs(animationAngleMod.y) > 1e-5 || fabs(animationAngleMod.z) > 1e-5) {
				log::info("{} animationAngleMod: {},{},{}", actor_name(actor), animationAngleMod.x,animationAngleMod.y,animationAngleMod.z);
			}
			auto& pathingCurrentRotationSpeed = actor->currentProcess->high->pathingCurrentRotationSpeed;
			if (fabs(pathingCurrentRotationSpeed.x) > 1e-5 || fabs(pathingCurrentRotationSpeed.y) > 1e-5 || fabs(pathingCurrentRotationSpeed.z) > 1e-5) {
				log::info("{} pathingCurrentRotationSpeed: {},{},{}", actor_name(actor), pathingCurrentRotationSpeed.x,pathingCurrentRotationSpeed.y,pathingCurrentRotationSpeed.z);
			}
			auto& pathingDesiredRotationSpeed = actor->currentProcess->high->pathingDesiredRotationSpeed;
			if (fabs(pathingDesiredRotationSpeed.x) > 1e-5 || fabs(pathingDesiredRotationSpeed.y) > 1e-5 || fabs(pathingDesiredRotationSpeed.z) > 1e-5) {
				log::info("{} pathingDesiredRotationSpeed: {},{},{}", actor_name(actor), pathingDesiredRotationSpeed.x,pathingDesiredRotationSpeed.y,pathingDesiredRotationSpeed.z);
			}
		}
	}

	void update_effective_multi(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data) {
		if (!actor) {
			return;
		}
		if (!persi_actor_data) {
			return;
		}
		auto small_massive_threat = Runtime::GetSingleton().smallMassiveThreat;
		if (!small_massive_threat) {
			return;
		}
		if (actor->HasMagicEffect(small_massive_threat)) {
			persi_actor_data->effective_multi = 4.0;
		} else {
			persi_actor_data->effective_multi = 1.0;
		}
	}

	void update_actor(Actor* actor) {
		Transient::GetSingleton().UpdateActorData(actor);

		auto temp_data = Transient::GetSingleton().GetActorData(actor);
		auto saved_data = Persistent::GetSingleton().GetActorData(actor);
		update_effective_multi(actor, saved_data, temp_data);
		update_height(actor, saved_data, temp_data);
	}

	void apply_actor(Actor* actor, bool force = false) {
		auto temp_data = Transient::GetSingleton().GetData(actor);
		auto saved_data = Persistent::GetSingleton().GetData(actor);
		apply_height(actor, saved_data, temp_data, force);
		apply_speed(actor, saved_data, temp_data, force);
	}
}

GtsManager& GtsManager::GetSingleton() noexcept {
	static GtsManager instance;

	static std::atomic_bool initialized;
	static std::latch latch(1);
	if (!initialized.exchange(true)) {
		latch.count_down();
	}
	latch.wait();

	return instance;
}

// Poll for updates
void GtsManager::poll() {
	if (!this->enabled) {
		return;
	}
	auto player_char = RE::PlayerCharacter::GetSingleton();
	if (!player_char) {
		return;
	}
	if (!player_char->Is3DLoaded()) {
		return;
	}

	auto ui = RE::UI::GetSingleton();
	if (!ui->GameIsPaused()) {
		const auto& frame_config = Gts::Config::GetSingleton().GetFrame();
		auto init_delay = frame_config.GetInitDelay();
		auto step = frame_config.GetStep() + 1; // 1 Based index

		auto current_frame = this->frame_count.fetch_add(1);
		if (current_frame < init_delay) {
			return;
		}
		if ((current_frame - init_delay) % step != 0) {
			return;
		}

		auto actors = find_actors();
		for (auto actor: actors) {
			if (!actor) {
				continue;
			}
			if (!actor->Is3DLoaded()) {
				continue;
			}
			update_actor(actor);
			apply_actor(actor);
		}
	}
}

// Fired during the Papyrus OnUpdate event
void GtsManager::on_update() {
	if (!this->enabled) {
		return;
	}
	auto player_char = RE::PlayerCharacter::GetSingleton();
	if (!player_char) {
		return;
	}
	if (!player_char->Is3DLoaded()) {
		return;
	}
	auto actors = find_actors();
	for (auto actor: actors) {
		if (!actor) {
			continue;
		}
		if (!actor->Is3DLoaded()) {
			continue;
		}
		auto temp_data = Transient::GetSingleton().GetData(actor);
		auto saved_data = Persistent::GetSingleton().GetData(actor);
		apply_highheel(actor, temp_data, true);
	}
}

void GtsManager::reapply(bool force) {
	// Get everyone in loaded AI data and reapply
	auto actors = find_actors();
	for (auto actor: actors) {
		if (!actor) {
			continue;
		}
		if (!actor->Is3DLoaded()) {
			continue;
		}
		reapply_actor(actor, force);
	}
}
void GtsManager::reapply_actor(Actor* actor, bool force) {
	// Reapply just this actor
	if (!actor) {
		return;
	}
	if (!actor->Is3DLoaded()) {
		return;
	}
	apply_actor(actor, force);
}
