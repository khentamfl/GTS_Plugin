#include <SKSE/SKSE.h>
#include <GtsManager.h>
#include <chrono>
#include <thread>

using namespace Gts;
using namespace SKSE;
using namespace RE;
using namespace std::chrono_literals;

GtsManager& GtsManager::GetSingleton() noexcept {
	static GtsManager instance;
	return instance;
}

void GtsManager::find_actors() {
	auto process_list = ProcessLists::GetSingleton();
	for (ActorHandle actor_handle: process_list->highActorHandles)
	{
		auto actor = actor_handle.get();
		if (actor && actor->Is3DLoaded())
		{
			auto base_actor = actor->GetActorBase();
			auto actor_name = base_actor->GetFullName();

			auto race = actor->GetRace();
			auto race_name = race->GetFullName();
			log::info("Actor {} with race {} found!", actor_name, race_name);
		}
	}
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
		this->find_actors ();
	}
}
