#include "papyrus/plugin.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Attributes.hpp"
#include "managers/GtsManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include <math.h>
#include <sstream>
#include <iomanip>


using namespace SKSE;
using namespace Gts;
using namespace RE;
using namespace RE::BSScript;

namespace {
	constexpr std::string_view PapyrusClass = "GtsPlugin";
	float GetDistanceToCamera(StaticFunctionTag*, Actor* actor) {
		return get_distance_to_camera(actor);
	}

	float GetSizeRelatedDamage(StaticFunctionTag*, Actor* actor, float attribute) {
		return SizeManager::GetSingleton().GetSizeAttribute(actor, attribute);
	}
	float GetSizeVulnerability(StaticFunctionTag*, Actor* actor) {
		return SizeManager::GetSingleton().GetSizeVulnerability(actor);
	}

	float GetAttributeBonus(StaticFunctionTag*, Actor* actor, float value) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (!actor) {
			return 1.0;
		}
		if (!transient) {
			return 1.0;
		}
		if (value == 1.0) {
			return AttributeManager::GetSingleton().GetAttributeBonus(actor, ActorValue::kHealth); // Health
		}
		if (value == 2.0) {
			return AttributeManager::GetSingleton().GetAttributeBonus(actor, ActorValue::kCarryWeight); // Carry Weight
		}
		if (value == 3.0) {
			return AttributeManager::GetSingleton().GetAttributeBonus(actor, ActorValue::kSpeedMult) - 1.0; // Speed Multi
		}
		if (value == 4.0) {
			return AttributeManager::GetSingleton().GetAttributeBonus(actor, ActorValue::kAttackDamageMult) - 1.0;
		}
		if (value == 5.0) {
			return AttributeManager::GetSingleton().GetAttributeBonus(actor, ActorValue::kJumpingBonus) - 1.0;
		}
		return 1.0;
	}

	float GetFlatAttributeBonus(StaticFunctionTag*, Actor* actor, float value) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (!actor) {
			return 0.0;
		}
		if (!transient) {
			return 0.0;
		}
		if (value == 1.0) { //get hp
			return transient->health_boost;
		}
		if (value == 2.0) { // get carry weight
			return transient->carryweight_boost;
		}
		return 0.0;
	}

	bool ModSizeVulnerability(StaticFunctionTag*, Actor* actor, float amt) {
		if (actor) {
			SizeManager::GetSingleton().ModSizeVulnerability(actor, amt);
			return true;
		}
		return false;
	}

	float GetHitGrowth(StaticFunctionTag*, Actor* actor) {
		return SizeManager::GetSingleton().GetHitGrowth(actor);
	}
	bool SetHitGrowth(StaticFunctionTag*, Actor* actor, float allow) {
		if (actor) {
			SizeManager::GetSingleton().SetHitGrowth(actor, allow);
			return true;
		}
		return false;
	}

	bool SetGrowthHalfLife(StaticFunctionTag*, Actor* actor, float halflife) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				actor_data->half_life = halflife;
				return true;
			}
		}
		return false;
	}

	float GetGrowthHalfLife(StaticFunctionTag*, Actor* actor) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				return actor_data->half_life;
			}
		}
		return 0.05;
	}

	bool SetAnimSpeed(StaticFunctionTag*, Actor* actor, float animspeed) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetData(actor);
			if (actor_data) {
				actor_data->anim_speed = animspeed;
				return true;
			}
		}
		return false;
	}

	// From https://stackoverflow.com/questions/17211122/formatting-n-significant-digits-in-c-without-scientific-notation
	std::string format(double f, int n)
	{
		if (f == 0) {
			return "0";
		}
		int d = (int)::ceil(::log10(f < 0 ? -f : f)); /*digits before decimal point*/
		double order = ::pow(10., n - d);
		std::stringstream ss;
		ss << std::fixed << std::setprecision(std::max(n - d, 0)) << round(f * order) / order;
		return ss.str();
	}

	std::string SigFig(StaticFunctionTag*, float number, int sf) {
		return format(number, sf);
	}
	
	void SetFeetTracking(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().allow_feetracking = enabled;
	}

	bool GetIsHighHeelEnabled(StaticFunctionTag*) {
		return Persistent::GetSingleton().highheel_correction;
	}

	void SetIsHighHeelEnabled(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().highheel_correction = enabled;
	}

	void SetAllowPlayerVore(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().vore_allowplayervore = enabled;
	}

	void SetOnlyCombatVore(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().vore_combatonly = enabled;
	}

	void SetIsHHFurnitureEnabled(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().highheel_furniture = enabled;
		if (enabled == false) {
			auto actors = find_actors();
			for (auto actor: actors) {
				if (!actor) {
					return;
				}
				for (bool person: {false, true}) {
					auto npc_root_node = find_node(actor, "NPC", person);
					if (npc_root_node && actor->GetOccupiedFurniture()) {
						npc_root_node->local.translate.z = 0.0;
						update_node(npc_root_node);
					}
				}
			}
		}
	}

	void DisintegrateTarget(StaticFunctionTag*, Actor* actor) {
		if (actor) {
			Disintegrate(actor);
		}
	}

	bool WasDragonEaten(StaticFunctionTag*) {
		auto pc = PlayerCharacter::GetSingleton();
		auto transient = Transient::GetSingleton().GetData(pc);
		if (transient) {
			return transient->dragon_was_eaten;
		}
		return false;
	}

	bool GetDevourmentCompatibility(StaticFunctionTag*) {
		return Persistent::GetSingleton().devourment_compatibility;
	}

	void SetDevourmentCompatibility(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().devourment_compatibility = enabled;
	}

	bool DragonCheck(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return false;
		}
		return IsDragon(actor);
	}

	bool GetPreciseDamage(StaticFunctionTag*) {
		return SizeManager::GetSingleton().GetPreciseDamage();
	}

	bool GetIsSpeedAdjusted(StaticFunctionTag*) {
		return Persistent::GetSingleton().is_speed_adjusted;
	}

	void SetIsSpeedAdjusted(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().is_speed_adjusted = enabled;
	}

	void SetSpeedParameterK(StaticFunctionTag*, float k) {
		Persistent::GetSingleton().speed_adjustment.k = k;
	}
	float GetSpeedParameterK(StaticFunctionTag*) {
		return Persistent::GetSingleton().speed_adjustment.k;
	}
	void SetSpeedParameterN(StaticFunctionTag*, float n) {
		Persistent::GetSingleton().speed_adjustment.n = n;
	}
	float GetSpeedParameterN(StaticFunctionTag*) {
		return Persistent::GetSingleton().speed_adjustment.n;
	}
	void SetSpeedParameterS(StaticFunctionTag*, float s) {
		Persistent::GetSingleton().speed_adjustment.s = s;
	}
	float GetSpeedParameterS(StaticFunctionTag*) {
		return Persistent::GetSingleton().speed_adjustment.s;
	}

	bool IsJumping(StaticFunctionTag*, Actor* actor) {
		return Gts::IsJumping(actor);
	}

	bool IsInAir(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return false;
		}
		return actor->IsInMidair();
	}

	float GetTremorScale(StaticFunctionTag*) {
		return Persistent::GetSingleton().tremor_scale;
	}

	void SetTremorScale(StaticFunctionTag*, float value) {
		Persistent::GetSingleton().tremor_scale = value;
	}

	float GetTremorScaleNPC(StaticFunctionTag*) {
		return Persistent::GetSingleton().npc_tremor_scale;
	}

	void SetTremorScaleNPC(StaticFunctionTag*, float value) {
		Persistent::GetSingleton().npc_tremor_scale = value;
	}

	float GetExperimentFloat(StaticFunctionTag*) {
		return GtsManager::GetSingleton().experiment;
	}

	void SetExperimentFloat(StaticFunctionTag*, float value) {
		GtsManager::GetSingleton().experiment = value;
	}
}

