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
	class AccurateDamage : public EventListener  {
		public:
			[[nodiscard]] static AccurateDamage& GetSingleton() noexcept;

			virtual std::string DebugName() override;

			virtual void UnderFootEvent(const UnderFoot& evt) override;

			void DoAccurateCollisionLeft(Actor* actor, float damage, float radius, int random, float bbmult);
			void DoAccurateCollisionRight(Actor* actor, float damage, float radius, int random, float bbmult);
			void ApplySizeEffect(Actor* giant, Actor* tiny, float force, int random, float bbmult);
			void DoSizeDamage(Actor* giant, Actor* tiny, float totaldamage, float mult, int random, float bbmult, bool DoDamage);
	};
}
