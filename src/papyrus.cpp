#include "papyrus.h"
#include "scale.h"
#include "persistent.h"
#include "GtsManager.h"
#include "papyrus/scale.h"
#include "papyrus/events.h"
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
		if (actor) {
			auto camera = PlayerCamera::GetSingleton();
			if (camera) {
				auto point_a = actor->GetPosition();
				auto point_b = camera->pos;
				auto delta = point_a - point_b;
				return delta.Length();
			}
		}
		return 3.4028237E38; // Max float
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

	void EnableHighHeelCorrection(StaticFunctionTag*, bool enabled) {
		Persistent::GetSingleton().highheel_correction = enabled;
	}
}

namespace Gts {
	bool register_papyrus(IVirtualMachine* vm) {
		vm->RegisterFunction("GetDistanceToCamera", PapyrusClass, GetDistanceToCamera);
		vm->RegisterFunction("SetGrowthHalfLife", PapyrusClass, SetGrowthHalfLife);
		vm->RegisterFunction("SetAnimSpeed", PapyrusClass, SetAnimSpeed);
		vm->RegisterFunction("SigFig", PapyrusClass, SigFig);
		vm->RegisterFunction("EnableHighHeelCorrection", PapyrusClass, EnableHighHeelCorrection);
		register_papyrus_scale(vm);
		register_papyrus_events(vm);
		return true;
	}
}
