#include "util.hpp"
#include <ehdata.h>
#include <rttidata.h>
#include "node.hpp"

using namespace Gts;

namespace {
	// Spring code from https://theorangeduck.com/page/spring-roll-call
	float halflife_to_damping(float halflife, float eps = 1e-5f)
	{
		return (4.0f * 0.69314718056f) / (halflife + eps);
	}

	float damping_to_halflife(float damping, float eps = 1e-5f)
	{
		return (4.0f * 0.69314718056f) / (damping + eps);
	}
	float fast_negexp(float x)
	{
		return 1.0f / (1.0f + x + 0.48f*x*x + 0.235f*x*x*x);
	}
}
namespace Gts {
	std::string GetRawName(const void* obj_c) {
		// Get the meta entry in vftable
		void* obj = const_cast<void*>(obj_c);
		_RTTICompleteObjectLocator* col = reinterpret_cast<_RTTICompleteObjectLocator***>(obj)[0][-1];

		// Calculate image base by subtracting the RTTICompleteObjectLocator's pSelf offset from RTTICompleteObjectLocator's pointer
		uintptr_t imageBase = reinterpret_cast<uintptr_t>(col) - col->pSelf;

		// Get the type descriptor by adding TypeDescriptor's offset to the image base
		TypeDescriptor* tDesc = reinterpret_cast<TypeDescriptor*>(imageBase + col->pTypeDescriptor);

		// At the end, we can get the type's mangled name
		const char* colName = tDesc->name;
		return colName;
	}

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

	float unit_to_meter(const float& unit) {
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

	float meter_to_unit(const float& meter) {
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

	NiPoint3 unit_to_meter(const NiPoint3& unit) {
		return unit / 70.0;
	}
	NiPoint3 meter_to_unit(const NiPoint3& meter) {
		return meter * 70.0;
	}

	void critically_damped(
		float& x,
		float& v,
		float x_goal,
		float halflife,
		float dt)
	{
		float y = halflife_to_damping(halflife) / 2.0f;
		float j0 = x - x_goal;
		float j1 = v + j0*y;
		float eydt = fast_negexp(y*dt);

		x = eydt*(j0 + j1*dt) + x_goal;
		v = eydt*(v - j1*y*dt);
	}

	std::atomic_bool& get_main_thread() {
		static std::atomic_bool main_thread(false);
		return main_thread;
	}

	bool IsJumping(Actor* actor) {
		if (!actor) {
			return false;
		}
		if (!actor->Is3DLoaded()) {
			return false;
		}
		bool result = false;
		actor->GetGraphVariableBool("bInJumpState", result);
		return result;
	}

	bool IsFirstPerson() {
		auto playercamera = PlayerCamera::GetSingleton();
		if (!playercamera) {
			return false;
		}
		if (playercamera->currentState == playercamera->cameraStates[CameraState::kFirstPerson]) {
			return true;
		}
		return false;
	}

	bool IsFreeCamera() {
		auto playercamera = PlayerCamera::GetSingleton();
		if (!playercamera) {
			return false;
		}
		if (playercamera->currentState == playercamera->cameraStates[CameraState::kFree]) {
			return true;
		}
		return false;
	}

}