namespace Gts {
	bool register_papyrus_plugin(IVirtualMachine* vm) {
		vm->RegisterFunction("GetDistanceToCamera", PapyrusClass, GetDistanceToCamera);
		vm->RegisterFunction("GetSizeRelatedDamage", PapyrusClass, GetSizeRelatedDamage);
		vm->RegisterFunction("ModSizeVulnerability", PapyrusClass, ModSizeVulnerability);
		vm->RegisterFunction("GetSizeVulnerability", PapyrusClass, GetSizeVulnerability);
		vm->RegisterFunction("GetAttributeBonus", PapyrusClass, GetAttributeBonus);
		vm->RegisterFunction("GetFlatAttributeBonus", PapyrusClass, GetFlatAttributeBonus);
		vm->RegisterFunction("GetHitGrowth", PapyrusClass, GetHitGrowth);
		vm->RegisterFunction("GetPreciseDamage", PapyrusClass, GetPreciseDamage);
		vm->RegisterFunction("SetHitGrowth", PapyrusClass, SetHitGrowth);
		vm->RegisterFunction("SetGrowthHalfLife", PapyrusClass, SetGrowthHalfLife);
		vm->RegisterFunction("GetGrowthHalfLife", PapyrusClass, GetGrowthHalfLife);
		vm->RegisterFunction("SetAnimSpeed", PapyrusClass, SetAnimSpeed);
		vm->RegisterFunction("SigFig", PapyrusClass, SigFig);
		vm->RegisterFunction("GetIsHighHeelEnabled", PapyrusClass, GetIsHighHeelEnabled);
		vm->RegisterFunction("SetFeetTracking", PapyrusClass, SetFeetTracking);
		vm->RegisterFunction("SetIsHighHeelEnabled", PapyrusClass, SetIsHighHeelEnabled);
		vm->RegisterFunction("SetIsHHFurnitureEnabled", PapyrusClass, SetIsHHFurnitureEnabled);
		vm->RegisterFunction("SetAllowPlayerVore", PapyrusClass, SetAllowPlayerVore);
		vm->RegisterFunction("SetOnlyCombatVore", PapyrusClass, SetOnlyCombatVore);
		vm->RegisterFunction("DisintegrateTarget", PapyrusClass, DisintegrateTarget);
		vm->RegisterFunction("WasDragonEaten", PapyrusClass, WasDragonEaten);
		vm->RegisterFunction("GetDevourmentCompatibility", PapyrusClass, GetDevourmentCompatibility);
		vm->RegisterFunction("SetDevourmentCompatibility", PapyrusClass, SetDevourmentCompatibility);
		vm->RegisterFunction("DragonCheck", PapyrusClass, DragonCheck);
		vm->RegisterFunction("GetIsSpeedAdjusted", PapyrusClass, GetIsSpeedAdjusted);
		vm->RegisterFunction("SetIsSpeedAdjusted", PapyrusClass, SetIsSpeedAdjusted);
		vm->RegisterFunction("SetSpeedParameterK", PapyrusClass, SetSpeedParameterK);
		vm->RegisterFunction("GetSpeedParameterK", PapyrusClass, GetSpeedParameterK);
		vm->RegisterFunction("SetSpeedParameterN", PapyrusClass, SetSpeedParameterN);
		vm->RegisterFunction("GetSpeedParameterN", PapyrusClass, GetSpeedParameterN);
		vm->RegisterFunction("SetSpeedParameterS", PapyrusClass, SetSpeedParameterS);
		vm->RegisterFunction("GetSpeedParameterS", PapyrusClass, GetSpeedParameterS);
		vm->RegisterFunction("IsJumping", PapyrusClass, IsJumping);
		vm->RegisterFunction("IsInAir", PapyrusClass, IsInAir);
		vm->RegisterFunction("GetTremorScale", PapyrusClass, GetTremorScale);
		vm->RegisterFunction("SetTremorScale", PapyrusClass, SetTremorScale);
		vm->RegisterFunction("GetTremorScaleNPC", PapyrusClass, GetTremorScaleNPC);
		vm->RegisterFunction("SetTremorScaleNPC", PapyrusClass, SetTremorScaleNPC);
		vm->RegisterFunction("GetExperimentFloat", PapyrusClass, GetExperimentFloat);
		vm->RegisterFunction("SetExperimentFloat", PapyrusClass, SetExperimentFloat);

		return true;
	}
}
