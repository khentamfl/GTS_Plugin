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
}

namespace Hooks
{
	void Hook_Damage::Hook(Trampoline& trampoline) {
		static FunctionHook<void(Actor* a_this, float dmg, uintptr_t maybe_hit_data, Actor* aggressor,TESObjectREFR* damageSrc)> SkyrimTakeDamage(
			RELOCATION_ID(36345, 37335),
			[](auto* a_this, auto dmg, auto maybe_hit_data,auto* aggressor,auto* damageSrc) {
			log::info("Someone taking damage");
			log::info("{}: Taking {} damage", a_this->GetDisplayFullName(), dmg);
			float IsNotImmune = 1.0;
			float resistance = GetDamageResistance(a_this) * HugDamageResistance(a_this);
			float healthgate = HealthGate(a_this, dmg * 4);
			float tiny = TinyShield(a_this);

			if (a_this->formID == 0x14) {
				auto transient = Transient::GetSingleton().GetData(a_this);
				if (transient) {
					IsNotImmune = transient->Immunity;
				}
			}
			// float multiplier = GetDamageMultiplier(aggressor);

			//^ Multiplier will CTD since aggressor is none
			// TO-DO: Repair Overkill. Currently disabled

			dmg *= (resistance * tiny * healthgate * IsNotImmune);
			log::info("    - Reducing damage to {}, resistance: {}, shield: {}", dmg, resistance, tiny);
			SkyrimTakeDamage(a_this, dmg, maybe_hit_data, aggressor, damageSrc);
			return;
			}
			);
	}
}
