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
	log::info("A");
	auto process_list = ProcessLists::GetSingleton();
	log::info("B");
	for (ActorHandle actor_handle: process_list->highActorHandles)
	{
		auto actor = actor_handle.get();
		if (actor && actor->Is3DLoaded())
		{
			log::info("F");
			auto race = actor->GetRace();
			log::info("G");
			auto race_name = race.GetFullName();
			log::info("Actor with race %s found!", race_name);
		}
	}
}

// Poll for updates
void GtsManager::poll() {
	auto player_char = RE::PlayerCharacter::GetSingleton();
	if (!player_char) {
		return;
	}
	if (!player_char.Is3DLoaded()) {
		return;
	}

	auto ui = RE::UI::GetSingleton();
	if (!ui->GameIsPaused()) {
		log::info("Poll.");
		this->find_actors ();
	}
}
