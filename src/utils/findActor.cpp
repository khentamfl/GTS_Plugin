#include "utils/findActor.hpp"
#include "utils/actorUtils.hpp"

using namespace std;
using namespace RE;
using namespace SKSE;

namespace {
    class FindActorData {
        public:
        /// Actors that have been done recently;
        unordered_set<FormID> previousActors;
    };
}

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
	
	vector<Actor*> FindSomeActors(std::string_view tag, uint32_t howMany) {
	    static unordered_map<string, FindActorData> allData;
	    allData.try_emplace(string(tag));
	    auto& data = allData.at(string(tag));
	    
	    log::info("Looking for actor for {} up to a count of {}", tag, howMany);
	    unordered_set<Actor*> finalActors;
	    unordered_set<Actor*> notAddedAcrors;
	    auto prevCount = data.previousActors.size();
	    for (auto actor: find_actor()) {
	        // Player or teammate are always updated
	        if (actor->formId == 0x14 || IsTeammate(actor)) {
	            finalActors.push(actor);
	            log::info(". Adding: {}", actor->GetDisplayNameFull());
	        } else if (data.previousActors.count(actor->formId) == 0) {
	        // Other actors are only added if they are not in the previous actor list
	            if (addedCount < howMany) {
	                log::info(". Adding: {}", actor->GetDisplayNameFull());
            	    finalActors.push(actor);
            	    data.previousActors.insert(actor->formId);
            	    addedCount += 1;
            	}
        	} else {
        	    notAddedAcrors.push(actor);
        	}
	    }
	    // Reached the end of all actor
	    if (addedCount < howMany) {
	        // We need more. Reset the used list and add from
	        // those not added set
    	    data.previousActors.clear();
    	    for (auto actor: notAddedAcrors) {
        	    if (addedCount < howMany) {
        	        finalActors.push(actor);
            	    data.previousActors.insert(actor->formId);
            	    addedCount += 1;
        	    } else {
        	        break;
        	    }
    	    }
	    }
    	return finalActors;
	}
	    
}
