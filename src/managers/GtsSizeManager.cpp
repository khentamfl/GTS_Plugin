#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/highheel.hpp"
#include "magic/effects/common.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "timer.hpp"
#include "node.hpp"

#include <random>


using namespace Gts;
using namespace RE;
using namespace REL;
using namespace SKSE;

namespace {
	const double LAUNCH_COOLDOWN = 3.0;
	const double DAMAGE_COOLDOWN = 0.10;
	const float LAUNCH_DAMAGE_BASE = 2.0f;
	const float LAUNCH_KNOCKBACK_BASE = 0.02f;

	void SmallMassiveThreatModification(Actor* Caster, Actor* Target) {
		if (!Caster || !Target || Caster == Target) {
			return;
		}
		auto& persistent = Persistent::GetSingleton();
		if (persistent.GetData(Caster)->smt_run_speed >= 1.0) {
			float caster_scale = get_target_scale(Caster);
			float target_scale = get_target_scale(Target);
			float Multiplier = (caster_scale/target_scale);
			float CasterHp = Caster->GetActorValue(ActorValue::kHealth);
			float TargetHp = Target->GetActorValue(ActorValue::kHealth);
			if (CasterHp >= (TargetHp / Multiplier) && !CrushManager::AlreadyCrushed(Target)) {
				CrushManager::Crush(Caster, Target);
				shake_camera(Caster, 0.25 * caster_scale, 0.25);
				ConsoleLog::GetSingleton()->Print("%s was instantly turned into mush by the body of %s", Target->GetDisplayFullName(), Caster->GetDisplayFullName());
				if (Runtime::HasPerk(Caster, "NoSpeedLoss")) {
					AttributeManager::GetSingleton().OverrideBonus(0.65); // Reduce speed after crush
				} else if (!Runtime::HasPerk(Caster, "NoSpeedLoss")) {
					AttributeManager::GetSingleton().OverrideBonus(0.35); // Reduce more speed after crush
				}
			} else if (CasterHp < (TargetHp / Multiplier) && !CrushManager::AlreadyCrushed(Target)) {
				PushActorAway(Caster, Target, 0.8);
				PushActorAway(Target, Caster, 0.2);
				Caster->ApplyCurrent(0.5 * target_scale, 0.5 * target_scale); Target->ApplyCurrent(0.5 * caster_scale, 0.5 * caster_scale);  // Else simulate collision
				Target->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, -CasterHp * 0.75); Caster->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage,ActorValue::kHealth, -CasterHp * 0.25);
				shake_camera(Caster, 0.35, 0.5);
				Runtime::PlaySound("lJumpLand", Caster, 0.5, 1.0);
				std::string text_a = Target->GetDisplayFullName();
				std::string text_b = " is too tough to be crushed";
				std::string Message = text_a + text_b;
				DebugNotification(Message.c_str(), 0, true);
				AttributeManager::GetSingleton().OverrideBonus(0.75); // Less speed loss after force crush
			}
		}
	}

	void SizeModifications(Actor* giant, Actor* target, float HighHeels) {
		float InstaCrushRequirement = 24.0;
		float giantscale = get_visual_scale(giant);
		float targetscale = get_visual_scale(target);
		float size_difference = giantscale/targetscale;
		float Gigantism = 1.0 - SizeManager::GetSingleton().GetEnchantmentBonus(giant)/200;
		float BonusShrink = (IsJumping(giant) * 3.0) + 1.0;

			if (CrushManager::AlreadyCrushed(target)) {
				return;
			}

			if (Runtime::HasPerk(giant, "LethalSprint") && giant->IsSprinting()) {
				InstaCrushRequirement = 18.0 * HighHeels * Gigantism;
			}

			if (size_difference >= InstaCrushRequirement && !target->IsPlayerTeammate()) {
				CrushManager::Crush(giant, target);
				CrushToNothing(giant, target);
			}

			if (Runtime::HasPerk(giant, "ExtraGrowth") && giant != target && (Runtime::HasMagicEffect(giant, "explosiveGrowth1") || Runtime::HasMagicEffect(giant, "explosiveGrowth2") || Runtime::HasMagicEffect(giant, "explosiveGrowth3"))) {
				ShrinkActor(giant, 0.0014 * BonusShrink, 0.0);
				Grow(giant, 0.0, 0.0004 * BonusShrink);
				// ^ Augmentation for Growth Spurt: Steal size of enemies.
			}

			if (Runtime::HasMagicEffect(giant, "SmallMassiveThreat") && giant != target) {
				SmallMassiveThreatModification(giant, target);
				size_difference += 7.2; // Allows to crush same size targets.

			if (Runtime::HasPerk(giant, "SmallMassiveThreatSizeSteal")) {
				float HpRegen = GetMaxAV(giant, ActorValue::kHealth) * 0.005 * size_difference;
				giant->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, ActorValue::kHealth, (HpRegen * TimeScale()) * size_difference);
				ShrinkActor(giant, 0.0015 * BonusShrink, 0.0);
				Grow(giant, 0.00045 * targetscale * BonusShrink, 0.0);
			}
		}
		
	}
}

