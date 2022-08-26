
#include "Config.hpp"
#include "managers/GtsManager.hpp"
#include "managers/highheel.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "UI/DebugAPI.hpp"
#include "util.hpp"
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
		SoftPotential& MS_adjustment = Persistent::GetSingleton().MS_adjustment;
		float speed_mult = soft_core(scale, speed_adjustment);
		persi_actor_data->anim_speed = speed_mult;
		float MS_mult = soft_core(scale, MS_adjustment);
		persi_actor_data->anim_speed = MS_mult;
		actor->SetActorValue(ActorValue::kSpeedMult, trans_actor_data->base_walkspeedmult / MS_mult);
		if (actor->IsWalking() == true) {
			actor->SetActorValue(ActorValue::kSpeedMult, trans_actor_data->base_walkspeedmult * 0.50 / MS_mult);
		} else if (actor->IsSprinting() == true) {
			actor->SetActorValue(ActorValue::kSpeedMult, trans_actor_data->base_walkspeedmult * 1.28 / MS_mult);
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
			persi_actor_data->effective_multi = 2.0;
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

	enum ChosenGameMode {
		None,
		Grow,
		Shrink,
		Standard,
	};
	// Handles changes like slowly loosing height
	// over time
	void GameMode(Actor* actor)  {
		auto& runtime = Runtime::GetSingleton();
		float size_limit = runtime.sizeLimit->value;

		set_max_scale(actor, size_limit);
		if (get_target_scale(actor) > size_limit) {
			set_target_scale(actor, size_limit);
		}

		ChosenGameMode game_mode = ChosenGameMode::None;
		int game_mode_int = 0;
		if (actor->formID == 0x14 ) {
			game_mode_int = runtime.ChosenGameMode->value;

		} else if (actor->IsPlayerTeammate()) {
			int game_mode_int = runtime.ChosenGameModeNPC->value;
		}
		if (game_mode_int >=0 && game_mode_int <= 3) {
			game_mode =  static_cast<ChosenGameMode>(game_mode_int);
		}

		if (game_mode != ChosenGameMode::None) {
			float GrowthRate = runtime.GrowthModeRate->value;
			float ShrinkRate = runtime.ShrinkModeRate->value;

			float natural_scale = 1.0;
			float Scale = get_visual_scale(actor);
			switch (game_mode) {
				case ChosenGameMode::Grow: {
					mod_target_scale(actor, Scale * (0.00010 + (GrowthRate * 0.25)));
					break;
				}
				case ChosenGameMode::Shrink: {
					if (Scale > natural_scale) {
						mod_target_scale(actor, Scale * -(0.00025 + (ShrinkRate * 0.25)));
					}
					break;
				}
				case ChosenGameMode::Standard: {
					if (actor->IsInCombat()) {
						mod_target_scale(actor, Scale * (0.00008 + (GrowthRate * 0.17)));
					} else {
						mod_target_scale(actor, Scale * -(0.00029 + (ShrinkRate * 0.34)));
					}
				}
			}
		}
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

		for (auto actor: find_actors()) {
			if (!actor) {
				continue;
			}
			if (!actor->Is3DLoaded()) {
				continue;
			}
			update_actor(actor);
			apply_actor(actor);
			GameMode(actor);
		}
	}

	auto model = player_char->GetCurrent3D();
	if (model) {
		auto spine_node = model->GetObjectByName("NPC Spine [Spn0]");
		if (spine_node) {
			DebugAPI::DrawSphere(Ni2Glm(spine_node->world.translate), meter_to_unit(2.0));
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
