#include "managers/GtsSizeManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/GtsManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/highheel.hpp"
#include "managers/InputManager.hpp"
#include "managers/Rumble.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "timer.hpp"
#include "node.hpp"

#include <random>


using namespace Gts;
using namespace RE;
using namespace REL;
using namespace SKSE;

namespace {
	const double LAUNCH_COOLDOWN = 3.0;
	const double DAMAGE_COOLDOWN = 1.0;
	const double HANDDAMAGE_COOLDOWN = 1.0;
	const float LAUNCH_DAMAGE_BASE = 1.0f;
	const float LAUNCH_KNOCKBACK_BASE = 0.02f;

	void TotalControlGrowEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (Runtime::HasPerk(player, "TotalControl")) {
			float scale = get_visual_scale(player);
			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(player));
			DamageAV(player, ActorValue::kStamina, 0.15 * (scale * 0.5 + 0.5) * stamina * TimeScale());
			Grow(player, 0.0010 * stamina, 0.0);
			float Volume = clamp(0.20, 2.0, get_visual_scale(player)/16);
			GRumble::Once("TotalControl", player, scale/10, 0.05);
			static Timer timergrowth = Timer(2.00);
			if (timergrowth.ShouldRun()) {
				Runtime::PlaySoundAtNode("growthSound", player, Volume, 1.0, "NPC Pelvis [Pelv]");
			}
		}
	}
	void TotalControlShrinkEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (Runtime::HasPerk(player, "TotalControl")) {
			float scale = get_visual_scale(player);
			float stamina = clamp(0.05, 1.0, GetStaminaPercentage(player));
			
			if (get_target_scale(player) > 0.12) {
				DamageAV(player, ActorValue::kStamina, 0.10 * (scale * 0.5 + 0.5) * stamina * TimeScale());
				ShrinkActor(player, 0.0010 * stamina, 0.0);
			} else {
				set_target_scale(player, 0.12);
			}
			
			float Volume = clamp(0.05, 2.0, get_visual_scale(player)/16);
			GRumble::Once("TotalControl", player, scale/14, 0.05);
			static Timer timergrowth = Timer(2.00);
			if (timergrowth.ShouldRun()) {
				Runtime::PlaySound("shrinkSound", player, Volume, 0.0);
			}
		}
	}
	void TotalControlGrowOtherEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (Runtime::HasPerk(player, "TotalControl")) {
			for (auto actor: find_actors()) {
				if (!actor) {
					continue;
				}
				if (actor->formID != 0x14 && (actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction"))) {
					float npcscale = get_visual_scale(actor);
					float magicka = clamp(0.05, 1.0, GetMagikaPercentage(player));
					DamageAV(player, ActorValue::kMagicka, 0.15 * (npcscale * 0.5 + 0.5) * magicka * TimeScale());
					Grow(actor, 0.0010 * magicka, 0.0);
					float Volume = clamp(0.20, 2.0, get_visual_scale(actor)/16);
					GRumble::Once("TotalControlOther", actor, 0.25, 0.05);
					static Timer timergrowth = Timer(2.00);
					if (timergrowth.ShouldRun()) {
						Runtime::PlaySoundAtNode("growthSound", actor, Volume, 1.0, "NPC Pelvis [Pelv]");
					}
				}
			}
		}
	}
	void TotalControlShrinkOtherEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (Runtime::HasPerk(player, "TotalControl")) {
			for (auto actor: find_actors()) {
				if (!actor) {
					continue;
				}
				if (actor->formID != 0x14 && (actor->IsPlayerTeammate() || Runtime::InFaction(actor, "FollowerFaction"))) {
					float npcscale = get_visual_scale(actor);
					float magicka = clamp(0.05, 1.0, GetMagikaPercentage(player));
					DamageAV(player, ActorValue::kMagicka, 0.10 * (npcscale * 0.5 + 0.5) * magicka * TimeScale());
					ShrinkActor(actor, 0.0010 * magicka, 0.0);
					float Volume = clamp(0.05, 2.0, get_visual_scale(actor)/16);
					GRumble::Once("TotalControlOther", actor, 0.20, 0.05);
					static Timer timergrowth = Timer(2.00);
					if (timergrowth.ShouldRun()) {
						Runtime::PlaySound("shrinkSound", actor, Volume, 0.0);
					}
				}
			}
		}
	}
}

namespace Gts {
	SizeManager& SizeManager::GetSingleton() noexcept {
		static SizeManager instance;
		return instance;
	}

	std::string SizeManager::DebugName() {
		return "SizeManager";
	}

	void SizeManager::DataReady() {
		InputManager::RegisterInputEvent("TotalControlGrow", TotalControlGrowEvent);
		InputManager::RegisterInputEvent("TotalControlShrink", TotalControlShrinkEvent);
		InputManager::RegisterInputEvent("TotalControlGrowOther", TotalControlGrowOtherEvent);
		InputManager::RegisterInputEvent("TotalControlShrinkOther", TotalControlShrinkOtherEvent);
	}

