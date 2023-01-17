#pragma once
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "events.hpp"
#include "node.hpp"

using namespace std;
using namespace RE;
// Module for accurate size-related damage

namespace Gts {
	/**
	 * The class which tracks gts size effects.
	 */
	class accuratedamage : public EventListener  {
		public:
			[[nodiscard]] static accuratedamage& GetSingleton() noexcept;

            virtual void UnderFootEvent(const UnderFoot& evt) override;

			virtual std::string DebugName() override;
			virtual void Update() override;

			void DoAccurateCollision(Actor* actor);
	};
}
