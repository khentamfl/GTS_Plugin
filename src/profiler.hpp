#pragma once

namespace Gts {
	class Profiler {
		private:
			// Type aliases to make accessing nested type easier
			using Clock = std::chrono::steady_clock;
			using Second = std::chrono::duration<double, std::ratio<1> >;

			std::chrono::time_point<Clock> m_beg { Clock::now() };

			double elapsed = 0.0;

		public:
			void Start();

			void Stop();

			void Reset();

			double Elapsed();
	};
}
