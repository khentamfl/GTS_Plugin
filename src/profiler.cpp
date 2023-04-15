#include "profiler.hpp"
#include <chrono>

namespace Gts {
	Profiler::Profiler(std::string_view name) : name(std::string(name)) {

	}

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

	std::string Profiler::GetName() {
		return this->name;
	}

  ProfilerHandle::ProfilerHandle(std::string_view name) : name(std::string(name)) {
    Profilers::Start(name);
  }

  ProfilerHandle::~ProfilerHandle() {
    Profilers::Stop(this->name);
  }

  ProfilerHandle Profilers::Profile(std::string_view name) {
    return ProfilerHandle(name);
  }

	void Profilers::Start(std::string_view name) {
    if (Config::GetSingleton().GetDebug().ShouldProfile()) {
      auto key = std::string(name);
      this->profilers.try_emplace(key, name);
      this->profilers.at(key).Start();
    }
  }

  void Profilers::Stop(std::string_view name) {
    if (Config::GetSingleton().GetDebug().ShouldProfile()) {
      auto key = std::string(name);
      this->profilers.try_emplace(key, name);
      this->profilers.at(key).Stop();
    }
  }

  void Profilers::Report() {
    std::string report = "Reporting Profilers:";
		report += std::format("\n|{:20}|", "Name");
		report += std::format("{:15s}|", "Seconds");
		report += std::format("{:15s}|", "% OurCode");
		report += std::format("{:15s}|", "s per frame");
		report += std::format("{:15s}|", "% of frame");
		report += "\n------------------------------------------------------------------------------------------------";

		static std::uint64_t last_report_frame = 0;
		static double last_report_time = 0.0;
		std::uint64_t current_report_frame = Time::WorldTimeElapsed();
		double current_report_time = Time::WorldTimeElapsed();
		double total_time = current_report_time - last_report_time;

		double total = 0.0;
		for (auto listener: Profilers::GetSingleton().listeners) {
			total += listener->profiler.Elapsed();
		}
		for (auto listener: Profilers::GetSingleton().listeners) {
			double elapsed = listener->profiler.Elapsed();
			double spf = elapsed / (current_report_frame - last_report_frame);
			double time_percent = elapsed/total_time*100;
			report += std::format("\n {:20}:{:15.3f}|{:14.1f}%|{:15.3f}|{:14.3f}%", listener->DebugName(), elapsed, elapsed*100.0/total, spf, time_percent);
			listener->profiler.Reset();
		}
		log::info("{}", report);

		last_report_frame = current_report_frame;
		last_report_time = current_report_time;
  }

	Profilers& Profilers::GetSingleton() {
    static Profilers instance;
    return instance;
  }
}
