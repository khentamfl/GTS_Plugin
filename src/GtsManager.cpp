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
	
	void walk_nodes(Actor* actor) {
		if (!actor->Is3DLoaded()) {
			return;
		}
		auto model = actor->Get3D();
		auto name = model->name;

		std::deque<NiAVObject*> queue;
		queue.push_back(model.get());


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
					log::info("Node {}", currentnode->name);
				}
			}
			catch (const std::overflow_error& e) {
				log::info("Overflow: {}", e.what());
			} // this executes if f() throws std::overflow_error (same type rule)
			catch (const std::runtime_error& e) {
				log::info("Underflow: {}", e.what());
			} // this executes if f() throws std::underflow_error (base class rule)
			catch (const std::exception& e) {
				log::info("Exception: {}", e.what());
			} // this executes if f() throws std::logic_error (base class rule)
			catch (...) {
				log::info("Exception Other");
			}
		}
	}

	NiAVObject* find_node(Actor* actor, string& node_name) {
		if (!actor->Is3DLoaded()) {
			return nullptr;
		}
		auto model = actor->Get3D();
		auto name = model->name;

		std::deque<NiAVObject*> queue;
		queue.push_back(model.get());


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
				log::info("Overflow: {}", e.what());
			} // this executes if f() throws std::overflow_error (same type rule)
			catch (const std::runtime_error& e) {
				log::info("Underflow: {}", e.what());
			} // this executes if f() throws std::underflow_error (base class rule)
			catch (const std::exception& e) {
				log::info("Exception: {}", e.what());
			} // this executes if f() throws std::logic_error (base class rule)
			catch (...) {
				log::info("Exception Other");
			}
		}

		return nullptr;
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
			BSBound* bbx = dynamic_cast<BSBound*>(extra_bbx);
			float height = bbx->extents.z * 2; // Half widths so x2
			height *= actor->GetBaseHeight();
			return height;
		}
		return 0.0;
	}

	BSBound get_base_bound(Actor* actor) {
		// Using NiExtraNodeData, see get_base_height
		auto model = actor->Get3D();
		BSBound result;
		result.name = "BBX";
		result.center.x = 0.0;
		result.center.y = 0.0;
		result.center.z = 0.0;
		result.extents.x = 0.0;
		result.extents.y = 0.0;
		result.extents.z = 0.0;
		auto extra_bbx = model->GetExtraData("BBX");
		if (extra_bbx) {
			BSBound* bbx = dynamic_cast<BSBound*>(extra_bbx);
			result.center.x = bbx.center.x;
			result.center.y = bbx.center.y;
			result.center.z = bbx.center.z;
			result.extents.x = bbx.extents.x;
			result.extents.y = bbx.extents.y;
			result.extents.z = bbx.extents.z;

			float base_scale = actor->GetBaseHeight();
			result.center *= base_scale;
			result.extents *= base_scale;
		}
		return result;
	}

	float get_scale(Actor* actor) {
		auto node = find_node(actor, "NPC Root [Root]");
		if (node) {
			float scale = node->world.scale;
			return scale;
		}
		return 1.0;
	}

	void set_scale(Actor* actor, float scale) {
		auto node = find_node(actor, "NPC Root [Root]");
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

		auto model = actor->Get3D();
		auto name = model->name;

		auto base_actor = actor->GetActorBase();
		auto actor_name = base_actor->GetFullName();

		if (model) {
			float scale = get_scale(actor);
			float height = get_height(actor);

			auto char_controller = actor->GetCharController();
			if (char_controller) {
				char_controller->scale = scale;
				auto base_bound = get_base_bound(actor);
				char_controller->collisionBound.extents = base_bound.extents * scale;
				char_controller->collisionBound.center = base_bound.center * scale;
				// char_controller->bumperCollisionBound.extents = base_height->bumperCollisionBound.extents * scale;
				// char_controller->bumperCollisionBound.center = base_height->bumperCollisionBound.center * scale;
				char_controller->swimFloatHeight = height * 0.75;
				char_controller->actorHeight = height;
			}else {
				log::info("No char controller: {}", actor_name);
			}

			auto ai_process = actor->currentProcess;
			if (ai_process) {
				ai_process->SetCachedHeight(height);
				ai_process->cachedValues->cachedEyeLevel = height * 0.95;
			}else {
				log::info("No ai: {}", actor_name);
			}
			// }
		} else {
			log::info("No model: {}", actor_name);
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


		// log::info("Updating height of {}", actor_name);
		update_height(actor);
		// walk_nodes(actor);
	}
}
