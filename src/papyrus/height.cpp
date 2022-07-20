#include "papyrus/height.h"
#include "scale.h"
#include "persistent.h"
#include "transient.h"
#include "GtsManager.h"


using namespace SKSE;
using namespace Gts;
using namespace RE;
using namespace RE::BSScript;

namespace {
	constexpr std::string_view PapyrusClass = "GtsHeight";

	// Target Scales
	void SetTargetHeight(StaticFunctionTag*, Actor* actor, float height) {
		if (!actor) {
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return;
		}
		float scale = height / temp_actor_data->base_height;
		if (actor_data) {
			actor_data->target_scale = scale;
		}
	}

	float GetTargetHeight(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return 0.0;
		}
		float scale = actor_data->target_scale;
		if (actor_data) {
			return scale * temp_actor_data->base_height;
		}
		return 0.0;
	}

	void ModTargetHeight(StaticFunctionTag*, Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return;
		}
		float current_scale = actor_data->target_scale;
		float current_height = current_scale * temp_actor_data->base_height;
		float target_height = (current_height + amt);
		float target_scale = target_height / temp_actor_data->base_height;
		float scale_delta = target_scale - current_scale;

		if (actor_data) {
			actor_data->target_scale += scale_delta;
		}
	}

	void SetMaxHeight(StaticFunctionTag*, Actor* actor, float height) {
		if (!actor) {
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return;
		}
		float scale = height / temp_actor_data->base_height;
		if (actor_data) {
			actor_data->max_scale = scale;
		}
	}

	float GetMaxHeight(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return 0.0;
		}
		float scale = actor_data->max_scale;
		if (actor_data) {
			return scale * temp_actor_data->base_height;
		}
		return 0.0;
	}

	void ModMaxHeight(StaticFunctionTag*, Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return;
		}
		float current_scale = actor_data->max_scale;
		float current_height = current_scale * temp_actor_data->base_height;
		float target_height = (current_height + amt);
		float target_scale = target_height / temp_actor_data->base_height;
		float scale_delta = target_scale - current_scale;

		if (actor_data) {
			actor_data->max_scale += scale_delta;
		}
	}

	float GetVisualHeight(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
		if (!temp_actor_data) {
			return 0.0;
		}
		float scale = actor_data->visual_scale;
		if (actor_data) {
			return scale * temp_actor_data->base_height;
		}
		return 0.0;
	}

	void ModTeammateHeight(StaticFunctionTag*, float amt) {
		for (auto actor: find_actors()) {
			if (!actor) {
				continue;
			}
			if (!actor->Is3DLoaded()) {
				continue;
			}
			if (actor->IsPlayerTeammate()) {
				auto actor_data = Persistent::GetSingleton().GetActorData(actor);
				auto temp_actor_data = Transient::GetSingleton().GetActorData(actor);
				if (!temp_actor_data) {
					continue;
				}
				float current_scale = actor_data->target_scale;
				float current_height = current_scale * temp_actor_data->base_height;
				float target_height = (current_height + amt);
				float target_scale = target_height / temp_actor_data->base_height;
				float scale_delta = target_scale - current_scale;

				if (actor_data) {
					actor_data->target_scale += scale_delta;
				}
			}
		}
	}
}

namespace Gts {
	bool register_papyrus_height(IVirtualMachine* vm) {
		vm->RegisterFunction("SetTargetHeight", PapyrusClass, SetTargetScale);
		vm->RegisterFunction("GetTargetHeight", PapyrusClass, GetTargetScale);
		vm->RegisterFunction("ModTargetHeight", PapyrusClass, ModTargetScale);

		vm->RegisterFunction("SetMaxHeight", PapyrusClass, SetMaxScale);
		vm->RegisterFunction("GetMaxHeight", PapyrusClass, GetMaxScale);
		vm->RegisterFunction("ModMaxHeight", PapyrusClass, ModMaxScale);

		vm->RegisterFunction("GetVisualHeight", PapyrusClass, GetVisualScale);

		vm->RegisterFunction("ModTeammateHeight", PapyrusClass, ModTeammateScale);

		return true;
	}
}
