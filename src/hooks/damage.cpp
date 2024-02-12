#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/OverkillManager.hpp"
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
#include "data/time.hpp"
#include "timer.hpp"


using namespace RE;
using namespace SKSE;

namespace {
	void CameraFOVTask_TP(Actor* actor, PlayerCamera* camera, TempActorData* data, bool AllowEdits) {
		std::string name = std::format("CheatDeath_TP_{}", actor->formID);
		ActorHandle gianthandle = actor->CreateRefHandle();

		if (AllowEdits) {
			camera->worldFOV *= 0.35;
		}

		float DefaultTP = data->WorldFov_Default;
		float Start = Time::WorldTimeElapsed();

		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			float Finish = Time::WorldTimeElapsed();

			if (AllowEdits) {
				camera->worldFOV += DefaultTP * 0.003;
				if (camera->worldFOV >= DefaultTP) {
					camera->worldFOV = DefaultTP;
					data->IsNotImmune = 1.0;
					return false; // stop it
				}
			} else {
				float timepassed = Finish - Start;
				if (timepassed > 2.6) {
					data->IsNotImmune = 1.0;
					return false;
				}
			}
			return true;
		});
	}
	void CameraFOVTask_FP(Actor* actor, PlayerCamera* camera, TempActorData* data, bool AllowEdits) {
		std::string name = std::format("CheatDeath_FP_{}", actor->formID);
		ActorHandle gianthandle = actor->CreateRefHandle();

		camera->firstPersonFOV *= 0.35;
		float DefaultFP = data->FpFov_Default;

		float Start = Time::WorldTimeElapsed();

		TaskManager::Run(name,[=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}

			auto giantref = gianthandle.get().get();
			float Finish = Time::WorldTimeElapsed();

			if (AllowEdits) {
				camera->firstPersonFOV += DefaultFP * 0.003;
				if (camera->firstPersonFOV >= DefaultFP) {
					camera->firstPersonFOV = DefaultFP;
					data->IsNotImmune = 1.0;
					return false; // stop it
				}
			} else {
				float timepassed = Finish - Start;
				if (timepassed > 2.6) {
					data->IsNotImmune = 1.0;
					return false;
				}
			}
			return true;
		});
	}

	void StartDamageIsNotImmune(Actor* actor) {
		if (actor->formID == 0x14) {
			auto camera = PlayerCamera::GetSingleton();
			if (!camera) {
				return;
			}
			auto AllowEdits = Persistent::GetSingleton().Camera_PermitFovEdits;

			auto tranData = Transient::GetSingleton().GetData(actor);
			bool TP = camera->IsInThirdPerson();
			bool FP = camera->IsInFirstPerson();
			if (tranData) {
				tranData->WorldFov_Default = camera->worldFOV;
				tranData->FpFov_Default = camera->firstPersonFOV;
				tranData->IsNotImmune = 0.0; // make actor immune to damage
				if (TP) {
					CameraFOVTask_TP(actor, camera, tranData, AllowEdits);
				} else if (FP) {
					CameraFOVTask_FP(actor, camera, tranData, AllowEdits);
				}
			}
		}
	}

	void DoOverkill(Actor* attacker, Actor* receiver, float damage) {
		if (damage > GetMaxAV(receiver, ActorValue::kHealth)) { // Overkill effect
			float size_difference = GetSizeDifference(attacker, receiver, true);
			if (size_difference >= 12.0) {
				OverkillManager::GetSingleton().Overkill(attacker, receiver);
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

	float HealthGate(Actor* receiver, Actor* attacker, float a_damage) {
		float protection = 1.0;
		if (receiver->formID == 0x14 && a_damage > GetAV(receiver, ActorValue::kHealth)) {
			if (Runtime::HasPerk(receiver, "HealthGate")) {
				static Timer protect = Timer(60.00);
				if (protect.ShouldRunFrame()) {
					float maxhp = GetMaxAV(receiver, ActorValue::kHealth);
					float target = get_target_scale(receiver);
					float natural = get_natural_scale(receiver);

					float scale = get_visual_scale(receiver);

					update_target_scale(receiver, -0.35 * scale, SizeEffectType::kShrink);
					if (target < natural) {
						set_target_scale(receiver, natural); // to prevent becoming < natural scale
					}
					GRumble::For("CheatDeath", receiver, 240.0, 0.10, "NPC COM [COM ]", 1.50);
					Runtime::PlaySound("TriggerHG", receiver, 2.0, 0.5);
					
					StaggerActor(receiver, attacker, 1.0f);
					StaggerActor(attacker, receiver, 1.0f);
					// stagger each-other

					StartDamageIsNotImmune(receiver);

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

	float GetTotalDamageResistance(Actor* receiver, Actor* aggressor) {
		float resistance = GetDamageResistance(receiver) * HugDamageResistance(receiver);
		float multiplier = GetDamageMultiplier(aggressor);
		float tiny = 1.0;
		float IsNotImmune = 1.0;

		float mult = 1.0;

		auto transient = Transient::GetSingleton().GetData(receiver);

		if (transient) {
			if (receiver->formID == 0x14) {
				IsNotImmune = transient->IsNotImmune;
				tiny = TinyShield(receiver);
			}
		}
		//log::info("    - Damage Mult: {}, resistance: {}, shield: {}", multiplier, resistance, tiny);
		mult *= (multiplier * resistance * tiny * IsNotImmune);
		return mult;
	}
}

namespace Hooks
{
	void Hook_Damage::Hook(Trampoline& trampoline) {

    // SE(5D6300)
		static FunctionHook<void(Actor* a_this, float dmg, Actor* aggressor, uintptr_t maybe_hitdata, TESObjectREFR* damageSrc)> SkyrimTakeDamage(
			RELOCATION_ID(36345, 37335),
			[](auto* a_this, auto dmg, auto* aggressor, uintptr_t maybe_hitdata, auto* damageSrc) { // Universal damage function before Difficulty damage
				//log::info("Someone taking damage");
				//log::info("{}: Taking {} damage", a_this->GetDisplayFullName(), dmg);

				if (aggressor) { // apply to hits only, We don't want to decrease fall damage for example
					if (aggressor != a_this) {
						//log::info("Found Aggressor");
						//log::info("Aggressor: {}", aggressor->GetDisplayFullName());
						dmg *= GetTotalDamageResistance(a_this, aggressor);
						dmg *= HealthGate(a_this, aggressor, dmg);


						DoOverkill(aggressor, a_this, dmg);

						//log::info("Changing damage to: {}", dmg);
					}
				}

				SkyrimTakeDamage(a_this, dmg, aggressor, maybe_hitdata, damageSrc);
				return;
			}
		);

		static FunctionHook<void(Actor* a_this, uintptr_t param_1, uintptr_t param_2, uintptr_t* param_3, uintptr_t param_4, uintptr_t param_5, uintptr_t param_6)> SkyrimTakeDamage_2(
			// SE: 140621120 : 37523
			RELOCATION_ID(37523, 37523),
			[](auto* a_this, uintptr_t param_1, uintptr_t param_2, uintptr_t* param_3, uintptr_t param_4, uintptr_t param_5, uintptr_t param_6) {
				if (a_this) {
					log::info("Actor: {}", a_this->GetDisplayFullName());
				}
				log::info("Param 1: {}", param_1);
				log::info("Param 2: {}", param_2);
				//if (param_3) {
					//log::info("Param 3 true");
					//log::info("Param 3: {}", GetRawName(param_3));
				//}
				log::info("Param 4: {}", param_4);
				log::info("Param 5: {}", param_5);
				log::info("Param 6: {}", param_6);

			
				SkyrimTakeDamage_2(a_this, param_1, param_2, param_3, param_4, param_5, param_6);
				return;
			}
		);
	}
}
