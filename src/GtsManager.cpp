#include <SKSE/SKSE.h>
#include <GtsManager.h>
#include <vector>

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

	float get_height(NiPointer<Actor> actor) {
		auto model = actor->Get3D(false);
		if (model) {
			model->UpdateWorldBound();
		}
		auto min = actor->GetBoundMin();
		auto max = actor->GetBoundMax();
		auto diff = max.z - min.z;
		auto height = actor->GetBaseHeight() * diff;

		return height;
	}

	void walking_node(NiPointer<NiAVObject> node) {
		log::info("Node {}!", node->name);
	}
	void walk_nodes(NiPointer<Actor> actor) {
		auto model = actor->Get3D(false);
		if (!model) {
			return;
		}

		auto node = model->AsNode();
		if (node) {
			auto name = node->name;
			log::info("Root Node {}!", name);
			for (auto child: node->GetChildren()) {
				walking_node(child);
			}
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
		log::info("Poll.");

		auto actor_handles = find_actors();
		for (auto actor_handle: actor_handles) {
			auto actor = actor_handle.get();
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
