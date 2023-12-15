// A task runs every frame
//  until it returns false


#include "events.hpp"
#include "data/time.hpp"

namespace Gts {

	enum class UpdateKind {
		Main,
		Camera,
		Havok,
	};

	class BaseTask {
		public:
			virtual bool Update() = 0;
			UpdateKind UpdateOn() {
				return this->updateOnKind;
			}

			void SetUpdateOn(UpdateKind updateOn) {
				this->updateOnKind = updateOn;
			}

		protected:
			UpdateKind updateOnKind = UpdateKind::Main;
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

			}

			virtual bool Update() override {
				TaskUpdate update;
				double currentTime = Time::WorldTimeElapsed();
				if (this->initRun) {
					update = TaskUpdate {
						.runtime = currentTime - this->startTime,
						.delta = currentTime - this->lastRunTime,
					};
				} else {
					update = TaskUpdate {
						.runtime = 0.0,
						.delta = 0.0,
					};
					this->initRun = true;
				}
				this->lastRunTime = currentTime;
				return this->tasking(update);
			}

		private:
			bool initRun = false;
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
			}

			virtual bool Update() override {
				double currentTime = Time::WorldTimeElapsed();
				double currentRuntime = currentTime - this->startTime;
				double currentProgress = std::clamp(currentRuntime / this->duration, 0.0, 1.0);
				
				TaskForUpdate update;
				if (this->initRun) {
					update	= TaskForUpdate {
							.runtime = currentRuntime,
							.delta = currentTime - this->lastRunTime,
							.progress = currentProgress,
							.progressDelta = currentProgress - this->lastProgress,
						};
					} else {
						update = TaskForUpdate {
							.runtime = 0.0,
							.delta = 0.0,
							.progress = 0.0,
							.progressDelta = 0.0,
						};
						this->initRun = true;
					}
				this->lastRunTime = currentTime;
				this->lastProgress = currentProgress;
				if (!this->tasking(update)) {
					return false;
				} else {
					return currentRuntime <= this->duration;
				}
			}
		private:
			bool initRun = false;
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
					if (task->UpdateOn() == UpdateKind::Main) {
						if (!task->Update()) {
							toRemove.push_back(name);
						}
					}
				}
				log::info("running task {}", name);
				for (auto task: toRemove) {
					this->taskings.erase(task);
				}

			}

			// Update in camera update locations too....
			virtual void CameraUpdate() override {
				std::vector<std::string> toRemove = {};
				for (auto& [name, task]: this->taskings) {
					if (task->UpdateOn() == UpdateKind::Camera) {
						if (!task->Update()) {
							toRemove.push_back(name);
						}
					}
				}

				for (auto task: toRemove) {
					this->taskings.erase(task);
				}

			}

			virtual void HavokUpdate() override {
				std::vector<std::string> toRemove = {};
				for (auto& [name, task]: this->taskings) {
					if (task->UpdateOn() == UpdateKind::Havok) {
						if (!task->Update()) {
							toRemove.push_back(name);
						}
					}
				}

				for (auto task: toRemove) {
					this->taskings.erase(task);
				}

			}

			static void ChangeUpdate(std::string_view name, UpdateKind updateOn) {
				auto& me = TaskManager::GetSingleton();
				try {
					me.taskings.at(std::string(name))->SetUpdateOn(updateOn);
				} catch (const std::out_of_range& oor) {
					// nothing
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
