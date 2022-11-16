#pragma once
// Module that handles Crushing
#include "events.hpp"
#include "timer.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	enum CrushState {
		Healthy,
		Crushing,
		Crushed,
	};

	struct CrushData {
		CrushData(Actor* giant, Actor* small);

		CrushState state;
		Timer delay;
		Actor* giant;
	};

	class CrushManager : public EventListener {
		public:
			[[nodiscard]] static CrushManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;

			static bool CanCrush(Actor* giant, Actor* small);
			static void Crush(Actor* giant, Actor* small);
		private:
			std::unordered_map<Actor*, CrushData> data;
	};
}
