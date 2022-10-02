#include "managers/hitmanager.hpp"

using namespace RE;
using namespace Gts;


namespace Gts {
	HitManager& HitManager::GetSingleton() noexcept {
		static HitManager instance;
		return instance;
	}

	void HitManager::HitEvent(const TESHitEvent* a_event) {

		if (!a_event) {
			return;
		}
		auto attacker = a_event->cause;
		if (!attacker) {
			return;
		}
		auto receiver = a_event->target;
		if (!receiver) {
			return;
		}
		auto HitId = a_event->source;
		auto ProjectileID = a_event->projectile;

		bool wasPowerAttack = a_event->flags.all(TESHitEvent::Flag::kPowerAttack);
		bool wasSneakAttack = a_event->flags.all(TESHitEvent::Flag::kSneakAttack);
		bool wasBashAttack = a_event->flags.all(TESHitEvent::Flag::kBashAttack);
		bool wasHitBlocked = a_event->flags.all(TESHitEvent::Flag::kHitBlocked);

		// Do something

	}
}
