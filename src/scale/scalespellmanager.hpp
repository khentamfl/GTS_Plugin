#pragma once
// Module that handles Spell Distribution to everyone
#include "scale/events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {

	class ScaleSpellManager {
		public:
			[[nodiscard]] static ScaleSpellManager& GetSingleton() noexcept;
			void CheckSize(Actor* actor);
			void ApplySpellBonus(Actor* actor, float scale);
	};
}
