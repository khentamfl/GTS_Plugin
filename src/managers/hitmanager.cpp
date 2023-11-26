#include "managers/hitmanager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"
#include "managers/Rumble.hpp"

using namespace RE;
using namespace Gts;
namespace {
	float GetPushPower(float sizeRatio) {
		// https://www.desmos.com/calculator/wh0vwgljfl
		SoftPotential push {
			.k = 1.42,
			.n = 0.78,
			.s = 0.50,
			.a = 0.0,
		};
		float power = soft_power(sizeRatio, push);
		return power;
	}
}

namespace Gts {
	HitManager& HitManager::GetSingleton() noexcept {
		static HitManager instance;
		return instance;
	}

	std::string HitManager::DebugName() {
		return "HitManager";
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
		auto receiver_ref = receiver_PTR.get();
		auto receiver = skyrim_cast<Actor*>(receiver_ref);
		if (!receiver) {
			return;
		}

		auto& sizemanager = SizeManager::GetSingleton();
		auto& Persist = Persistent::GetSingleton();

		auto HitIdForm = a_event->source;
		auto HitId = TESForm::LookupByID(HitIdForm);

		std::string hitName = HitId->GetName();

		if (hitName == "Stagger" || hitName == "SizeEffect" || hitName == "SprintingSizeEffect" || hitName == "GtsTastyFoe") {
			return;
		}
		if (attacker->formID == 0x14) {
			log::info("Hit Name: {}, HitForm: {}", HitId->GetName(), HitIdForm);
		}
		auto ProjectileIDForm = a_event->projectile;
		auto ProjectileID = TESForm::LookupByID(ProjectileIDForm);
		auto player = PlayerCharacter::GetSingleton();

		bool wasPowerAttack = a_event->flags.all(TESHitEvent::Flag::kPowerAttack);
		bool wasSneakAttack = a_event->flags.all(TESHitEvent::Flag::kSneakAttack);
		bool wasBashAttack = a_event->flags.all(TESHitEvent::Flag::kBashAttack);
		bool wasHitBlocked = a_event->flags.all(TESHitEvent::Flag::kHitBlocked);
		static Timer timer = Timer(0.25);

		float attackerscale = get_visual_scale(attacker);
		float receiverscale = get_visual_scale(receiver);
		if (IsDragon(receiver) || IsGiant(receiver) || IsMammoth(receiver)) {
			receiverscale *= 2.5;
		}

		float size_difference = attackerscale/receiverscale;

		if (HasSMT(player)) {
			size_difference += 3.0;
		}

		// Apply it
		float pushpower = GetPushPower(size_difference);
		if (attacker->formID == 0x14 && size_difference >= 4.0) {
			FormType formType = HitId->GetFormType();
			if (formType != FormType::Weapon) {
				return;
			}
			if (wasPowerAttack || hitName.find("Bow") != std::string::npos) {
				size_difference *= 2.0;
			}
			if (hitName.find("Bow") == std::string::npos) {
				shake_camera(attacker, size_difference * 0.20, 0.35);
			}
			//PushActorAway(attacker, receiver, pushpower);
			PushForward(attacker, receiver, pushpower * 100);
			log::info("Size difference is met, pushing actor away");
		}
	}

	void HitManager::Update() {
		return;
	}
	void HitManager::Overkill(Actor* receiver, Actor* attacker) {
		if (!receiver->IsDead()) {
			KillActor(attacker, receiver);
		}

		ActorHandle giantHandle = attacker->CreateRefHandle();
		ActorHandle tinyHandle = receiver->CreateRefHandle();

		std::string taskname = std::format("Overkill {}", receiver->formID);

		TaskManager::RunOnce(taskname, [=](auto& update){
			if (!tinyHandle) {
				return;
			}
			if (!giantHandle) {
				return;
			}
			auto giant = giantHandle.get().get();
			auto tiny = tinyHandle.get().get();
			float scale = get_visual_scale(tiny);
			TransferInventory(tiny, giant, scale, false, true, DamageSource::Overkill);
			// ^ transferInventory>TransferInventoryToDropBox also plays crush audio on loot pile
			// Was done like that because Audio disappears on actors
		});

		Runtime::CreateExplosion(receiver, get_visual_scale(receiver), "BloodExplosion");
		std::random_device rd;
		std::mt19937 gen(rd());
		std::uniform_real_distribution<float> dis(-0.2, 0.2);

		if (!IsLiving(receiver)) {
			SpawnDustParticle(receiver, attacker, "NPC Root [Root]", 3.0);
		} else {
			Runtime::PlayImpactEffect(receiver, "GtsBloodSprayImpactSet", "NPC Head [Head]", NiPoint3{dis(gen), 0, -1}, 512, true, true);
			Runtime::PlayImpactEffect(receiver, "GtsBloodSprayImpactSet", "NPC L Foot [Lft ]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
			Runtime::PlayImpactEffect(receiver, "GtsBloodSprayImpactSet", "NPC R Foot [Rft ]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
		}
        
		// We don't want to call CrushManager::crush here because it will double-transfer the loot
		 
		PrintDeathSource(attacker, receiver, DamageSource::Overkill);

		if (receiver->formID != 0x14) {
			Disintegrate(receiver); // Player can't be disintegrated: simply nothing happens.
		} else if (receiver->formID == 0x14) {
			TriggerScreenBlood(50);
			receiver->SetAlpha(0.0); // Just make player Invisible
		}
	}
}
