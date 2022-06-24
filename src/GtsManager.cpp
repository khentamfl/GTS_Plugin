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

// Poll for updates
void GtsManager::poll() {
	auto ui = RE::UI::GetSingleton();
	if (!ui->GameIsPaused()) {
		log::info("Poll.");
	}
}
