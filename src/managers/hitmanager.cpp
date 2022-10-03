#include "managers/hitmanager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "util.hpp"
#include "node.hpp"

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
		auto attacker_PTR = a_event->cause;
		auto atacker_ref = attacker_PTR.get();
		auto attacker = skyrim_cast<Actor*>(atacker_ref);
		if (!attacker) {
			return;
		}
		auto receiver_PTR = a_event->target;
		auto receiver_ref = attacker_PTR.get();
		auto receiver = skyrim_cast<Actor*>(receiver_ref);
		if (!receiver) {
			return;
		}
		auto runtime = Runtime::GetSingleton();
		auto sizemanager = SizeManager::GetSingleton();
		auto HitIdForm = a_event->source;
		auto HitId = TESForm::LookupByID(HitIdForm);

		auto ProjectileIDForm = a_event->projectile;
		auto ProjectileID = TESForm::LookupByID(ProjectileIDForm);
		auto player = PlayerCharacter::GetSingleton();

		bool wasPowerAttack = a_event->flags.all(TESHitEvent::Flag::kPowerAttack);
		bool wasSneakAttack = a_event->flags.all(TESHitEvent::Flag::kSneakAttack);
		bool wasBashAttack = a_event->flags.all(TESHitEvent::Flag::kBashAttack);
		bool wasHitBlocked = a_event->flags.all(TESHitEvent::Flag::kHitBlocked);

		// Do something
		ConsoleLog::GetSingleton()->Print("Hit Detection is fine.");

		if (receiver == player && receiver->HasPerk(runtime.GrowthOnHitPerk) && HitId->GetName() != "Stagger" && sizemanager.GetHitGrowth(receiver) < 0.01) {
			ConsoleLog::GetSingleton()->Print("First condition passed");
			if(wasHitBlocked == false && attacker->IsPlayerTeammate() == false && attacker != player) {
				ConsoleLog::GetSingleton()->Print("Hit Initialized.");
				float ReceiverScale = get_visual_scale(receiver);
				float DealerScale = get_visual_scale(attacker);
				float HealthMult = GetMaxAV(receiver, ActorValue::kHealth) / receiver->GetActorValue(ActorValue::kHealth);
				float SizeDifference = ReceiverScale/DealerScale;
				float LaughChance = rand() % 12;
				auto GrowthSound = runtime.growthSound;
				PlaySound(GrowthSound, receiver, ReceiverScale/10, 0.0);
				sizemanager.SetHitGrowth(receiver, 1.0);
				sizemanager.SetGrowthTime(receiver, HealthMult);
				if (SizeDifference >= 4.0 && LaughChance >= 12.0) {
					auto LaughSound = Runtime::GetSingleton().LaughSound;
					PlaySound(LaughSound, receiver, 1.0, 0.0); //FearCast()
				}
			}
		}
	}

	void HitManager::Update() {
			auto actor = PlayerCharacter::GetSingleton() ;
			auto Runtime = Runtime::GetSingleton();
			auto sizemanager = SizeManager::GetSingleton();
			
			if (sizemanager.GetHitGrowth(actor) > 0.0) {
				ConsoleLog::GetSingleton()->Print("OnUpdate Works.");
				float HealthMult = GetMaxAV(actor, ActorValue::kHealth) / actor->GetActorValue(ActorValue::kHealth);
				float GrowthValue = HealthMult/9700;
				auto& Persist = Persistent::GetSingleton();
				auto actor_data = Persist.GetData(actor);
				float delta_time = Time::WorldTimeDelta();
				actor_data->half_life = 1.0 - HealthMult;
				if (actor->HasMagicEffect(Runtime.SmallMassiveThreat)) {
					GrowthValue *= 0.50;
				}
				if (sizemanager.GetGrowthTime(actor) > 0.01) {
					ConsoleLog::GetSingleton()->Print("Growth Started.");
					GrowthTremorManager::GetSingleton().CallRumble(actor, actor, actor_data->half_life * 2);
					mod_target_scale(actor, GrowthValue * (get_visual_scale(actor) * 0.25 + 0.75));
				} else if (sizemanager.GetGrowthTime(actor) < 0.01) {
					actor_data->half_life = 1.0;
					sizemanager.SetHitGrowth(actor, 0.0);
					sizemanager.SetGrowthTime(actor, 0.0);
					ConsoleLog::GetSingleton()->Print("Growth Ended.");
			}
		}
	}
}

