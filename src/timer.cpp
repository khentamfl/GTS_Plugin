#include "timer.hpp"
#include "util.hpp"
#include "data/time.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace {
	inline double GetGameTime() {
		return Time::WorldTimeElapsed();
	}
}

namespace Gts {
	Timer::Timer(double delta) : delta(delta) {
	}
	bool Timer::ShouldRun() {
		double currentTime = GetGameTime();
		log::info("last_time: {}, elapsed_time: {}, currentTime: {}", this->last_time, this->elaped_time, currentTime);
		if (this->last_time + this->delta <= currentTime) {
			this->elaped_time = currentTime - this->last_time;
			this->last_time = currentTime;
			return true;
		}
		return false;
	}
	double Timer::TimeDelta() {
		return this->elaped_time;
	}
}