namespace Gts {
	SizeManager& SizeManager::GetSingleton() noexcept {
		static SizeManager instance;
		return instance;
	}

	float SizeManager::GetRaceScale(Actor* actor) {
		auto GetNode = find_node(actor, "NPC", false);
		float NodeScale = GetNode->world.scale;
		return GetNode ? NodeScale : 1; // <- not used, causes troubles with quest progression. (Can't reach 1.44 for example when 1.50 is needed.)
	}

	std::string SizeManager::DebugName() {
		return "SizeManager";
	}

	void SizeManager::Update() {
		for (auto actor: find_actors()) {
			// TODO move away from polling
			bool Balance = false; // Toggles Balance Mode for the mod. False = off, true = on.

			float QuestStage = Runtime::GetStage("MainQuest");

			float Gigantism = this->GetEnchantmentBonus(actor)/100;
			float GetLimit = clamp(1.0, 99999999.0, Runtime::GetFloat("sizeLimit")); // Default size limit
			float Persistent_Size = Persistent::GetSingleton().GetData(actor)->bonus_max_size;
			float SelectedFormula = Runtime::GetInt("SelectedSizeFormula");

			float FollowerLimit = Runtime::GetFloat("FollowersSizeLimit");
			float NPCLimit = Runtime::GetFloat("NPCSizeLimit");

			if (SelectedFormula >= 2.0 && actor->formID == 0x14) { // Apply Player Mass-Based max size
				GetLimit = clamp(1.0, 99999999.0, Runtime::GetFloat("MassBasedSizeLimit"));
			} else if (QuestStage > 100 && FollowerLimit > 1 && actor->formID != 0x14 && (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate())) { // Apply Follower Max Size
				GetLimit = clamp(1.0, 99999999.0, Runtime::GetFloat("FollowersSizeLimit")); // Apply only if Quest is done.
			} else if (QuestStage > 100 && NPCLimit > 1 &&  actor->formID != 0x14 && (!Runtime::InFaction(actor, "FollowerFaction") && !actor->IsPlayerTeammate())) { // Apply Other NPC's max size
				GetLimit = clamp(1.0, 99999999.0, Runtime::GetFloat("NPCSizeLimit"));           // Apply only if Quest is done.
			}

			float RaceScale = (GetRaceScale(actor) * (GetLimit + Persistent_Size)) * (1.0 + Gigantism);
			float TotalLimit = ((GetLimit + Persistent_Size) * (1.0 + Gigantism));

			if (TotalLimit < 1.0) {
				TotalLimit = 1.0;
			}
			if (get_max_scale(actor) < TotalLimit || get_max_scale(actor) > TotalLimit) {
				set_max_scale(actor, TotalLimit);
				//log::info("Current Size Limit of: {} is {}", actor->GetDisplayFullName(), get_max_scale(actor));
			}
		}
	}

