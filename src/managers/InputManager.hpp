#pragma once
#include "data/transient.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "toml.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	enum class InputEventState {
		Idle,
		Held,
	};
	enum class TriggerMode {
		Once,
		Continuous,
	};

	class InputEventData {
		public:
			// Construct from toml::table (toml11)
			InputEventData(const toml::value& data);

			// Return time since it was first pressed
			float Duration() const;

			// Will take a key list and process if the event should fire.
			//   will return true if the events conditions are met
			bool ShouldFire(const std::unordered_set<std::uint32_t>& keys);

			// Returns true if all keys are pressed this frame
			//  Not taking into account things like duration
			bool AllKeysPressed(const std::unordered_set<std::uint32_t>& keys);

			// Returns true if ONLY the specicified keys are pressed this frame
			//   Not taking into account things like duration
			bool OnlyKeysPressed(const std::unordered_set<std::uint32_t>& keys);


			std::string GetName();

			// Checks if it has keys associaed with it
			//   This is to check to ensure that it was loaded correctly
			//   since the user can specify ANY key name and not all will
			//   produce valid results
			bool HasKeys();
		private:
			std::string name = "";
			unordered_set<std::uint32_t> keys = {};
			float minDuration = 0.0;
			double startTime = 0.0;
			// If true this event won't fire unles ONLY the keys are pressed for the entire duration
			bool exclusive = false;
			TriggerMode trigger = TriggerMode::Once;
			InputEventState state = InputEventState::Idle;
	};

	struct RegisteredInputEvent {
		std::function<void(const InputEventData&)> callback;

		RegisteredInputEvent(std::function<void(const InputEventData&)> callback);
	};

	using EventResult = RE::BSEventNotifyControl;

	class InputManager : public EventListener, public BSTEventSink<InputEvent*>
	{
		public:
			[[nodiscard]] static InputManager& GetSingleton() noexcept;

			BSEventNotifyControl ProcessEvent(InputEvent* const* a_event, BSTEventSource<InputEvent*>* a_eventSource) override;

			virtual std::string DebugName() override;
			virtual void DataReady() override;
			void Start() override;

			static void RegisterInputEvent(std::string_view name, std::function<void(const InputEventData&)> callback);

			std::unordered_map<std::string, RegisteredInputEvent> registedInputEvents;
			std::vector<InputEventData> keyTriggers;
	};
}
