#include "managers/animation/AnimationManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/CrushManager.hpp"
#include "managers/hitmanager.hpp"
#include "managers/Attributes.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "events.hpp"
#include "Config.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	void Overkill(Actor* attacker, Actor* receiver, float damage) {
		if (damage > GetAV(receiver, ActorValue::kHealth) * 1.5) { // Overkill effect
			float attackerscale = get_visual_scale(attacker);
			float receiverscale = get_visual_scale(receiver);
			if (IsDragon(receiver) || IsGiant(receiver)) {
				receiverscale *= 2.0;
			} if (IsMammoth(receiver)) {
				receiverscale *= 3.0;
			}
			float size_difference = attackerscale/receiverscale;
			if (size_difference >= 12.0) {
				HitManager::GetSingleton().Overkill(receiver, attacker);
			}
		}
	}

	void StaggerImmunity(Actor* attacker, Actor* receiver) {
		float sizedifference = get_visual_scale(receiver)/get_visual_scale(attacker);
		auto charCont = receiver->GetCharController();
		if (charCont) {
			receiver->SetGraphVariableFloat("GiantessScale", sizedifference); // Manages Stagger Resistance inside Behaviors.
		}
	}

	void TinyAsShield(Actor* attacker, Actor* receiver, float a_damage) {

		auto grabbedActor = Grab::GetHeldActor(receiver);
		if (!grabbedActor) {
			return;
		}
		if (IsTeammate(grabbedActor)) {
			return; // Don't kill teammates
		}
		if (grabbedActor == attacker) {
			return; // Don't allow actor to do self-damage
		}
		receiver->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, a_damage * 0.25);
		DamageAV(grabbedActor, ActorValue::kHealth, a_damage * 0.25);
		if (grabbedActor->IsDead() || GetAV(grabbedActor, ActorValue::kHealth) < a_damage * 0.25) {
			if (!IsBetweenBreasts(receiver)) {
				PrintDeathSource(receiver, grabbedActor, DamageSource::BlockDamage);
			} else {
				PrintDeathSource(receiver, grabbedActor, DamageSource::Breast);
			}

			Grab::DetachActorTask(receiver);
			auto hand = find_node(receiver, "NPC L Hand [LHnd]");
			if (hand) {
				if (IsLiving(grabbedActor)) {
					SpawnParticle(receiver, 25.0, "GTS/Damage/Explode.nif", hand->world.rotate, hand->world.translate, get_visual_scale(grabbedActor) * 5, 4, hand);
					SpawnParticle(receiver, 25.0, "GTS/Damage/Crush.nif", hand->world.rotate, hand->world.translate, get_visual_scale(grabbedActor) * 5, 4, hand);
				} else {
					SpawnDustParticle(receiver, attacker, "NPC L Hand [LHnd]", 3.0);
				}
			}
			CrushManager::Crush(receiver, grabbedActor);
			if (!LessGore()) {
				Runtime::PlaySoundAtNode("CrunchImpactSound", receiver, 1.0, 0.0, "NPC L Hand [LHnd]");
				Runtime::PlaySoundAtNode("CrunchImpactSound", receiver, 1.0, 0.0, "NPC L Hand [LHnd]");
				Runtime::PlaySoundAtNode("CrunchImpactSound", receiver, 1.0, 0.0, "NPC L Hand [LHnd]");
			} else {
				Runtime::PlaySoundAtNode("SoftHandAttack", receiver, 1.0, 1.0, "NPC L Hand [LHnd]");
			}
			GRumble::Once("GrabAttackKill", receiver, 8.0, 0.15, "NPC L Hand [LHnd]");
			AnimationManager::StartAnim("GrabAbort", receiver); // Abort Grab animation
			Grab::Release(receiver);
		}
	}

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

	void HealthGate(Actor* attacker, Actor* receiver, float a_damage) {
		if (a_damage > GetAV(receiver, ActorValue::kHealth)) {
			if (Runtime::HasPerk(receiver, "HealthGate")) {
				static Timer protect = Timer(60.00);
				if (protect.ShouldRunFrame()) {
					float maxhp = GetMaxAV(receiver, ActorValue::kHealth);
					float scale = get_visual_scale(receiver);
					attacker->SetGraphVariableFloat("staggerMagnitude", 100.00f); // Stagger actor
					attacker->NotifyAnimationGraph("staggerStart");

					mod_target_scale(receiver, -0.35 * scale);
					GRumble::For("CheatDeath", receiver, 240.0, 0.10, "NPC COM [COM ]", 1.50);
					Runtime::PlaySound("TriggerHG", receiver, 2.0, 0.5);
					receiver->SetGraphVariableFloat("staggerMagnitude", 100.00f); // Stagger actor
					receiver->NotifyAnimationGraph("staggerStart");

					float overkill = a_damage + maxhp/5;

					receiver->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, overkill); // Restore to full

					CameraFOVTask(receiver);

					Cprint("Health Gate triggered, death avoided");
					Cprint("Damage: {:.2f}, Lost Size: {:.2f}", a_damage, -0.35 * scale);
					Notify("Health Gate triggered, death avoided");
					Notify("Damage: {:.2f}, Lost Size: {:.2f}", a_damage, -0.35 * scale);
				}
			}
		}
		if (Runtime::HasPerk(receiver, "DarkArts_Max") && GetHealthPercentage(receiver) <= 0.40) {
			static Timer Shrink = Timer(180.00);
			if (Shrink.ShouldRunFrame()) {
				ShrinkOutburstExplosion(receiver, true);
			}
		}
	}

	void HugDamageResistance(Actor* receiver, float damage) {
		if (!Runtime::HasPerk(receiver, "HugCrush_ToughGrip")) {
			return;
		}
		if (HugShrink::GetHuggiesActor(receiver)) {
			float reduction = 0.25; // 25% resistance
			if (Runtime::HasPerk(receiver, "HugCrush_HugsOfDeath")) {
				reduction += 0.35; // 35% additional resistance
			}
			receiver->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, damage * reduction);
			// ^ Restore % hp, fake damage resistance
		}
	}

	void DropTinyChance(Actor* receiver, float damage, float scale) {
		static Timer DropTimer = Timer(0.33); // Check once per .33 sec
		float bonus = 1.0;
		if (Runtime::HasPerkTeam(receiver, "HugCrush_HugsOfDeath")) {
			return; // Full immunity
		}
		if (Runtime::HasPerkTeam(receiver, "HugCrush_Greed")) {
			bonus = 4.0; // 4 times bigger damage threshold to cancel hugs
		}
		if (Runtime::HasPerkTeam(receiver, "HugCrush_ToughGrip")) {
			float GetHP = GetHealthPercentage(receiver);
			if (GetHP >= 0.85) {
				return; // Drop only if hp is < 85%
			}
		}
		if (damage < 6.0 * bonus * scale) {
			return;
		}
		HugShrink::CallRelease(receiver); // Else release
	}

	float GetAttackBonus(Actor* giant) {
		return AttributeManager::GetSingleton().GetAttributeBonus(giant, ActorValue::kAttackDamageMult);
	}

	void InflictDamage(Actor* attacker, Actor* receiver, float a_damage) { // function receives negative number first (-6.0 for example)
		log::info("Attacker: {}, Reciever: {}, a_damage: {}", attacker->GetDisplayFullName(), receiver->GetDisplayFullName(), a_damage);
		float damagemult_att = GetAttackBonus(attacker);
		float damageresist = AttributeManager::GetSingleton().GetAttackBonus(receiver, ActorValue::khealth); // get damage resistance
		float damage = (a_damage * damagemult_att) - (a_damage * damageresist); // damage is used to restore health if > 0, if < = deal more damage
		// We * damage by damage mult (actor damage bonus) and reduce damage by original damage. (-6.0 * 0.8) - 6.0
		// as a result, -4.8 - 6.0 
		// We restore 1.2 points of health based on that.
		log::info("a_damage * mult: {}, damage result: {}", a_damage * damagemult_att, damage);
		log::info("Resistance of {} is {}", receiver->GetDisplayFullName(), damageresist);
		float sizedifference = get_visual_scale(receiver)/get_visual_scale(attacker);
		// apply other functions
		HealthGate(attacker, receiver, -(a_damage + damage));
		TinyAsShield(attacker, receiver, -(a_damage + damage));
		HugDamageResistance(receiver, -(a_damage + damage));
		DropTinyChance(receiver, -(a_damage + damage), sizedifference);

		if (damage < 0) {
			Overkill(attacker, receiver, -(a_damage + damage));
			InflictSizeDamage(attacker, receiver, -damage); // damage hp
			return;
		}
		if (damage > 0) {
			receiver->AsActorValueOwner()->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, damage); // Restore hp
		}
	}

	void DoHitShake(Actor* receiver, float value) {
		if (IsFirstPerson()) {
			value *= 0.05;
		}
		GRumble::Once("HitGrowth", receiver, value, 0.15);
	}
}

