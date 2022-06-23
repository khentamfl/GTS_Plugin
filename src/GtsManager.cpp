#include <SKSE/SKSE.h>
#include <GtsManager.h>

using namespace Gts;
using namespace SKSE;

GtsManager& GtsManager::GetSingleton() noexcept {
	static GtsManager instance;
	return instance;
}

void GtsManager::poll() {
	auto task = SKSE::GetTaskInterface();

	// using a lambdaL
	task->AddTask([]() {
		Gts::GtsManager::GetSingleton().run();;
	});
}

void GtsManager::run() {
	auto ui = RE::UI::GetSingleton();
	if (!ui->GameIsPaused()) {
		log::info("Poll.");
	}

	auto mainmenu = ui->GetMenu<RE::MainMenu>();
	if (!mainmenu) {
		this->poll();
	} else {
		log::info("Arrived at main menu");
	}
}