	void SizeManager::Update() {
		auto profiler = Profilers::Profile("SizeManager: Update");
		for (auto actor: find_actors()) {
			// TODO move away from polling
			float Endless = 0.0;
			if (actor->formID == 0x14 && Runtime::HasPerk(actor, "TotalControl")) {
				Endless = 999999.0;
			}
			float NaturalScale = get_neutral_scale(actor);
			float QuestStage = Runtime::GetStage("MainQuest");
			float Gigantism = this->GetEnchantmentBonus(actor)/100;
			float GetLimit = clamp(NaturalScale, 99999999.0, NaturalScale + ((Runtime::GetFloat("sizeLimit") - 1.0) * NaturalScale)); // Default size limit
			auto Persistent = Persistent::GetSingleton().GetData(actor);
			float Persistent_Size = 0.0;
			if (Persistent) {
				Persistent_Size = Persistent->bonus_max_size;
			}
			float SelectedFormula = Runtime::GetInt("SelectedSizeFormula");

			float FollowerLimit = Runtime::GetFloat("FollowersSizeLimit"); // 0 by default
			float NPCLimit = Runtime::GetFloat("NPCSizeLimit"); // 0 by default

			if (SelectedFormula >= 1.0 && actor->formID == 0x14) { // Apply Player Mass-Based max size
				GetLimit = clamp(NaturalScale, 99999999.0, NaturalScale + (Runtime::GetFloat("GtsMassBasedSize") * NaturalScale));
			} else if (QuestStage > 100 && FollowerLimit > 0.0 && FollowerLimit != 1.0 && actor->formID != 0x14 && IsTeammate(actor)) { // Apply Follower Max Size
				GetLimit = clamp(NaturalScale * FollowerLimit, 99999999.0, NaturalScale + ((Runtime::GetFloat("FollowersSizeLimit") - 1.0) * NaturalScale)); // Apply only if Quest is done.
			} else if (QuestStage > 100 && NPCLimit > 0.0 && NPCLimit != 1.0 && actor->formID != 0x14 && !IsTeammate(actor)) { // Apply Other NPC's max size
				GetLimit = clamp(NaturalScale * NPCLimit, 99999999.0, NaturalScale + ((Runtime::GetFloat("NPCSizeLimit") - 1.0) * NaturalScale));       // Apply only if Quest is done.
			}

			float TotalLimit = ((GetLimit + Persistent_Size) * (1.0 + Gigantism));

			if (get_max_scale(actor) < TotalLimit + Endless || get_max_scale(actor) > TotalLimit + Endless) {
				set_max_scale(actor, TotalLimit);
			}
		}
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
		float EB = clamp(0.0, 10000.0, this->GetData(actor).enchantmentBonus);
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
		this->GetData(actor).GrowthSpurtSize = amt;
	}

	float SizeManager::GetGrowthSpurt(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float GS = clamp (0.0, 999999.0, this->GetData(actor).GrowthSpurtSize);
		return GS;
	}

	void SizeManager::ModGrowthSpurt(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		this->GetData(actor).GrowthSpurtSize += amt;
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
			return GetHighHeelsBonusDamage(actor);
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
		return clamp(0.0, 3.0, Persistent->SizeVulnerability);
	}

	void SizeManager::ModSizeVulnerability(Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto Persistent = Persistent::GetSingleton().GetData(actor);
		if (!Persistent) {
			return;
		}
		if (Persistent->SizeVulnerability < 3) {
			Persistent->SizeVulnerability += amt;
		}
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

	//===============Bools
	void SizeManager::SetActionBool(Actor* actor, bool enable, float type) {
		SetCameraOverride(actor, enable);
		if (type == 0.0) {
			this->GetData(actor).IsThighCrushing = enable;
		} else if (type == 1.0) {
			this->GetData(actor).IsThighSandwiching = enable;
		} else if (type == 2.0) {
			this->GetData(actor).IsVoring = enable;
		} else if (type == 3.0) {
			this->GetData(actor).AlterSandwichCamera = enable;
		} else if (type == 4.0) {
			this->GetData(actor).ShouldTrackHand = enable;
		} else if (type == 5.0) {
			this->GetData(actor).TrackLeftFeet = enable;
		} else if (type == 6.0) {
			this->GetData(actor).TrackRightFeet = enable;
		} else if (type == 7.0) {
			this->GetData(actor).TrackLeftHand = enable;
		} else if (type == 8.0) {
			this->GetData(actor).TrackButt = enable;
		} else if (type == 9.0) {
			this->GetData(actor).TrackBreasts = enable;
		}
	}
	bool SizeManager::GetActionBool(Actor* actor, float type) {
		if (type == 0.0) {
			return this->GetData(actor).IsThighCrushing;
		} else if (type == 1.0) {
			return this->GetData(actor).IsThighSandwiching;
		} else if (type == 2.0) {
			return this->GetData(actor).IsVoring;
		} else if (type == 3.0) {
			return this->GetData(actor).AlterSandwichCamera;
		} else if (type == 4.0) {
			return this->GetData(actor).ShouldTrackHand;
		} else if (type == 5.0) {
			return this->GetData(actor).TrackLeftFeet;
		} else if (type == 6.0) {
			return this->GetData(actor).TrackRightFeet;
		} else if (type == 7.0) {
			return this->GetData(actor).TrackLeftHand;
		} else if (type == 8.0) {
			return this->GetData(actor).TrackButt;
		} else if (type == 9.0) {
			return this->GetData(actor).TrackBreasts;
		}
		return false;
	}
	//

	//===============Balance Mode
	float SizeManager::BalancedMode()
	{
		if (Runtime::GetBool("BalanceMode")) {
			return 2.0;
		} else {
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

	bool SizeManager::IsHandDamaging(Actor* actor) {
		return Time::WorldTimeElapsed() <= (SizeManager::GetSingleton().GetDamageData(actor).lastHandDamageTime + HANDDAMAGE_COOLDOWN);
	}

	bool SizeManager::GetPreciseDamage() {
		auto result = Runtime::GetBool("PreciseDamage");
		return result;
	}
}