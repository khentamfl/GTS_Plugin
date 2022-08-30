#include "managers/RandomGrowth.hpp"
#include "managers/GtsManager.hpp"
#include "util.hpp"
#include "scale/scale.hpp"

using namespace RE;
using namespace Gts;

namespace {
	bool ShouldGrow() {
		if (GtsManager::GetSingleton().GetFrameNum() % 120) {
		int random = rand() % 200 + 1;
		int decide_chance = 1;
		}
		return (random <= decide_chance);
	}
}

namespace Gts {
	RandomGrowth& RandomGrowth::GetSingleton() noexcept {
		static RandomGrowth instance;
		return instance;
	}

	void RandomGrowth::Update() {
		auto player = PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}
		if (!player->Is3DLoaded()) {
			return;
		}

		switch (this->state) {
			case State::Idle: {
				if (ShouldGrow()) {
					// Start growing state
					this->state = State::Working;
					this->growth_time = 0.0;
					// Play sound
				}
			}
			case State::Working: {
				// Do the growing
				float delta_time = *g_delta_time;
				float base_power = 0.01;// Put in actual power please
				mod_target_scale(player, base_power * delta_time); // Use delta_time so that the growth will be the same regardless of fps

				this->growth_time += *g_delta_time;
				if (this->growth_time >= 2.6) { // Time in seconds" 160tick / 60 ticks per secong ~= 2.6s
					// End growing
					this->state = State::Idle;
				}
			}
		}
	}
}