	void SizeManager::OnHighheelEquip(const HighheelEquip& evt) {
		float hh_length = evt.hhLength;
		Actor* actor = evt.actor;
		// TODO move away from polling
		if (hh_length > 0 && Runtime::HasPerkTeam(actor, "hhBonus")) { // HH damage bonus start
			auto shoe = evt.shoe;
			float shoe_weight = 1.0; // TODO: Just absorb it into the base
			auto char_weight = actor->GetWeight()/280;
			if (shoe) {
				shoe_weight = shoe->weight/20;
			}
			float expectedhhdamage = 1.5 + shoe_weight + char_weight;
			if (this->GetSizeAttribute(actor, 3) != expectedhhdamage) {
				this->SetSizeAttribute(actor, 1.5 + shoe_weight + char_weight, 3);
				log::info("SizeManager HH Actor {} value: {}", actor->GetDisplayFullName(), this->GetSizeAttribute(actor, 3));
			}
		} else if (hh_length <= 1e-4) {
			if (this->GetSizeAttribute(actor, 3) != 1.0) {
				this->SetSizeAttribute(actor, 1.0, 3);
				log::info("SizeManager HH Actor {} RESET value: {}", actor->GetDisplayFullName(), this->GetSizeAttribute(actor, 3));
			}
		}
	}

