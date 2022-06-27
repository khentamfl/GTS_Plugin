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

	float get_height(Actor* actor) {
		auto model = actor->Get3D(false);
		if (model) {
			float previous_radius = model->worldBound.radius;
			model->UpdateWorldBound();
			float new_radius = model->worldBound.radius;
			log::info("Old: {}, New: {}", previous_radius, new_radius);
			float factor = new_radius/previous_radius;
			if (fabs(factor - 1.0) > 1e-4) {
				auto char_controller = actor->GetCharController();
				if (char_controller) {
					log::info("Updating collision bounds");
					char_controller->collisionBound.extents *= factor;
					char_controller->bumperCollisionBound.extents *= factor;
				}
			}
		}
		auto min = actor->GetBoundMin();
		auto max = actor->GetBoundMax();
		auto diff = max.z - min.z;
		auto height = actor->GetBaseHeight() * diff;

		return height;
	}

	void walk_nodes(Actor* actor) {
		if (!actor->Is3DLoaded()) {
			return;
		}
		if (!actor || !actor->loadedData || !actor->loadedData->data3D) {
			return;
		}
		auto model = actor->loadedData->data3D;
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
		if (!actor || !actor->loadedData || !actor->loadedData->data3D) {
			return nullptr;
		}
		auto model = actor->loadedData->data3D;
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

	void update_height(Actor* actor) {
		if (!actor->Is3DLoaded()) {
			return;
		}
		if (!actor || !actor->loadedData || !actor->loadedData->data3D) {
			return;
		}
		auto model = actor->Get3D();
		auto name = model->name;
		log::info("Model name: {}", name);

		auto base_actor = actor->GetActorBase();
		auto actor_name = base_actor->GetFullName();

		if (model) {
			string root_name = "NPC Root [Root]";
			NiAVObject* root_node = find_node(actor, root_name);
			if (root_node) {
				auto world_transform = root_node->local;
				float scale = world_transform.scale;

				// if (fabs(scale - 1.0) >= 1e-5) {
				auto char_controller = actor->GetCharController();
				if (char_controller) {
					char_controller->scale = 10.;
					const auto min = actor->GetBoundMin();
					const auto max = actor->GetBoundMax();
					const auto diff = max.z - min.z;
					const auto height = actor->GetBaseHeight() * diff;
					log::info("Updated height: {}", height);
					// log::info("Updating collision bounds");
					// char_controller->collisionBound.extents = base_height->collisionBound.extents * scale;
					// char_controller->collisionBound.center = base_height->collisionBound.center * scale;
					// log::info("Updating bumper collision bounds");
					// char_controller->bumperCollisionBound.extents = base_height->bumperCollisionBound.extents * scale;
					// char_controller->bumperCollisionBound.center = base_height->bumperCollisionBound.center * scale;
					//
					// char_controller->swimFloatHeight = base_height->swimFloatHeight * scale;
					// log::info("Updated water float height: {}", char_controller->swimFloatHeight);
					//
					// char_controller->actorHeight = base_height->actorHeight * scale;
					// log::info("Updated char height: {}", char_controller->actorHeight);
				}else {
					log::info("No char controller: {}", actor_name);
				}

				auto ai_process = actor->currentProcess;
				if (ai_process) {
					const auto min = actor->GetBoundMin();
					const auto max = actor->GetBoundMax();
					const auto diff = max.z - min.z;
					const auto height = actor->GetBaseHeight() * diff;
					ai_process->SetCachedHeight(height);
					log::info("Updated cached ai height: {}", ai_process->GetCachedHeight());

					ai_process->cachedValues->cachedEyeLevel = height * 0.95;
					log::info("Updated cached ai eye level: {}", ai_process->cachedValues->cachedEyeLevel);
				}else {
					log::info("No ai: {}", actor_name);
				}
			} else {
				log::info("No root: {}", actor_name);
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
		walk_nodes(actor);
	}
}
