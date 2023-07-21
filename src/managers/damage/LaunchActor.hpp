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

			void ApplyLaunch(Actor* giant, float radius, float damagebonus, FootEvent kind, float power);
			void ApplyLaunch_Crawling(Actor* giant, float radius, float damagebonus, NiAVObject& node, float power);
			void Launch_Crawling(Actor* giant, float radius, float damagebonus, float power, NiAVObject& node);
			void LaunchLeft(Actor* giant, float radius, float damagebonus, float power);
			void LaunchRight(Actor* giant, float radius, float damagebonus, float power);
	};
}
