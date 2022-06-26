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

	void walking_node(NiAVObject* node, std::string& indent) {
		if (!node) {
			return;
		}
		std::string name = ""; //node->name.c_str();
		if (name.data()) {
			if (!node->name.empty()) {
				log::info("{}Node {}", indent, node->name);
			} else {
				log::info("{}Node <Unnamed>", indent);
			}
		} else {
			log::info("{}Node <Invalid>", indent);
		}
		NiNode* ni_node = node->AsNode();
		if (ni_node) {
			const auto children = ni_node->GetChildren();
			if (children.begin()) {
				if (!children.empty()) {
					auto size = children.size();
					NiAVObject* child;
					for(std::size_t i = 0; i < size; ++i) {
						child = children[i].get();
						indent.push_back(' ');
						indent.push_back(' ');
						if (child) {
							walking_node(child, indent);
						}
						indent.pop_back();
						indent.pop_back();
					}
				}
			}
		}
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
		std::string indent = "  ";
		walking_node(model.get(), indent);
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
		if (current_frame % step != 0) {
			return;
		}

		auto actor_handles = find_actors();
		for (auto actor_handle: actor_handles) {
			auto sptr_actor = actor_handle.get();
			if (sptr_actor) {
				auto actor = sptr_actor.get();
				if (actor) {
					auto base_actor = actor->GetActorBase();
					auto actor_name = base_actor->GetFullName();

					auto race = actor->GetRace();
					auto race_name = race->GetFullName();

					auto height = get_height(actor);

					log::info("Actor {} with race {} found with height {}!", actor_name, race_name, height);
					walk_nodes(actor);
				}
			}
		}

	}
}
