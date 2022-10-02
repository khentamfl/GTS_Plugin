#include "managers/hitmanager.hpp"

using namespace RE;
using namespace Gts;


namespace Gts {
	HitManager& HitManager::GetSingleton() noexcept {
		static HitManager instance;
		return instance;
	}

	void HitEvent(const TESHitEvent* a_event) {

		if (!a_event) {
			return;
		}
		auto attacker = a_eventSource->cause;
		if (!attacker) {
			return;
		}
		auto receiver = a_eventSource->target;
		if (!receiver) {
			return;
		}
		auto HitId = a_eventSource->source;
		auto ProjectileID = a_eventSource->projectile;

		bool wasPowerAttack = a_eventSource->flags.all(TESHitEvent::Flag::kPowerAttack);
		bool wasSneakAttack = a_eventSource->flags.all(TESHitEvent::Flag::kSneakAttack);
		bool wasBashAttack = a_eventSource->flags.all(TESHitEvent::Flag::kBashAttack);
		bool wasHitBlocked = a_eventSource->flags.all(TESHitEvent::Flag::kHitBlocked);

		// Do something

	}
}
