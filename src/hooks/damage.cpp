#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/hitmanager.hpp"
#include "managers/Attributes.hpp"
#include "utils/actorUtils.hpp"
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
		if (actor->formID == 0x14) {
			auto camera = PlayerCamera::GetSingleton();
			if (!camera) {
				return;
			}
			auto AllowEdits = Persistent::GetSingleton().Camera_PermitFovEdits;
			if (!AllowEdits) {
				return;
			}
			auto tranData = Transient::GetSingleton().GetData(actor);
			bool TP = camera->IsInThirdPerson();
			bool FP = camera->IsInFirstPerson();
			if (tranData) {
				tranData->WorldFov_Default = camera->worldFOV;
				tranData->FpFov_Default = camera->firstPersonFOV;
				tranData->Immunity = 0.0;
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
							tranData->Immunity = 1.0;
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
							tranData->Immunity = 1.0;
							return false; // stop it
						}
						return true;
					});
				}
			}
		}
	}

	void DoOverkill(Actor* attacker, Actor* receiver, float damage) {
		if (damage > GetAV(receiver, ActorValue::kHealth)) { // Overkill effect
			float size_difference = GetSizeDifference(attacker, receiver);
			log::info("Overkill Size Difference: {}", size_difference);
			if (size_difference >= 12.0) {
				HitManager::GetSingleton().Overkill(receiver, attacker);
			}
		}
	}

	float TinyShield(Actor* receiver) {
		float protection = 1.0;
		if (receiver->formID == 0x14) {
			auto grabbedActor = Grab::GetHeldActor(receiver);
			if (grabbedActor) {
				protection = 0.75; // 25% damage reduction
			}
		}
		return protection;
	}

	float HealthGate(Actor* receiver, float a_damage) {
		float protection = 1.0;
		if (receiver->formID == 0x14 && a_damage > GetAV(receiver, ActorValue::kHealth)) {
			if (Runtime::HasPerk(receiver, "HealthGate")) {
				static Timer protect = Timer(60.00);
				if (protect.ShouldRunFrame()) {
					float maxhp = GetMaxAV(receiver, ActorValue::kHealth);
					float scale = get_visual_scale(receiver);

					update_target_scale(receiver, -0.35 * scale, SizeEffectType::kShrink);
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

	float GetTotalDamageResistance(Actor* receiver, Actor* aggressor, float dmg) {
		float resistance = GetDamageResistance(receiver) * HugDamageResistance(receiver);
		float multiplier = GetDamageMultiplier(aggressor);
		float healthgate = HealthGate(receiver, dmg * 4);
		float tiny = 1.0;
		float IsNotImmune = 1.0;

		float mult = 1.0;

		auto transient = Transient::GetSingleton().GetData(receiver);

		if (transient) {
			if (receiver->formID == 0x14) {
				IsNotImmune = transient->Immunity;
				tiny = TinyShield(receiver);
			}
		}
		log::info("    - Damage Mult: {}, resistance: {}, shield: {}", multiplier, resistance, tiny);
		mult *= (multiplier * resistance * tiny * IsNotImmune * healthgate);
		return mult;
	}
}

namespace Hooks
{
	void Hook_Damage::Hook(Trampoline& trampoline) {
		/*static FunctionHook<void(Actor* a_this, float dmg, Actor* aggressor, uintptr_t maybe_hitdata, TESObjectREFR* damageSrc)> SkyrimTakeDamage(
			RELOCATION_ID(36345, 37335),
			[](auto* a_this, auto dmg, auto* aggressor, uintptr_t maybe_hitdata, auto* damageSrc) { // Universal damage function before Difficulty damage
				log::info("Someone taking damage");
				log::info("{}: Taking {} damage", a_this->GetDisplayFullName(), dmg);

				if (aggressor) { // apply to hits only, We don't want to decrease fall damage for example
					if (aggressor != a_this) {
						log::info("Found Aggressor");
						log::info("Aggressor: {}", aggressor->GetDisplayFullName());
						dmg *= GetTotalDamageResistance(a_this, aggressor, dmg);


						DoOverkill(aggressor, a_this, dmg);

						log::info("Changing damage to: {}", dmg);
					}
				}

				SkyrimTakeDamage(a_this, dmg, aggressor, maybe_hitdata, damageSrc);
				return;
			}
		);*/

		static FunctionHook<void(TESObjectREFR* a_this, HitData* hit_data)> SkyrimProcessHitEvent( // Works on HitData based events. Not affected by Difficulty modifiers.
			RELOCATION_ID(37633, 38586),
			[](auto* a_this, auto* hit_data) {
				if (a_this) {
					log::info("Checking a_this");
					log::info("SkyrimProcessHitEvent:a_this: {}", GetRawName(a_this));
				}
				if (hit_data) {
					if (hit_data->aggressor) {
						log::info("Checking agressor");
						Actor* aggressor = hit_data->aggressor.get().get();
						if (hit_data->target) {
							Actor* receiver = hit_data->target.get().get();

							log::info("aggressor: {}", aggressor->GetDisplayFullName());
							log::info("receiver: {}", receiver->GetDisplayFullName());

							log::info("Push: {}", hit_data->pushBack);
							log::info("Total Damage: {}", hit_data->totalDamage);
							log::info("Physical Damage: {}", hit_data->physicalDamage);

							float rec_scale = std::powf(get_visual_scale(receiver), 3.0);
							float att_scale = std::powf(get_visual_scale(aggressor), 3.0);
							float sizedifference = std::clamp(rec_scale/att_scale, 1.0f, 100.0f);

							auto AttackData = hit_data->attackData.get().data;
							if (AttackData) {
								log::info("DamageMult old: {}", AttackData->damageMult);
								log::info("Knockdown: {}", AttackData->knockDown);
								AttackData->damageMult *= GetTotalDamageResistance(receiver, aggressor, hit_data->physicalDamage);
								log::info("DamageMult new: {}", AttackData->damageMult);
							}

							//hit_data->physicalDamage *= GetTotalDamageResistance(receiver, aggressor, hit_data->physicalDamage);
							//hit_data->totalDamage *= GetTotalDamageResistance(receiver, aggressor, hit_data->totalDamage);

							log::info("New push: {}", hit_data->pushBack);

							log::info("New Total Damage: {}", hit_data->totalDamage);
							log::info("New Physical Damage: {}", hit_data->physicalDamage);
							log::info("bonusHealthDamageMult {}", hit_data->bonusHealthDamageMult);

							DoOverkill(aggressor, receiver, hit_data->physicalDamage);
						}
					}
				}
				SkyrimProcessHitEvent(a_this, hit_data);
			}
		);

      // This is the function that applies ValueModifierEffect to an actor including any damage and av adjustments
      // static FunctionHook<void(ValueModifierEffect* a_this, Actor* actor, std::uintptr_t unknown, ActorValue actorValue)> SkyrimAvMagicEffect(
      //   RELOCATION_ID(34286, 35086),
  		// 	[](auto* a_this, auto* actor, auto unknown, auto av) {
      //     if (actorValue == ActorValue::kHealth) {
      //       auto casterHandle = a_this->caster;
      //       if (casterHandle) {
      //         Actor* caster = casterHandle.get().get();
      //         if (caster) {
      //           EffectSetting* baseEffect = a_this->GetBaseObject();
      //           if (baseEffect) {
      //             if (baseEffect->HasKeywordString("MagicDamageFire") || baseEffect->HasKeywordString("MagicDamageFrost") || baseEffect->HasKeywordString("MagicDamageShock")) {
      //               // Physical spells are scaled
      //               a_this->magnitude;
      //             }
      //           }
      //         }
      //       }
      //     }
      //     SkyrimAvMagicEffect(a_this, actor, unknown, av);
      // });


      // Scale all magic based damage // 567A80 (SE)
	  //Actor* a_this, float dmg, Actor* aggressor, std::uintptr_t unknown, TESObjectREFR* damageSrc
     /*static CallHook<void(ActorValue* param1, float dmg, Actor* aggressor, uintptr_t param4, Actor* victim)> SkyrimMagicDamage(
        RELOCATION_ID(34286, 35086), 
        RELOCATION_OFFSET(0x237, 0x232),
        [](auto* param1, auto dmg, auto* aggressor, auto param4, auto* victim) {
			//log::info("a_this: {}", GetRawName(a_this));
			//log::info("agressor: {}", GetRawName(aggressor));
			//log::info("unknown: {}", GetRawName(unknown));
			//log::info("damage src: {}", GetRawName(damageSrc));

			// Param 1 = .?AVCharacter@@
			// Param 2 = float (damage), printed fine
			// Param 3, 5 = AVCharacter (Actor) (printed name just fine)
			// param 4 = ???

			log::info("Param1: {}", GetRawName(param1));
			if (victim) {
				log::info("Found victim");
				if (aggressor) {
					log::info("Found aggressor");
					log::info("Aggressor name: {}", aggressor->GetDisplayFullName());
					log::info("Victim name: {}", victim->GetDisplayFullName());
					log::info("Damage: {}", dmg);
					//dmg = dmg * (std::clamp(get_visual_scale(aggressor), 0.1f, 10.0f) / std::clamp(get_visual_scale(victim), 0.1f, 10.0f));
				} else {
					log::info("Not Found aggressor");
					log::info("Damage: {}", dmg);
					//dmg = dmg / std::clamp(get_visual_scale(victim), 0.1f, 10.0f);
				}
			}
			SkyrimMagicDamage(param1, dmg, aggressor, param4, victim);
			}
		);*/
	}
}
