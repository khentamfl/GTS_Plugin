#include "papyrus/plugin.h"
#include "data/persistent.h"
#include "managers/GtsManager.h"
#include "util.h"
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

	bool SetGrowthHalfLife(StaticFunctionTag*, Actor* actor, float halflife) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetActorData(actor);
			if (actor_data) {
				actor_data->half_life = halflife;
				return true;
			}
		}
		return false;
	}

	float GetGrowthHalfLife(StaticFunctionTag*, Actor* actor) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetActorData(actor);
			if (actor_data) {
				return actor_data->half_life;
			}
		}
		return 0.05;
	}

	bool SetAnimSpeed(StaticFunctionTag*, Actor* actor, float animspeed) {
		if (actor) {
			auto actor_data = Persistent::GetSingleton().GetActorData(actor);
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

	bool GetIsHighHeelEnabled(StaticFunctionTag*) {
		return Persistent::GetSingleton().highheel_correction;
	}

	void SetIsHighHeelEnabled(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().highheel_correction = enabled;
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
		if (!actor) return false;
		return actor->IsInMidair();
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
		vm->RegisterFunction("SetGrowthHalfLife", PapyrusClass, SetGrowthHalfLife);
		vm->RegisterFunction("GetGrowthHalfLife", PapyrusClass, GetGrowthHalfLife);
		vm->RegisterFunction("SetAnimSpeed", PapyrusClass, SetAnimSpeed);
		vm->RegisterFunction("SigFig", PapyrusClass, SigFig);
		vm->RegisterFunction("GetIsHighHeelEnabled", PapyrusClass, GetIsHighHeelEnabled);
		vm->RegisterFunction("SetIsHighHeelEnabled", PapyrusClass, SetIsHighHeelEnabled);
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
		vm->RegisterFunction("GetExperimentFloat", PapyrusClass, GetExperimentFloat);
		vm->RegisterFunction("SetExperimentFloat", PapyrusClass, SetExperimentFloat);

		return true;
	}
}
