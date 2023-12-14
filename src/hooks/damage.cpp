#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/Attributes.hpp"
#include "managers/Rumble.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "hooks/damage.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"

using namespace RE;
using namespace SKSE;

namespace {
  void CameraFOVTask(Actor* actor) {
    auto camera = PlayerCamera::GetSingleton();
    if (!camera) {
      return;
    }
    auto AllowEdits = Persistent::GetSingleton().Camera_PermitFovEdits;
    if (!AllowEdits) {
      return;
    }
    if (actor->formID == 0x14) {
      auto tranData = Transient::GetSingleton().GetData(actor);
      bool TP = camera->IsInThirdPerson();
      bool FP = camera->IsInFirstPerson();
      if (tranData) {
        tranData->WorldFov_Default = camera->worldFOV;
        tranData->FpFov_Default = camera->firstPersonFOV;
        float DefaultTP = tranData->WorldFov_Default;
        float DefaultFP = tranData->FpFov_Default;
        if (DefaultTP > 0) {
          std::string name = std::format("RandomGrowth_TP_{}", actor->formID);
          ActorHandle gianthandle = actor->CreateRefHandle();
          camera->worldFOV *= 0.35;
          TaskManager::Run(name, [=](auto& progressData) {
            if (!gianthandle) {
              return false;
            }
            auto giantref = gianthandle.get().get();
            camera->worldFOV += DefaultTP * 0.003;
            if (camera->worldFOV >= DefaultTP) {
              camera->worldFOV = DefaultTP;
              return false; // stop it
            }
            return true;
          });
        } else if (FP && DefaultFP > 99999) {
          std::string name = std::format("RandomGrowth_FP_{}", actor->formID);
          ActorHandle gianthandle = actor->CreateRefHandle();
          camera->firstPersonFOV *= 0.35;
          TaskManager::Run(name,[=](auto& progressData) {
            if (!gianthandle) {
              return false;
            }
            auto giantref = gianthandle.get().get();
            camera->firstPersonFOV += DefaultFP * 0.003;
            if (camera->firstPersonFOV >= DefaultFP) {
              camera->firstPersonFOV = DefaultFP;
              return false; // stop it
            }
            return true;
          });
        }
      }
    }
  }
  float TinyShied(Actor* receiver) {
    float protection = 1.0;
    if (receiver->formID == 0x14) {
      auto grabbedActor = Grab::GetHeldActor(receiver);
      if (grabbedActor) {
        protection = 0.75;
      } 
    }
    return protection; // 25% damage reduction
  }
  float HealthGate(Actor* receiver, Actor* attacker, float a_damage) {
    float protection = 1.0;
		if (receiver->formID == 0x14 && a_damage > GetAV(receiver, ActorValue::kHealth)) {
      if (Runtime::HasPerk(receiver, "HealthGate")) {
          static Timer protect = Timer(60.00);
        if (protect.ShouldRunFrame()) {
            float maxhp = GetMaxAV(receiver, ActorValue::kHealth);
            float scale = get_visual_scale(receiver);
            if (attacker) {
              attacker->SetGraphVariableFloat("staggerMagnitude", 100.00f); // Stagger actor
              attacker->NotifyAnimationGraph("staggerStart");
            } else {
              log::info("ATTACKER IS NONE");
            }

            mod_target_scale(receiver, -0.35 * scale);
            GRumble::For("CheatDeath", receiver, 240.0, 0.10, "NPC COM [COM ]", 1.50);
            Runtime::PlaySound("TriggerHG", receiver, 2.0, 0.5);
            receiver->SetGraphVariableFloat("staggerMagnitude", 100.00f); // Stagger actor
            receiver->NotifyAnimationGraph("staggerStart");

            CameraFOVTask(receiver);

            Cprint("Health Gate triggered, death avoided");
            Cprint("Damage: {:.2f}, Lost Size: {:.2f}", a_damage, -0.35 * scale);
            Notify("Health Gate triggered, death avoided");
            Notify("Damage: {:.2f}, Lost Size: {:.2f}", a_damage, -0.35 * scale);
            protection = 0.0;
            }
        }
		}
		if (Runtime::HasPerk(receiver, "DarkArts_Max") && GetHealthPercentage(receiver) <= 0.40) {
			static Timer Shrink = Timer(180.00);
			if (Shrink.ShouldRunFrame()) {
				ShrinkOutburstExplosion(receiver, true);
			}
		}
    return protection;
	}
  void ApplyHitGrowth(Actor* receiver, Actor* attacker, float damage) {
      SizeHitEffects::GetSingleton().DoHitGrowth(receiver, attacker, damage);
  }
  float GetDamageMultiplier(Actor* attacker) {
      return AttributeManager::GetSingleton().GetAttributeBonus(attacker, ActorValue::kAttackDamageMult);
  }
  float GetDamageResistance(Actor* receiver) {
      return AttributeManager::GetSingleton().GetAttributeBonus(receiver, ActorValue::kHealth);
  }

  float HugDamageResistance(Actor* receiver) {
    float reduction = 1.0;
    if (HugShrink::GetHuggiesActor(receiver)) {
        if (Runtime::HasPerk(receiver, "HugCrush_ToughGrip")) {
          reduction -= 0.25; // 25% resistance
        }
        if (Runtime::HasPerk(receiver, "HugCrush_HugsOfDeath")) {
          reduction -= 0.35; // 35% additional resistance
        }
      }
      return reduction;
    }
}

namespace Hooks
{
	void Hook_Damage::Hook(Trampoline& trampoline) {
		static FunctionHook<void(Actor* a_this, float dmg, uintptr_t maybe_hit_data, Actor* aggressor,TESObjectREFR* damageSrc)> SkyrimTakeDamage(
      RELOCATION_ID(36345, 37335),
      [](auto* a_this, auto dmg, auto maybe_hit_data,auto* aggressor,auto* damageSrc) {
        log::info("{}: Taking {} damage", a_this->GetDisplayFullName(), dmg);

        log::info("Looking for Aggressor");
        Actor* attacker = skyrim_cast<Actor*>(aggressor);
        if (attacker) {
          log::info("Aggressor Name: {}", attacker->GetDisplayFullName());
        } 
        log::info("Aggressor numbers: {}", aggressor);
        
        
        /*float resistance = GetDamageResistance(a_this) * HugDamageResistance(a_this);
        float healthgate = HealthGate(a_this, aggressor, dmg * 4);
        float multiplier = GetDamageMultiplier(aggressor);
        float tiny = TinyShied(a_this);*/
        
        //dmg *= (resistance * multiplier * tiny);
        //dmg *= healthgate;
        //log::info("    - Reducing damage to {}, resistance: {}, multiplier: {}, shield: {}", dmg, resistance, multiplier, tiny);
        SkyrimTakeDamage(a_this, dmg, maybe_hit_data, aggressor, damageSrc);
        //ApplyHitGrowth(a_this, aggressor, dmg);
        return;
			}
    );
	}
}
