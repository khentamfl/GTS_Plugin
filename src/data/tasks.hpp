// A task runs every frame
//  until it returns false


#include "events.hpp"
#include "data/time.hpp"

namespace Gts {

  class BaseTask {
  public:
    virtual bool Update() = 0;
  };

  // A `Task` runs until it returns false
  struct TaskUpdate {
    // Total runtime in seconds`
    double runtime;
    // Time delta since last runtime
    double delta;
  };

  class Task: public BaseTask {
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
  class TaskFor: public BaseTask {
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
      double currentRuntime = Time::WorldTimeElapsed() - this->startTime;
      double currentProgress = std::clamp(currentRuntime / this->duration, 0.0, 1.0);
      auto update = TaskForUpdate {
        .runtime = currentRuntime,
        .delta = currentRuntime - this->lastRunTime,
        .progress = currentProgress,
        .progressDelta = currentProgress - this->lastProgress,
      };
      this->lastRunTime = currentRuntime;
      this->lastProgress = currentProgress;
      if (! this->tasking(update))  {
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
      for (auto& task: this->taskings) {
        if (!task->Update()) {
          this->taskings.erase(task);
        }
      }
    }

    static void Run(std::function<bool(const TaskUpdate&)> tasking) {
      auto& me = TaskManager::GetSingleton();
      me.taskings.insert(
        new Task(tasking)
      );
    }

    static void RunFor(float duration, std::function<bool(const TaskForUpdate&)> tasking) {
      auto& me = TaskManager::GetSingleton();
      me.taskings.insert(
        new TaskFor(duration, tasking)
      );
    }

    std::unordered_set<BaseTask*> taskings;
  };
}
