#include <SKSE/SKSE.h>
#include <Config.h>
#include <GtsManager.h>
#include <persistent.h>
#include <transient.h>
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	void smooth_height_change(Actor* actor, ActorData* persi_actor_data, TempActorData* trans_actor_data) {
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
		critically_damped(
			persi_actor_data->visual_scale,
			persi_actor_data->visual_scale_v,
			min(persi_actor_data->target_scale, persi_actor_data->max_scale),
			0.05,
			*g_delta_time
			);
	}
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
		log::info("    + get_scale");
		float scale = get_scale(actor);
		log::info("    - get_scale");
		float visual_scale = persi_actor_data->visual_scale;

		// Is scale correct already?
		if (fabs(visual_scale - scale) <= 1e-5) {
			return;
		}

		// Is scale too small
		if (visual_scale <= 1e-5) {
			return;
		}

		log::info("Scale changed from {} to {}. Updating",scale, visual_scale);
		set_scale(actor, visual_scale);
		NiUpdateData ctx;
		auto model = actor->Get3D(false);
		// We are on the main thread so we can update this now
		if (model) {
			model->UpdateWorldData(&ctx);
		}
		auto first_model = actor->Get3D(true);
		if (first_model) {
			first_model->UpdateWorldData(&ctx);
		}
	}

	void update_actor(Actor* actor) {
		auto temp_data = Transient::GetSingleton().GetActorData(actor);
		auto saved_data = Persistent::GetSingleton().GetActorData(actor);
		log::info("    + smooth_height_change");
		smooth_height_change(actor, saved_data, temp_data);
		log::info("  - smooth_height_change");
		log::info("  + update_height");
		update_height(actor, saved_data, temp_data);
		log::info("  - update_height");
	}
}

GtsManager& GtsManager::GetSingleton() noexcept {
	static GtsManager instance;

	static std::atomic_bool initialized;
	static std::latch latch(1);
	if (!initialized.exchange(true)) {
		instance.size_method = SizeMethod::All;
		latch.count_down();
	}
	latch.wait();

	return instance;
}

// Poll for updates
void GtsManager::poll() {
	log::info("+ Poll");
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
		log::info("  + Walking Actors");
		auto actors = find_actors();
		int i = 0;
		int count = actors.size();
		for (auto actor: actors) {
			log::info("Iter {} of {}", i, count);
			i += 1;
			if (!actor) {
				continue;
			}
			if (!actor->Is3DLoaded()) {
				continue;
			}
			update_actor(actor);
		}
		log::info("  - Walking Actors");
		log::info("  + Update player");
		update_actor(player_char);
		log::info("  - Update player");
	}
	log::info("- Poll");
}
