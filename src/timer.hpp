#pragma once

namespace Gts {
	class Timer {
		public:
			Timer(float delta);
			bool ShouldRun();
			float TimeDelta();
		private:
			float last_time = 0.0;
			float delta = 0.01666;
			float elaped_time = 0.0;
	};
}