	void SizeManager::UnderFootEvent(const UnderFoot& evt) {
		auto giant = evt.giant;
		auto tiny = evt.tiny;
		float force = evt.force;

		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->formID == 0x14 && tiny->IsPlayerTeammate()) {
			return;
		}
		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->IsPlayerTeammate() && tiny->IsPlayerTeammate()) {
			return;
		} if (Runtime::GetBool("GtsPCEffectImmunityToggle") && tiny->formID == 0x14) {
			return;
		}	
		
		log::info("Underfoot event: {} stepping on {} with force {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName(), force);

		float giantSize = get_visual_scale(giant);
		bool hasSMT = Runtime::HasMagicEffect(giant, "SmallMassiveThreat");
		if (hasSMT) {
			giantSize += 4.0;
		}

		float tinySize = get_visual_scale(tiny);

		float movementFactor = 1.0;
		
		if (giant->IsSneaking()) {
			movementFactor *= 0.5;
		}
		if (giant->IsSprinting()) {
			movementFactor *= 1.5;
		}
		if (evt.footEvent == FootEvent::JumpLand) {
			movementFactor *= 3.0;
		}

		float sizeRatio = giantSize/tinySize * movementFactor;
		float knockBack = LAUNCH_KNOCKBACK_BASE  * giantSize * movementFactor * force;
		const float UNDERFOOT_FORCE = 0.60;

		if (force > UNDERFOOT_FORCE && sizeRatio >= 2.5) { // If under the foot
			log::info("Applying Size Related Damage, Force is > 0.5");
			DoSizeRelatedDamage(giant, tiny, movementFactor, force);
			if (sizeRatio >= 4.0) {
				PushActorAway(giant, tiny, knockBack);
			}
		} else if (!SizeManager::IsLaunching(tiny) && force <= UNDERFOOT_FORCE) {
			if (Runtime::HasPerkTeam(giant, "LaunchPerk")) {
				log::info("Launch Perk is True");
				if (sizeRatio >= 8.0) {
					// Launch
					this->GetLaunchData(tiny).lastLaunchTime = Time::WorldTimeElapsed();
					if (Runtime::HasPerkTeam(giant, "LaunchDamage")) {
						float damage = LAUNCH_DAMAGE_BASE * giantSize * movementFactor * force/UNDERFOOT_FORCE;
						DamageAV(tiny,ActorValue::kHealth, damage);
						log::info("Underfoot damage: {} on {}", damage, tiny->GetDisplayFullName());
					}
					PushActorAway(giant, tiny, knockBack);
					ApplyHavokImpulse(tiny, 0, 0, 200 * movementFactor * giantSize * force, 200 * movementFactor * giantSize * force);
				}
			}
		}
	}

	void SizeManager::DoSizeRelatedDamage(Actor* giant, Actor* tiny, float totaldamage, float mult) {
		if (!SizeManager::GetSingleton().GetPreciseDamage()) {
			return;
		}
		if (!giant) {
			return;
		} if (!tiny) {
			return;
		} if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->formID == 0x14 && tiny->IsPlayerTeammate()) {
			return;
		}
		if (Runtime::GetBool("GtsNPCEffectImmunityToggle") && giant->IsPlayerTeammate() && tiny->IsPlayerTeammate()) {
			return;
		} if (Runtime::GetBool("GtsPCEffectImmunityToggle") && tiny->formID == 0x14) {
			return;
		}	

		float giantsize = get_visual_scale(giant);
		float tinysize = get_visual_scale(tiny);
		float highheels = (1.0 + HighHeelManager::GetBaseHHOffset(giant).Length()/200);
		float multiplier = giantsize/tinysize  * highheels;
		float multipliernolimit = giantsize/tinysize  * highheels;
		if (multiplier > 4.0) {
			multiplier = 4.0; // temp fix
		}
		float additionaldamage = 1.0 + this->GetSizeVulnerability(tiny);
		float normaldamage = std::clamp(this->GetSizeAttribute(giant, 0) * 0.25, 0.25, 999999.0);
		float highheelsdamage = this->GetSizeAttribute(giant, 3);
		float sprintdamage = 1.0;
		float falldamage = 1.0;
		float weightdamage = giant->GetWeight()/100 + 1.0;

		SizeModifications(giant, tiny, highheels);

		if (giant->IsSprinting()) {
			sprintdamage = 1.5 * this->GetSizeAttribute(giant, 1);
		}
		if (totaldamage >= 3.0) {
			falldamage = this->GetSizeAttribute(giant, 2) * 2.0;
		}

		float result = ((multiplier * 4 * giantsize * 9.0) * totaldamage * 0.12) * (normaldamage * sprintdamage * falldamage) * 0.38 * highheelsdamage * additionaldamage;
		if (giant->IsSneaking()) {
			result *= 0.33;
		}
		
		if (multipliernolimit >= 8.0 && (GetAV(tiny, ActorValue::kHealth) <= (result * weightdamage * mult) || tiny->IsDead() || GetAV(tiny, ActorValue::kHealth) <= 0.0)) {
			CrushManager::GetSingleton().Crush(giant, tiny);
			log::info("Trying to crush: {}, multiplier: {}", tiny->GetDisplayFullName(), multiplier);
			return;
		}
		DamageAV(tiny, ActorValue::kHealth, result * weightdamage * mult * 0.25);
	}


	void SizeManager::SetEnchantmentBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).enchantmentBonus = amt;
	}

	float SizeManager::GetEnchantmentBonus(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float EB = clamp(0.0, 100.0, this->GetData(actor).enchantmentBonus);
		return EB;
	}

	void SizeManager::ModEnchantmentBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).enchantmentBonus += amt;
	}

	//=================Size Hunger

	void SizeManager::SetSizeHungerBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).SizeHungerBonus = amt;
	}

	float SizeManager::GetSizeHungerBonus(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float SHB = clamp(0.0, 100.0, this->GetData(actor).SizeHungerBonus);
		return SHB;
	}

	void SizeManager::ModSizeHungerBonus(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).SizeHungerBonus += amt;
	}

	//==================Growth Spurt

	void SizeManager::SetGrowthSpurt(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).GrowthSpurt = amt;
	}

	float SizeManager::GetGrowthSpurt(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float GS = clamp (0.0, 999999.0, this->GetData(actor).GrowthSpurt);
		return GS;
	}

	void SizeManager::ModGrowthSpurt(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).GrowthSpurt += amt;
	}

	//================Size-Related Damage
	void SizeManager::SetSizeAttribute(Actor* actor, float amt, float attribute) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		if (attribute == 0) {
			Persistent->NormalDamage = amt;
		} else if (attribute == 1) {
			Persistent->SprintDamage = amt;
		} else if (attribute == 2) {
			Persistent->FallDamage = amt;
		} else if (attribute == 3) {
			Persistent->HHDamage = amt;
		}
	}

	float SizeManager::GetSizeAttribute(Actor* actor, float attribute) {
		if (!actor) {
			return 1.0;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return 1.0;
		}
		float Normal = clamp (1.0, 999999.0, Persistent->NormalDamage);
		float Sprint = clamp (1.0, 999999.0, Persistent->SprintDamage);
		float Fall = clamp (1.0, 999999.0, Persistent->FallDamage);
		float HH = clamp (1.0, 999999.0, Persistent->HHDamage);
		if (attribute == 0) {
			return Normal;
		} else if (attribute == 1) {
			return Sprint;
		} else if (attribute == 2) {
			return Fall;
		} else if (attribute == 3) {
			return HH;
		}
		return 1.0;
	}

	void SizeManager::ModSizeAttribute(Actor* actor, float amt, float attribute) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		if (attribute == 0) {
			Persistent->NormalDamage += amt;
		} else if (attribute == 1) {
			Persistent->SprintDamage += amt;
		} else if (attribute == 2) {
			Persistent->FallDamage += amt;
		} else if (attribute == 3) {
			Persistent->HHDamage += amt;
		}
	}
	//===============Size-Related Attribute End


	//===============Size-Vulnerability

	void SizeManager::SetSizeVulnerability(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		Persistent->SizeVulnerability = amt;
	}

	float SizeManager::GetSizeVulnerability(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return 0.0;
		}
		return clamp (0.0, 999999.0, Persistent->SizeVulnerability);
	}

	void SizeManager::ModSizeVulnerability(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		Persistent->SizeVulnerability += amt;
	}
	//===============Size-Vulnerability

	//===============Hit Growth

	float SizeManager::GetHitGrowth(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return 0.0;
		}
		return Persistent->AllowHitGrowth;
	}

	void SizeManager::SetHitGrowth(Actor* actor, float allow) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		Persistent->AllowHitGrowth = allow;
	}

	//===============Size-Vulnerability

	//===============Balance Mode
	float SizeManager::BalancedMode()
	{
		if (Runtime::GetBool("BalanceMode")) {
			//log::info("Balance Mode True");
			return 2.0;
		} else {
			//log::info("Balance Mode False");
			return 1.0;
		}
	}

	SizeManagerData& SizeManager::GetData(Actor* actor) {
		this->sizeData.try_emplace(actor);
		return this->sizeData.at(actor);
	}

	void SizeManager::Reset() {
		auto caster = PlayerCharacter::GetSingleton();
		if (caster) {
			SetEnchantmentBonus(caster, 0.0);
			SetGrowthSpurt(caster, 0.0);
		}
	}

	LaunchData& SizeManager::GetLaunchData(Actor* actor) {
		this->launchData.try_emplace(actor);
		return this->launchData.at(actor);
	}

	DamageData& SizeManager::GetDamageData(Actor* actor) {
		this->DamageData.try_emplace(actor);
		return this->DamageData.at(actor);
	}

	bool SizeManager::IsLaunching(Actor* actor) {
		return Time::WorldTimeElapsed() <= (SizeManager::GetSingleton().GetLaunchData(actor).lastLaunchTime + LAUNCH_COOLDOWN);
	}

	bool SizeManager::IsDamaging(Actor* actor) {
		return Time::WorldTimeElapsed() <= (SizeManager::GetSingleton().GetDamageData(actor).lastDamageTime + DAMAGE_COOLDOWN);
	}

	bool SizeManager::GetPreciseDamage() {
		auto result = Runtime::GetBool("PreciseDamage");
		//log::info("Value: {}", result);
		return result;
	}
}
