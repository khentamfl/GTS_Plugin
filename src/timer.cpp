#include "timer.hpp"

using namespace RE;

namespace {
	inline float GetGameTime() {
		return GetDurationOfApplicationRunTime();
	}
}

namespace Gts {
	Timer::Timer(float delta) : delta(delta) {
	}
	bool Timer::ShouldRun() {
		float currentTime = GetGameTime();
		if (this->last_time + this->delta <= currentTime) {
			this->elaped_time = currentTime - this->last_time;
			this->last_time = currentTime;
			return true;
		}
		return false;
	}
	float Timer::TimeDelta() {
		return this->elaped_time;
	}
}
