#include "profiler.hpp"
#include <chrono>

namespace Gts {
	void Profiler::Start()
	{
		m_beg = Clock::now();
	}

	double Profiler::Stop()
	{
		this->elapsed += std::chrono::duration_cast<Second>(Clock::now() - m_beg).count();
	}

	double Profiler::Elapsed() {
		return this->elapsed;
	}
}
