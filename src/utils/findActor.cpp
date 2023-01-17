#include "utils/findActor.hpp"

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	/**
	 * Find actors in ai manager that are loaded
	 */
	vector<Actor*> find_actors() {
		vector<Actor*> result;

		auto high_actors = find_actors_high();
		result.insert(result.end(), high_actors.begin(), high_actors.end());

		auto middle_high_actors = find_actors_middle_high();
		result.insert(result.end(), middle_high_actors.begin(), middle_high_actors.end());

		auto middle_low_actors = find_actors_high();
		result.insert(result.end(), middle_low_actors.begin(), middle_low_actors.end());

		auto low_actors = find_actors_high();
		result.insert(result.end(), low_actors.begin(), low_actors.end());

		std::sort( result.begin(), result.end() );
		result.erase( std::unique( result.begin(), result.end() ), result.end() );
		return result;
	}

	vector<Actor*> find_actors_high() {
		vector<Actor*> result;

		auto process_list = ProcessLists::GetSingleton();
		for (ActorHandle actor_handle: process_list->highActorHandles)
		{
			if (!actor_handle) {
				continue;
			}
			auto actor_smartptr = actor_handle.get();
			if (!actor_smartptr) {
				continue;
			}

			Actor* actor = actor_smartptr.get();
			// auto actor = actor_handle.get().get();
			if (actor && actor->Is3DLoaded()) {
				result.push_back(actor);
			}
		}
		auto player = PlayerCharacter::GetSingleton();
		if (player && player->Is3DLoaded()) {
			result.push_back(player);
		}
		return result;
	}

	vector<Actor*> find_actors_middle_high() {
		vector<Actor*> result;

		auto process_list = ProcessLists::GetSingleton();
		for (ActorHandle actor_handle: process_list->middleHighActorHandles)
		{
			if (!actor_handle) {
				continue;
			}
			auto actor_smartptr = actor_handle.get();
			if (!actor_smartptr) {
				continue;
			}

			Actor* actor = actor_smartptr.get();
			// auto actor = actor_handle.get().get();
			if (actor && actor->Is3DLoaded()) {
				result.push_back(actor);
			}
		}
		return result;
	}

	vector<Actor*> find_actors_middle_low() {
		vector<Actor*> result;

		auto process_list = ProcessLists::GetSingleton();
		for (ActorHandle actor_handle: process_list->middleLowActorHandles)
		{
			if (!actor_handle) {
				continue;
			}
			auto actor_smartptr = actor_handle.get();
			if (!actor_smartptr) {
				continue;
			}

			Actor* actor = actor_smartptr.get();
			// auto actor = actor_handle.get().get();
			if (actor && actor->Is3DLoaded()) {
				result.push_back(actor);
			}
		}
		return result;
	}

	vector<Actor*> find_actors_low() {
		vector<Actor*> result;

		auto process_list = ProcessLists::GetSingleton();
		for (ActorHandle actor_handle: process_list->lowActorHandles)
		{
			if (!actor_handle) {
				continue;
			}
			auto actor_smartptr = actor_handle.get();
			if (!actor_smartptr) {
				continue;
			}

			Actor* actor = actor_smartptr.get();
			// auto actor = actor_handle.get().get();
			if (actor && actor->Is3DLoaded()) {
				result.push_back(actor);
			}
		}
		return result;
	}
}
