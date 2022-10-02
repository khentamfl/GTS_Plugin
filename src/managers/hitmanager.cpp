#include "managers/RandomGrowth.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/hitmanager.hpp"
#include "managers/camera.hpp"
#include "magic/effects/common.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "data/time.hpp"
#include "util.hpp"

using namespace RE;
using namespace Gts;


namespace Gts {
	HitManager& HitManager::GetSingleton() noexcept {
		static HitManager instance;
		return instance;
	}

	BSEventNotifyControl HitManager::ProcessEvent(InputEvent* const* a_event, BSTEventSource<TESHitEvent*>* a_eventSource) {

		if (!a_event) {
			return BSEventNotifyControl::kContinue;
		}
        auto attacker = a_eventSource->cause;
        auto receiver = a_eventSource->target;
        auto HitId = a_eventSource->source;
        auto ProjectileID = a_eventSource->projectile;
        }

    void HitManager::Start() {
		auto hitManager = RE::BSInputDeviceManager::TESHitEvent(TESObjectREFR* a_target, TESObjectREFR* a_aggressor, FormID a_weapon, FormID a_projectile, Flag a_flags);
		hitManager->AddEventSink(&HitManager::GetSingleton());
	}    
    
    