// A task runs every frame
//  until it returns false


#include "events.hpp"
#include "data/time.hpp"

namespace Gts {

	class BaseTask {
		public:
			virtual bool Update() = 0;
	};

  // A `Task` runs once in the next frame
	struct OneshotUpdate {
		// Total time since creation before run
		double timeToLive;
	};

  class Oneshot : public BaseTask {
		public:
			Oneshot(std::function<void(const OneshotUpdate&)> tasking) : tasking(tasking), creationTime(Time::WorldTimeElapsed()) {
			}

			virtual bool Update() override {
				double currentTime = Time::WorldTimeElapsed();
				auto update = OneshotUpdate {
					.timeToLive = currentTime - this->creationTime,
				};
				this->tasking(update);
        return false;
			}

		private:
			double creationTime = 0.0;
			std::function<void(const OneshotUpdate&)> tasking;
	};

	// A `Task` runs until it returns false
	struct TaskUpdate {
		// Total runtime in seconds`
		double runtime;
		// Time delta since last runtime
		double delta;
	};

	class Task : public BaseTask {
		public:
			Task(std::function<bool(const TaskUpdate&)> tasking) : tasking(tasking), lastRunTime(Time::WorldTimeElapsed()), startTime(Time::WorldTimeElapsed()) {

				auto update = TaskUpdate {
					.runtime = 0.0,
					.delta = 0.0,
				};
				tasking(update);
			}

			virtual bool Update() override {
				double currentTime = Time::WorldTimeElapsed();
				auto update = TaskUpdate {
					.runtime = currentTime - this->startTime,
					.delta = currentTime - this->lastRunTime,
				};
				this->lastRunTime = currentTime;
				return this->tasking(update);
			}

		private:
			double startTime = 0.0;
			double lastRunTime = 0.0;
			std::function<bool(const TaskUpdate&)> tasking;
	};

	struct TaskForUpdate {
		// Total runtime in seconds`
		double runtime;
		// Time delta since last runtime
		double delta;
		// How close to completion on a scale of 0.0...1.0
		double progress;
		// How much progress has been gained since last time
		double progressDelta;
	};
	// A `TaskFor` runs until it returns false OR the duration has elapsed
	class TaskFor : public BaseTask {
		public:
			TaskFor(double duration, std::function<bool(const TaskForUpdate&)> tasking) : tasking(tasking), duration(duration), lastRunTime(Time::WorldTimeElapsed()), startTime(Time::WorldTimeElapsed()) {
				auto update = TaskForUpdate {
					.runtime = 0.0,
					.delta = 0.0,
					.progress = 0.0,
					.progressDelta = 0.0,
				};
				tasking(update);
			}

			virtual bool Update() override {
				double currentTime = Time::WorldTimeElapsed();
				double currentRuntime = currentTime - this->startTime;
				double currentProgress = std::clamp(currentRuntime / this->duration, 0.0, 1.0);
				auto update = TaskForUpdate {
					.runtime = currentRuntime,
					.delta = currentTime - this->lastRunTime,
					.progress = currentProgress,
					.progressDelta = currentProgress - this->lastProgress,
				};
				this->lastRunTime = currentTime;
				this->lastProgress = currentProgress;
				if (!this->tasking(update)) {
					return false;
				} else {
					return currentRuntime <= this->duration;
				}
			}
		private:
			double startTime = 0.0;
			double lastRunTime = 0.0;
			double lastProgress = 0.0;
			std::function<bool(const TaskForUpdate&)> tasking;
			double duration;
	};


	class TaskManager : public EventListener {
		public:
			virtual std::string DebugName() {
				return "TaskManager";
			}

			static TaskManager& GetSingleton() {
				static TaskManager instance;
				return instance;
			}

			virtual void Update() override {
				std::vector<std::string> toRemove = {};
				for (auto& [name, task]: this->taskings) {
					if (!task->Update()) {
						toRemove.push_back(name);
					}
				}

				for (auto task: toRemove) {
					this->taskings.erase(task);
				}

			}

			static void Cancel(std::string_view name) {
				auto& me = TaskManager::GetSingleton();
				me.taskings.erase(std::string(name));
			}

			static void Run(std::function<bool(const TaskUpdate&)> tasking) {
				auto& me = TaskManager::GetSingleton();
				auto task = new Task(tasking);
				std::string name = std::format("UNNAMED_{}", *reinterpret_cast<std::uintptr_t*>(task));
				me.taskings.try_emplace(
					name,
					task
				);
			}

			static void Run(std::string_view name, std::function<bool(const TaskUpdate&)> tasking) {
				auto& me = TaskManager::GetSingleton();
				me.taskings.try_emplace(
					std::string(name),
					new Task(tasking)
					);
			}

			static void RunFor(float duration, std::function<bool(const TaskForUpdate&)> tasking) {
				auto& me = TaskManager::GetSingleton();
				auto task = new TaskFor(duration, tasking);
				std::string name = std::format("UNNAMED_{}", *reinterpret_cast<std::uintptr_t*>(task));
				me.taskings.try_emplace(
					name,
					task
				);
			}

			static void RunFor(std::string_view name, float duration, std::function<bool(const TaskForUpdate&)> tasking) {
				auto& me = TaskManager::GetSingleton();
				me.taskings.try_emplace(
					std::string(name),
					new TaskFor(duration, tasking)
					);
			}

      static void RunOnce(std::function<void(const OneshotUpdate&)> tasking) {
        auto& me = TaskManager::GetSingleton();
				auto task = new Oneshot(tasking);
				std::string name = std::format("UNNAMED_{}", *reinterpret_cast<std::uintptr_t*>(task));
				me.taskings.try_emplace(
					name,
					task
				);
			}

      static void RunOnce(std::string_view name, std::function<void(const OneshotUpdate&)> tasking) {
				auto& me = TaskManager::GetSingleton();
				me.taskings.try_emplace(
					std::string(name),
					new Oneshot(tasking)
				);
			}

			std::unordered_map<std::string, BaseTask*> taskings;
	};
}
