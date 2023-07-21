#pragma once
#include <vector>
#include <atomic>
#include <unordered_map>

#include <RE/Skyrim.h>

#include "events.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
// Module for accurate size-related damage

namespace Gts {
	class LaunchActor : public EventListener  {
		public:
			[[nodiscard]] static LaunchActor& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			void LaunchCrawling(Actor* giant, float radius, float power, NiAVObject* node, float damagebonus);
			void ApplyLaunch(Actor* giant, float radius, float damagebonus, FootEvent kind, float power);
			void LaunchLeft(Actor* giant, float radius, float damagebonus, float power);
			void LaunchRight(Actor* giant, float radius, float damagebonus, float power);
	};
}
