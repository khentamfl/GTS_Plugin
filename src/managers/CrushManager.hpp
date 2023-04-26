#pragma once
// Module that handles footsteps
#include "events.hpp"
#include "timer.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	enum class CrushState {
		Healthy,
		Crushing,
		Crushed
	};

	class CrushData {
		public:
			CrushData(Actor* giant, Actor* tiny);

			CrushState state;
			Timer delay;
			Actor* giant;
	};

	class CrushManager : public EventListener {
		public:
			[[nodiscard]] static CrushManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void PapyrusUpdate() override; // PapyrusUpdate
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;

			static bool CanCrush(Actor* giant, Actor* tiny);
			static bool AlreadyCrushed(Actor* actor);
			static void Crush(Actor* giant, Actor* tiny);
		private:
			std::unordered_map<Actor*, CrushData> data;
	};
}
