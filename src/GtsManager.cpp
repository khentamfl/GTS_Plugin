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

// Poll will call it on the main thread
void GtsManager::poll() {
	auto task = SKSE::GetTaskInterface();

	// using a lambdaL
	task->AddTask([]() {
		auto& gts = Gts::GtsManager::GetSingleton();
		gts.run();
		gts.queued.store(false);
	});
}

// Run happens on the main thread, here we can safely do
// mutliple NiNode effects
void GtsManager::run() {
	auto ui = RE::UI::GetSingleton();
	if (!ui->GameIsPaused()) {
		log::info("Poll.");
	}

	auto mainmenu = ui->GetMenu<RE::MainMenu>();
	if (mainmenu) {
		this->abort();
	}
}

// Loop is a blocking call that will poll every x seconds
void GtsManager::loop() {
	this->aborted.store(false);

	static std::atomic_bool running;
	static std::latch latch(1);
	if (!running.exchange(true)) {
		this->queued.store(false);
		while (!this->aborted.load()) {
			if (!this->queued.exchange(true)) {
				this->poll();
			}
			std::this_thread::sleep_for(100ms);
		}
		running.store(false);
		latch.count_down();
	}
	latch.wait();
}

void GtsManager::abort() {
	this->aborted.store(true);
}
