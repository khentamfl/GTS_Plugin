#include "managers/GrowthTremorManager.hpp"
#include "managers/GtsSizeManager.hpp"
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
	SizeManager& SizeManager::GetSingleton() noexcept {
		static SizeManager instance;
		return instance;
	}

	inline float SizeManager::GetRaceScale(Actor* actor) {
		auto GetNode = find_node(actor, "NPC", false);
		float NodeScale = GetNode->world.scale;
		return GetNode ? NodeScale : 1; // <- not used, causes troubles with quest progression. (Can't reach 1.44 for example when 1.50 is needed.)
	}

	std::string SizeManager::DebugName() {
		return "SizeManager";
	}

	void SizeManager::Update() {
		for (auto actor: find_actors()) {

			bool Balance = false; // Toggles Balance Mode for the mod. False = off, true = on.

			float Gigantism = this->GetEnchantmentBonus(actor)/100;
			float GetLimit = clamp(1.0, 99999999.0, Runtime::GetFloat("sizeLimit")); // Default size limit
			float Persistent_Size = Persistent::GetSingleton().GetData(actor)->bonus_max_size;
			float SelectedFormula = Runtime::GetInt("SelectedSizeFormula");

			if (SelectedFormula >= 2.0 && actor->formID == 0x14) { // Apply Player Mass-Based max size
				GetLimit = Runtime::GetFloat("MassBasedSizeLimit") +1.0;
			}
			else if (actor->formID != 0x14 && (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate())) { // Apply Follower Max Size
				GetLimit = Runtime::GetFloat("FollowersSizeLimit") + 1.0;
			}
			else if (actor->formID != 0x14 && (!Runtime::InFaction(actor, "FollowerFaction") && !actor->IsPlayerTeammate())) {  // Apply Other NPC's max size
				GetLimit = Runtime::GetFloat("NPCSizeLimit") + 1.0;
			}

			float RaceScale = (GetRaceScale(actor) * (GetLimit + Persistent_Size)) * (1.0 + Gigantism);
			float TotalLimit = (GetLimit + Persistent_Size) * (1.0 + Gigantism);

			if (TotalLimit < 1.0) {
				TotalLimit = 1.0;
			}
			if (get_max_scale(actor) < TotalLimit || get_max_scale(actor) > TotalLimit) {
				set_max_scale(actor, TotalLimit);
				//log::info("Current Size Limit of: {} is {}", actor->GetDisplayFullName(), get_max_scale(actor));
			}
		}
	}
	void SizeManager::SetEnchantmentBonus(Actor* actor, float amt) {
		this->GetData(actor).enchantmentBonus = amt;
	}

	float SizeManager::GetEnchantmentBonus(Actor* actor) {
		float EB = clamp(0.0, 100.0, this->GetData(actor).enchantmentBonus);
		return EB;
	}

	void SizeManager::ModEnchantmentBonus(Actor* actor, float amt) {
		this->GetData(actor).enchantmentBonus += amt;
	}

	//=================Size Hunger

	void SizeManager::SetSizeHungerBonus(Actor* actor, float amt) {
		this->GetData(actor).SizeHungerBonus = amt;
	}

	float SizeManager::GetSizeHungerBonus(Actor* actor) {
		float SHB = clamp(0.0, 100.0, this->GetData(actor).SizeHungerBonus);
		return SHB;
	}

	void SizeManager::ModSizeHungerBonus(Actor* actor, float amt) {
		this->GetData(actor).SizeHungerBonus += amt;
	}

	//==================Growth Spurt

	void SizeManager::SetGrowthSpurt(Actor* actor, float amt) {
		this->GetData(actor).GrowthSpurt = amt;
	}

	float SizeManager::GetGrowthSpurt(Actor* actor) {
		float GS = clamp (0.0, 999999.0, this->GetData(actor).GrowthSpurt);
		return GS;
	}

	void SizeManager::ModGrowthSpurt(Actor* actor, float amt) {
		this->GetData(actor).GrowthSpurt += amt;
	}


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
		SetEnchantmentBonus(caster, 0.0);
		SetGrowthSpurt(caster, 0.0);
		//log::info("Resetting Enchantment and Growth Spurt");
	}
}
