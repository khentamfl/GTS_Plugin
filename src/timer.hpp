#pragma once

namespace Gts {
	class Timer {
		public:
			Timer(double delta);
			bool ShouldRun();
			double TimeDelta();
		private:
			double last_time = 0.0;
			double delta = 0.01666;
			double elaped_time = 0.0;
	};
}
