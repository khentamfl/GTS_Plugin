#include <SKSE/SKSE.h>
#include <Config.h>
#include <GtsManager.h>
#include <persistent.h>
#include <transient.h>
#include <highheel.h>
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
		float scale = get_scale(actor);
		if (scale < 0.0) {
			return;
		}
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

		for (bool person: {false, true}) {
			NiAVObject* model = nullptr;
			switch (Persistent::GetSingleton().size_method) {
				case SizeMethod::ModelScale:
				{
					model = actor->Get3D(person);
					break;
				}
				case SizeMethod::RootScale:
				{
					string node_name = "NPC Root [Root]";
					model = find_node(actor, node_name, person);
					break;
				}
			}
			// We are on the main thread so we can update this now
			if (model) {
				NiUpdateData ctx;
				model->UpdateWorldData(&ctx);
			}
		}
	}

	void experiment001(Actor* actor) {
		if (!actor) {
			return;
		}
		auto HighProcess = actor->currentProcess->high;
		if (!HighProcess) {
			return;
		}

		log::info("Actor {} has an animationDelta: {},{},{}", actor_name(actor), HighProcess->animationDelta.x, HighProcess->animationDelta.y, HighProcess->animationDelta.z);
	}

	void experiment002(Actor* actor) {
		if (!actor) {
			return;
		}
		auto middlehighprocess = actor->currentProcess->middleHigh;
		if (!middlehighprocess) {
			return;
		}
		auto thisAGmanager = middlehighprocess->animationGraphManager.get();
		if (!thisAGmanager) {
			return;
		}
		auto thisgraph = thisAGmanager->graphs.begin()->get();
		if (!thisgraph) {
			return;
		}
		log::info("Actor {} has an anim graph", actor_name(actor));
	}

	void experiment003(Actor* actor) {
		if (!actor) {
			return;
		}
		auto middlehighprocess = actor->currentProcess->middleHigh;
		if (!middlehighprocess) {
			return;
		}
		auto thisAGmanager = middlehighprocess->animationGraphManager.get();
		if (!thisAGmanager) {
			return;
		}

		log::info("Actor {} bound channels", actor_name(actor));
		for (auto boundChannel: thisAGmanager->boundChannels) {
			std::string channelName = boundChannel->channelName.c_str();
			log::info("  - channelName: {}", channelName);
			log::info("  - Value (int): {}", boundChannel->value);
			log::info("  - Value (float): {}", reinterpret_cast<float &>(boundChannel->value));
            if (channelName == "TimeDelta") {
                float& value = reinterpret_cast<float &>(boundChannel->value);
                value *= 0.12;
                log::info("  - New Value (float) multipled by 0.12: {}", reinterpret_cast<float &>(boundChannel->value));
            }
		}
		log::info("Actor {} bumped channels", actor_name(actor));
		for (auto bumpedChannel: thisAGmanager->bumpedChannels) {
			std::string channelName = bumpedChannel->channelName.c_str();
			log::info("  - channelName: {}", channelName);
			log::info("  - Value (int): {}", bumpedChannel->value);
			log::info("  - Value (float): {}", reinterpret_cast<float &>(bumpedChannel->value));
		}
	}

	void experiment004(Actor* actor) {
		if (!actor) {
			return;
		}
		auto middlehighprocess = actor->currentProcess->middleHigh;
		if (!middlehighprocess) {
			return;
		}
		auto variables = middlehighprocess->animationVariableCache;
		if (variables) {
			return;
		}
		log::info("Actor {} has variables", actor_name(actor));
		for (auto variable: variables->variableCache) {
			std::string name = variable.variableName.c_str();
			log::info("  - Variable name {}", name);
		}
	}

	void update_actor(Actor* actor) {
		auto temp_data = Transient::GetSingleton().GetActorData(actor);
		auto saved_data = Persistent::GetSingleton().GetActorData(actor);
		smooth_height_change(actor, saved_data, temp_data);
		update_height(actor, saved_data, temp_data);
		apply_high_heel_scale(actor, temp_data);
		experiment003(actor);
		experiment004(actor);
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
		int i = 0;
		int count = actors.size();
		for (auto actor: actors) {
			i += 1;
			if (!actor) {
				continue;
			}
			if (!actor->Is3DLoaded()) {
				continue;
			}
			update_actor(actor);
		}
	}
}

void GtsManager::poll_actor(Actor* actor) {
	if (!actor) {
		return;
	}
	if (!actor->Is3DLoaded()) {
		return;
	}
	auto saved_data = Persistent::GetSingleton().GetActorData(actor);
	if (!saved_data) {
		return;
	}
	float scale = get_scale(actor);
	if (scale < 0.0) {
		return;
	}
	float visual_scale = saved_data->visual_scale;

	// Is scale correct already?
	if (fabs(visual_scale - scale) <= 1e-5) {
		return;
	}

	// Is scale too small
	if (visual_scale <= 1e-5) {
		return;
	}

	log::info("Scale changed from {} to {}. Updating on ActorUpdate",scale, visual_scale);
	set_scale(actor, visual_scale);
}