namespace Gts {

	SizeHitEffects& SizeHitEffects::GetSingleton() noexcept {
		static SizeHitEffects instance;
		return instance;
	}

	std::string SizeHitEffects::DebugName() {
		return "SizeHitEffects";
	}

	void SizeHitEffects::ApplyEverything(Actor* attacker, Actor* receiver, float damage) {
		InflictDamage(attacker, receiver, damage);
		StaggerImmunity(attacker, receiver);
		SizeHitEffects::GetSingleton().DoHitGrowth(receiver, attacker, damage);
	}


	void SizeHitEffects::DoHitGrowth(Actor* receiver, Actor* attacker, float damage) {
		auto grabbedActor = Grab::GetHeldActor(receiver);
		if (grabbedActor == attacker) {
			return;
		} if (attacker == receiver) {
			return;
		}
		int LaughChance = rand() % 12;
		int ShrinkChance = rand() % 5;
		float scale = get_visual_scale(receiver);
		float naturalscale = get_visual_scale(receiver);
		auto& sizemanager = SizeManager::GetSingleton();
		float BalanceMode = sizemanager.BalancedMode();
		float SizeHunger = 1.0 + sizemanager.GetSizeHungerBonus(receiver)/100;
		float Gigantism = 1.0 + sizemanager.GetEnchantmentBonus(receiver)/100;
		float SizeDifference = get_visual_scale(receiver)/get_visual_scale(attacker);
		float Dragon = 1.0;
		if (IsDragon(attacker) || IsGiant(attacker) || IsMammoth(attacker)) {
			Dragon = 2.5;
		}
		float resistance = 1.0;
		static Timer soundtimer = Timer(1.5);
		static Timer laughtimer = Timer(4.0);

		if (Runtime::HasMagicEffect(receiver, "ResistShrinkPotion")) {
			resistance = 0.25;
		}

		if (receiver->formID == 0x14 && Runtime::HasPerk(receiver, "GrowthOnHitPerk") && sizemanager.GetHitGrowth(receiver) >= 1.0) {
			float GrowthValue = std::clamp((-damage/2800) * SizeHunger * Gigantism, 0.0f, 0.25f * Gigantism);
			mod_target_scale(receiver, GrowthValue);
			DoHitShake(receiver, GrowthValue * 10);
			if (soundtimer.ShouldRunFrame()) {
				Runtime::PlaySoundAtNode("growthSound", receiver, GrowthValue * 2, 1.0, "NPC Pelvis [Pelv]");
			}
			if (ShrinkChance >= 2) {
				mod_target_scale(attacker, -GrowthValue/(6.0 * Dragon* BalanceMode)); // Shrink Attacker
				mod_target_scale(receiver, GrowthValue/(2.0 * Dragon * BalanceMode)); // Grow Attacker
				if (get_visual_scale(attacker) <= 0.12/Dragon) {
					mod_target_scale(attacker, 0.12/Dragon);
				}
			}
			if (SizeDifference >= 4.0 && LaughChance >= 11 && laughtimer.ShouldRunFrame()) {
				Runtime::PlaySoundAtNode("LaughSound", receiver, 1.0, 0.5, "NPC Head [Head]");
			}
			return;
		} else if (BalanceMode >= 2.0 && receiver->formID == 0x14 && !Runtime::HasPerk(receiver, "GrowthOnHitPerk")) { // Shrink us
			if (scale > naturalscale) {
				float sizebonus = std::clamp(get_visual_scale(attacker), 0.10f, 1.0f);
				float ShrinkValue = std::clamp(((-damage/850)/SizeHunger/Gigantism * sizebonus) * resistance, 0.0f, 0.25f / Gigantism); // affect limit by decreasing it

				if (scale < naturalscale) {
					set_target_scale(receiver, naturalscale); // reset to normal scale
					return;
				}
				mod_target_scale(receiver, -ShrinkValue);
			}
		}
	}

