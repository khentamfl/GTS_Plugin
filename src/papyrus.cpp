#include "papyrus.h"
#include "scale.h"
#include "persistent.h"
#include "GtsManager.h"
#include "papyrus/scale.h"

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
}

namespace Gts {
	bool register_papyrus(IVirtualMachine* vm) {
		vm->RegisterFunction("GetDistanceToCamera", PapyrusClass, GetDistanceToCamera);

		register_papyrus_scale(vm);
		return true;
	}
}
