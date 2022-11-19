#include "profiler.hpp"
#include <chrono>

namespace Gts {
	void Profiler::Start()
	{
		m_beg = Clock::now();
	}

	void Profiler::Stop()
	{
		this->elapsed += std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
	}

	void Profiler::Reset()
	{
		this->elapsed = 0.0;
	}

	double Profiler::Elapsed() {
		return this->elapsed;
	}
}