	void SizeHitEffects::BreakBones(Actor* giant, Actor* tiny, float damage, int random) { // Used as a debuff
		if (tiny->IsDead()) {
			return;
		}
		if (!Runtime::HasPerkTeam(giant, "BoneCrusher")) {
			return;
		}
		int rng = (rand()% random + 1);
		if (rng <= 2) {
			float gs = get_visual_scale(giant);
			float ts = get_visual_scale(tiny);
			if (HasSMT(giant)) {
				gs += 3.0; // Allow to break bones with SMT
			}
			float sizediff = gs/ts;
			if (sizediff < 3.0) {
				return;
			}

			std::random_device rd;
			std::mt19937 gen(rd());
			std::uniform_real_distribution<float> dis(-0.2, 0.2);
			if (!IsLiving(tiny)) {
				SpawnDustParticle(giant, tiny, "NPC Root [Root]", 1.0);
			} else {
				auto root = find_node(tiny, "NPC Root [Root]");
				if (root) {
					SpawnParticle(tiny, 0.20, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, ts * 1, 7, root);
				}
			}
			SizeManager::GetSingleton().ModSizeVulnerability(tiny, 0.15);
			InflictSizeDamage(giant, tiny, damage);
			float Health = GetAV(tiny, ActorValue::kHealth);
			float sizedifference = get_visual_scale(giant)/get_visual_scale(tiny);
			
			if (damage >= Health && sizedifference >= 8.0) {
				CrushManager::Crush(giant, tiny);
			}
		}
	}
}
