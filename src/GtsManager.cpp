#include <SKSE/SKSE.h>
#include <Config.h>
#include <GtsManager.h>
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	/**
	 * Find actors in ai manager that are loaded
	 */
	vector<ActorHandle> find_actors() {
		vector<ActorHandle> result;

		auto process_list = ProcessLists::GetSingleton();
		for (ActorHandle actor_handle: process_list->highActorHandles)
		{
			auto actor = actor_handle.get();
			if (actor && actor->Is3DLoaded())
			{
				result.push_back(actor_handle);
			}
		}

		return result;
	}

	float unit_to_meter(float unit) {
		// Game reports that the height of a slaughterfish is 0.31861934
		// From inspecting the bounding box of the slaughterfish and applying
		// base actor scales the unit height is 22.300568
		// Assuming 0.31861934 is meters and that bouding box is in model unit space
		// then the conversion factor is 70
		// Slaughterfish was chosen because it has scales of 1.0 (and was in my worldspace)
		// The scaling factor of 70 also applies to actor heights (once you remove)
		// race specific height scaling
		return unit / 70.0;
	}

	float meter_to_unit(float meter) {
		// Game reports that the height of a slaughterfish is 0.31861934
		// From inspecting the bounding box of the slaughterfish and applying
		// base actor scales the unit height is 22.300568
		// Assuming 0.31861934 is meters and that bouding box is in model unit space
		// then the conversion factor is 70
		// Slaughterfish was chosen because it has scales of 1.0 (and was in my worldspace)
		// The scaling factor of 70 also applies to actor heights (once you remove)
		// race specific height scaling
		return meter * 70.0;
	}

	void walk_nodes(Actor* actor) {
		if (!actor->Is3DLoaded()) {
			return;
		}
		auto model = actor->Get3D();
		auto name = model->name;

		std::deque<NiAVObject*> queue;
		queue.push_back(model);


		while (!queue.empty()) {
			auto currentnode = queue.front();
			queue.pop_front();
			try {
				if (currentnode) {
					auto ninode = currentnode->AsNode();
					if (ninode) {
						for (auto child: ninode->GetChildren()) {
							// Bredth first search
							queue.push_back(child.get());
							// Depth first search
							//queue.push_front(child.get());
						}
					}
					// Do smth
					log::trace("Node {}", currentnode->name);
				}
			}
			catch (const std::overflow_error& e) {
				log::warn("Overflow: {}", e.what());
			} // this executes if f() throws std::overflow_error (same type rule)
			catch (const std::runtime_error& e) {
				log::warn("Underflow: {}", e.what());
			} // this executes if f() throws std::underflow_error (base class rule)
			catch (const std::exception& e) {
				log::warn("Exception: {}", e.what());
			} // this executes if f() throws std::logic_error (base class rule)
			catch (...) {
				log::warn("Exception Other");
			}
		}
	}

	NiAVObject* find_node(Actor* actor, string& node_name) {
		if (!actor->Is3DLoaded()) {
			return nullptr;
		}
		auto model = actor->Get3D();
		if (!model) {
			return nullptr;
		}
		auto game_lookup = model->GetNodeByName(node_name);
		if (game_lookup) {
			return game_lookup;
		}

		// Game lookup failed we try and find it manually
		std::deque<NiAVObject*> queue;
		queue.push_back(model);


		while (!queue.empty()) {
			auto currentnode = queue.front();
			queue.pop_front();
			try {
				if (currentnode) {
					auto ninode = currentnode->AsNode();
					if (ninode) {
						for (auto child: ninode->GetChildren()) {
							// Bredth first search
							queue.push_back(child.get());
							// Depth first search
							//queue.push_front(child.get());
						}
					}
					// Do smth
					if  (currentnode->name.c_str() == node_name) {
						return currentnode;
					}
				}
			}
			catch (const std::overflow_error& e) {
				log::warn("Overflow: {}", e.what());
			} // this executes if f() throws std::overflow_error (same type rule)
			catch (const std::runtime_error& e) {
				log::warn("Underflow: {}", e.what());
			} // this executes if f() throws std::underflow_error (base class rule)
			catch (const std::exception& e) {
				log::warn("Exception: {}", e.what());
			} // this executes if f() throws std::logic_error (base class rule)
			catch (...) {
				log::warn("Exception Other");
			}
		}

		return nullptr;
	}

	void set_ref_scale(Actor* actor, float target_scale) {
		// This is how the game sets scale with the `SetScale` command
		// It is limited to x10 and messes up all sorts of things like actor damage
		// and anim speeds
		float refScale = static_cast<float>(actor->refScale) / 100.0F;
		log::info("REF Scale: {}", refScale);
		if (fabs(refScale - target_scale) > 1e-5) {
			actor->refScale = static_cast<std::uint16_t>(target_scale * 100.0F);
			actor->DoReset3D(false);
		}
		log::info("Set REF Scale: {}, {}", actor->refScale, static_cast<float>(actor->refScale));
	}

	void set_base_scale(Actor* actor, float target_scale) {
		// This will set the scale of the model root (not the root npc node)
		if (!actor->Is3DLoaded()) {
			return;
		}
		auto model = actor->Get3D();
		if (!model) {
			return;
		}
		model->local.scale = target_scale;
	}

	void set_npcnode_scale(Actor* actor, float target_scale) {
		// This will set the scale of the root npc node
		string node_name = "NPC Root [Root]";
		auto node = find_node(actor, node_name);
		if (node) {
			node->local.scale = target_scale;
		}
	}

	float get_height_min_max(Actor* actor) {
		const auto min = actor->GetBoundMin();
		const auto max = actor->GetBoundMax();
		const auto diff = max.z - min.z;
		const auto height = actor->GetBaseHeight() * diff;
		return height;
	}

	float get_base_height(Actor* actor) {
		// The NiExtraNodeData has the original bounding box data.
		// The data in GetCharController is seperate from that in NiExtraNodeData
		// We can therefore use NiExtraNodeData as the base height.
		// Caveat the data in GetBoundMin use the NiExtraNodeData
		// I am not sure if it is better to change NiExtraNodeData so that
		// GetBoundMin changes or to change the data in GetCharController
		// that seems to govern the AI and controls
		//
		// The bb values on NiExtraNodeData might be shared between all actors of the
		// same skeleton. Changing on actorA seems to also change on actorB (needs confirm)
		//
		// Plan:
		// Use NiExtraNodeData for base height data
		// Use GetCharController for current height data
		//
		// Return 0.0 if we cannot get base height
		auto model = actor->Get3D();
		auto extra_bbx = model->GetExtraData("BBX");
		if (extra_bbx) {
			BSBound* bbx = static_cast<BSBound*>(extra_bbx);
			float height = bbx->extents.z * 2; // Half widths so x2
			height *= actor->GetBaseHeight();
			return height;
		}
		return 0.0;
	}

	struct CachedBound {
		string name;
		float center[3];
		float extents[3];
	};

	CachedBound get_base_bound(Actor* actor) {
		// Using NiExtraNodeData, see get_base_height
		auto model = actor->Get3D();
		CachedBound result;
		result.name = "BBX";
		result.center[0] = 0.0;
		result.center[1] = 0.0;
		result.center[2] = 0.0;
		result.extents[0] = 0.0;
		result.extents[1] = 0.0;
		result.extents[2] = 0.0;
		auto extra_bbx = model->GetExtraData("BBX");
		if (extra_bbx) {
			BSBound* bbx = static_cast<BSBound*>(extra_bbx);
			float base_scale = actor->GetBaseHeight();
			result.center[0] = bbx->center.x * base_scale;
			result.center[1] = bbx->center.y * base_scale;
			result.center[2] = bbx->center.z * base_scale;
			result.extents[0] = bbx->extents.x * base_scale;
			result.extents[1] = bbx->extents.y * base_scale;
			result.extents[2] = bbx->extents.z * base_scale;
		}
		return result;
	}

	float get_scale(Actor* actor) {
		string node_name = "NPC Root [Root]";
		auto node = find_node(actor, node_name);
		if (node) {
			float scale = node->world.scale;
			return scale;
		}
		return 1.0;
	}

	void set_scale(Actor* actor, float scale) {
		string node_name = "NPC Root [Root]";
		auto node = find_node(actor, node_name);
		if (node) {
			node->world.scale =  scale;
		}
	}

	float get_height(Actor* actor) {
		float scale = get_scale(actor);
		float base_height = get_base_height(actor);
		return base_height * scale;
	}

	void update_height(Actor* actor) {
		if (!actor->Is3DLoaded()) {
			return;
		}

		auto base_actor = actor->GetActorBase();
		auto actor_name = base_actor->GetFullName();
		log::trace("Updating height of: {}", actor_name);

		float scale = get_scale(actor);
		log::trace("Actor scale: {}", scale);
		float base_height = get_base_height(actor);
		log::trace("Actor base height: {}", base_height);
		float height = get_height(actor);
		log::trace("Actor height: {}", height);
		float height_min_max = get_height_min_max(actor);
		log::trace("Actor height min max: {}", height_min_max);

		log::trace("Getting base bounds");
		auto base_bound = get_base_bound(actor);




		log::trace("Getting character controller");
		auto char_controller = actor->GetCharController();
		if (char_controller) {
			log::trace("Current height data");
			log::trace("  - Scale: {}", char_controller->scale);
			log::trace("  - Height: {}", char_controller->actorHeight);
			log::trace("  - SwimFloat: {}", char_controller->swimFloatHeight);

			char_controller->scale = scale;
			char_controller->collisionBound.extents.x = base_bound.extents[0] * scale;
			char_controller->collisionBound.extents.y = base_bound.extents[1] * scale;
			char_controller->collisionBound.extents.z = base_bound.extents[2] * scale;
			char_controller->collisionBound.center.x = base_bound.center[0] * scale;
			char_controller->collisionBound.center.y = base_bound.center[1] * scale;
			char_controller->collisionBound.center.z = base_bound.center[2] * scale;
			char_controller->actorHeight = unit_to_meter(height);
			if (char_controller->swimFloatHeight > 1e-4) {
				// Fishes have a value of 0.0
				char_controller->swimFloatHeight = unit_to_meter(height * (1.6/1.8288));
			}
			log::trace("Data updated");
		}else {
			log::debug("No char controller: {}", actor_name);
		}

		auto ai_process = actor->currentProcess;
		if (ai_process) {
			ai_process->SetCachedHeight(height);
			ai_process->cachedValues->cachedEyeLevel = height * 0.95;
		}else {
			log::debug("No ai: {}", actor_name);
		}
	}
}

GtsManager& GtsManager::GetSingleton() noexcept {
	static GtsManager instance;
	return instance;
}

// Poll for updates
void GtsManager::poll() {
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

		auto actor_handles = find_actors();
		for (auto actor_handle: actor_handles) {
			auto sptr_actor = actor_handle.get();
			if (sptr_actor) {
				auto actor = sptr_actor.get();

				// Do smth
			}
		}

	}
}

void GtsManager::poll_actor(Actor* actor) {
	if (actor) {
		auto base_actor = actor->GetActorBase();
		auto actor_name = base_actor->GetFullName();

		auto race = actor->GetRace();
		auto race_name = race->GetFullName();


		// log::trace("Updating height of {}", actor_name);
		update_height(actor);
		// walk_nodes(actor);
	}
}
