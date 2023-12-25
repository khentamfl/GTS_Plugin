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

	void Overkill(Actor* attacker, Actor* receiver, float damage) {
		if (damage > GetAV(receiver, ActorValue::kHealth) * 1) { // Overkill effect
			float attackerscale = get_visual_scale(attacker);
			float receiverscale = get_visual_scale(receiver) * GetScaleAdjustment(receiver);
			float size_difference = attackerscale/receiverscale;
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
		mult *= (resistance * tiny * IsNotImmune * healthgate);
		return mult;
	}
}

namespace Hooks
{
	void Hook_Damage::Hook(Trampoline& trampoline) {
		static FunctionHook<void(Actor* a_this, float dmg, Actor* aggressor, uintptr_t maybe_hitdata, TESObjectREFR* damageSrc)> SkyrimTakeDamage(
			RELOCATION_ID(36345, 37335),
			[](auto* a_this, auto dmg, auto* aggressor, uintptr_t maybe_hitdata, auto* damageSrc) { // Universal damage function before Difficulty damage
				log::info("Someone taking damage");
				log::info("{}: Taking {} damage", a_this->GetDisplayFullName(), dmg);
			
				if (aggressor) { // apply to hits only, We don't want to decrease fall damage for example
					log::info("Found Aggressor");
					log::info("Aggressor: {}", aggressor->GetDisplayFullName());
					dmg *= GetTotalDamageResistance(a_this, aggressor, dmg);

					Overkill(aggressor, a_this, dmg);

					log::info("Changing damage to: {}", dmg);
				}
				
				SkyrimTakeDamage(a_this, dmg, hit_data, aggressor, damageSrc);
				return;
			}
		);
	
		static FunctionHook<void(TESObjectREFR* a_this, HitData* hit_data)> SkyrimProcessHitEvent( // Seems to work for Hit events, probably modified by Difficulty Damage
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
							float push = hit_data->pushBack;

							log::info("Push: {}", push);
							log::info("Total Damage: {}", hit_data->totalDamage);
							log::info("Physical Damage: {}", hit_data->physicalDamage);

							float rec_scale = std::powf(get_visual_scale(receiver), 3.0);
							float att_scale = std::powf(get_visual_scale(aggressor), 3.0);
							float sizedifference = std::clamp(rec_scale/att_scale, 1.0f, 100.0f);
							if (push > 0.01) { // We don't want to do 0/0 which will lead to ctd
								push /= sizedifference; 
							}
						}
						
						log::info("SkyrimProcessHitEvent:hit_data->aggressor: {}", GetRawName(aggressor));
					}
				}
				SkyrimProcessHitEvent(a_this, hit_data);
			}
		);
	}
}
