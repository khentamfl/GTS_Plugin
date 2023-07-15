#include "managers/damage/AccurateDamage.hpp"
#include "managers/damage/LaunchActor.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/runtime.hpp"
#include "UI/DebugAPI.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "node.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {

	const float LAUNCH_DAMAGE = 2.4f;
	const float LAUNCH_KNOCKBACK = 0.02f;
	const float UNDERFOOT_POWER = 0.05;
}

namespace Gts {

	LaunchActor& LaunchActor::GetSingleton() noexcept {
		static LaunchActor instance;
		return instance;
	}

	std::string LaunchActor::DebugName() {
		return "LaunchActor";
	}

	void LaunchActor::LaunchDecide(Actor* giant, Actor* tiny, float force, float damagebonus) {
		if (!Runtime::HasPerkTeam(giant, "LaunchPerk")) {
			return;
		} if (IsBeingHeld(tiny)) {
			return;
		} if (!tiny) {
			return;
		} 
		auto& accuratedamage = AccurateDamage::GetSingleton();
		float giantSize = get_visual_scale(giant);
		float SMT = 1.0;
		float threshold = 6.0;
		if (HasSMT(giant)) {
			giantSize += 1.0;
			threshold = 0.8;
			force += 0.20;
		}
		float tinySize = get_visual_scale(tiny);
		if (IsDragon(tiny)) {
			tinySize *= 2.6;
		}
		float sizeRatio = giantSize/tinySize;

		float knockBack = LAUNCH_KNOCKBACK * giantSize * force;

		auto& sizemanager = SizeManager::GetSingleton();
		if (force >= UNDERFOOT_POWER && sizeRatio >= 6.0 / GetMovementModifier(giant)) { // If close enough
		log::info("Launchig Actor: {}", tiny->GetDisplayFullName());
			if (!sizemanager.IsLaunching(tiny)) {
				sizemanager.GetSingleton().GetLaunchData(tiny).lastLaunchTime = Time::WorldTimeElapsed();
				if (Runtime::HasPerkTeam(giant, "LaunchDamage")) {
					float damage = LAUNCH_DAMAGE * giantSize * force * damagebonus;
					DamageAV(tiny, ActorValue::kHealth, damage * 0.25);
				}
				PushActorAway(giant, tiny, force, 2600);
			}
		} 
	}
}