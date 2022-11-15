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
		CrushState state;
		Timer delay;
		Actor* giant;

		CrushData(Actor* giant, Actor* small);
	};

	class CrushManager : public EventListener {
		public:
			[[nodiscard]] static AttributeManager& CrushManager() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
			virtual void Reset() override;
			virtual void ResetActor(Actor* actor) override;

			statuc bool CanCrush(Actor* giant, Actor* small);
			statuc void Crush(Actor* giant, Actor* small);
		private:
			std::unordered_map<Actor*, CrushData> data;
	};
}
