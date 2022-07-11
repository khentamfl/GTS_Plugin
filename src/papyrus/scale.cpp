#include "papyrus/scale.h"
#include "scale.h"
#include "persistent.h"
#include "GtsManager.h"


using namespace SKSE;
using namespace Gts;
using namespace RE;
using namespace RE::BSScript;

namespace {
	constexpr std::string_view PapyrusClass = "GtsScale";

	// Model Scale
	bool SetModelScale(StaticFunctionTag*, Actor* actor, float scale) {
		bool result = false;
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			result = set_model_scale(actor, scale);
			actor_data->visual_scale = scale;
			actor_data->visual_scale_v = 0.0;
			actor_data->target_scale = scale;
		}
		return result;
	}
	float GetModelScale(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto result = get_model_scale(actor);
		return result;
	}
	bool ModModelScale(StaticFunctionTag*, Actor* actor, float amt) {
		bool result = false;
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			auto scale = get_model_scale(actor) + amt;
			result = set_model_scale(actor, scale);
			actor_data->visual_scale = scale;
			actor_data->visual_scale_v = 0.0;
			actor_data->target_scale = scale;
		}
		return result;
	}

	// Node Scale
	bool SetNodeScale(StaticFunctionTag*, Actor* actor, float scale) {
		bool result = false;
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			result = set_npcnode_scale(actor, scale);
			actor_data->visual_scale = scale;
			actor_data->visual_scale_v = 0.0;
			actor_data->target_scale = scale;
		}
		return result;
	}
	float GetNodeScale(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto result = get_npcnode_scale(actor);
		return result;
	}
	bool ModNodeScale(StaticFunctionTag*, Actor* actor, float amt) {
		bool result = false;
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			auto scale = get_npcnode_scale(actor) + amt;
			result = set_npcnode_scale(actor, scale);
			actor_data->visual_scale = scale;
			actor_data->visual_scale_v = 0.0;
			actor_data->target_scale = scale;
		}
		return result;
	}

	// Ref scale
	bool SetRefScale(StaticFunctionTag*, Actor* actor, float scale) {
		bool result = false;
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			set_ref_scale(actor, scale);
			result = true; // Ref scale cannot fail
			actor_data->visual_scale = scale;
			actor_data->visual_scale_v = 0.0;
			actor_data->target_scale = scale;
		}
		return result;
	}
	float GetRefScale(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto result = get_ref_scale(actor);
		return result;
	}
	bool ModRefScale(StaticFunctionTag*, Actor* actor, float amt) {
		bool result = false;
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			auto scale = get_ref_scale(actor) + amt;
			set_ref_scale(actor, scale);
			result = true;
			actor_data->visual_scale = scale;
			actor_data->visual_scale_v = 0.0;
			actor_data->target_scale = scale;
		}
		return result;
	}

	// Configurable scale
	void SetScaleMethod(StaticFunctionTag*, int size_method) {
		switch (size_method) {
		case 0:
			// log::info("Scale method set to Model");
			Persistent::GetSingleton().size_method = SizeMethod::ModelScale;
			break;
		case 1:
			// log::info("Scale method set to Root");
			Persistent::GetSingleton().size_method = SizeMethod::RootScale;
			break;
		case 2:
			// log::info("Scale method set to Ref");
			Persistent::GetSingleton().size_method = SizeMethod::RefScale;
			break;
		}
	}
	int GetScaleMethod(StaticFunctionTag*) {
		switch (Persistent::GetSingleton().size_method) {
		case SizeMethod::ModelScale:
			// log::info("Scale method is Model");
			return 0;
			break;
		case SizeMethod::RootScale:
			// log::info("Scale method is Root");
			return 1;
			break;
		case SizeMethod::RefScale:
			// log::info("Scale method is Ref");
			return 2;
			break;
		}
		log::error("Scale method is invalid");
		return Persistent::GetSingleton().size_method;
	}
	bool SetScale(StaticFunctionTag*, Actor* actor, float scale) {
		bool result = false;
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			result = set_scale(actor, scale);
			actor_data->visual_scale = scale;
			actor_data->visual_scale_v = 0.0;
			actor_data->target_scale = scale;
		}
		return result;
	}
	float GetScale(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto result = get_scale(actor);
		return result;
	}
	bool ModScale(StaticFunctionTag*, Actor* actor, float amt) {
		bool result = false;
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			auto scale = get_scale(actor) + amt;
			result = set_scale(actor, scale);
			actor_data->visual_scale = scale;
			actor_data->visual_scale_v = 0.0;
			actor_data->target_scale = scale;
		}
		return result;
	}

	// Target Scales
	void SetTargetScale(StaticFunctionTag*, Actor* actor, float scale) {
		if (!actor) {
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			actor_data->target_scale = scale;
		}
	}

	float GetTargetScale(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			return actor_data->target_scale;
		}
		return 0.0;
	}

	void ModTargetScale(StaticFunctionTag*, Actor* actor, float amt) {
		log::info("+ ModTargetScale");
		if (!actor) {
			log::info("  - Actor invalid");
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			log::info("  - Data valid");
			actor_data->target_scale += amt;
		}
		log::info("- ModTargetScale");
	}

	void SetMaxScale(StaticFunctionTag*, Actor* actor, float scale) {
		if (!actor) {
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			actor_data->max_scale = scale;
		}
	}

	float GetMaxScale(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			return actor_data->max_scale;
		}
		return 0.0;
	}

	void ModMaxScale(StaticFunctionTag*, Actor* actor, float amt) {
		if (!actor) {
			return;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			actor_data->max_scale += amt;
		}
	}

	float GetVisualScale(StaticFunctionTag*, Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto actor_data = Persistent::GetSingleton().GetActorData(actor);
		if (actor_data) {
			return actor_data->visual_scale;
		}
		return 0.0;
	}

	void ModTeammateScale(StaticFunctionTag*, float amt) {
		for (auto actor: find_actors()) {
			if (!actor) {
				continue;
			}
			if (!actor->Is3DLoaded()) {
				continue;
			}
			if (actor->IsPlayerTeammate()) {
				auto actor_data = Persistent::GetSingleton().GetActorData(actor);
				if (actor_data) {
					actor_data->target_scale += amt;
				}
			}
		}
	}
}

namespace Gts {
	bool register_papyrus_scale(IVirtualMachine* vm) {
		vm->RegisterFunction("SetModelScale", PapyrusClass, SetModelScale);
		vm->RegisterFunction("GetModelScale", PapyrusClass, GetModelScale);
		vm->RegisterFunction("ModModelScale", PapyrusClass, ModModelScale);

		vm->RegisterFunction("SetNodeScale", PapyrusClass, SetNodeScale);
		vm->RegisterFunction("GetNodeScale", PapyrusClass, GetNodeScale);
		vm->RegisterFunction("ModNodeScale", PapyrusClass, ModNodeScale);

		vm->RegisterFunction("SetRefScale", PapyrusClass, SetRefScale);
		vm->RegisterFunction("GetRefScale", PapyrusClass, GetRefScale);
		vm->RegisterFunction("ModRefScale", PapyrusClass, ModRefScale);

		vm->RegisterFunction("SetScaleMethod", PapyrusClass, SetScaleMethod);
		vm->RegisterFunction("GetScaleMethod", PapyrusClass, GetScaleMethod);
		vm->RegisterFunction("SetScale", PapyrusClass, SetScale);
		vm->RegisterFunction("GetScale", PapyrusClass, GetScale);
		vm->RegisterFunction("ModScale", PapyrusClass, ModScale);

		vm->RegisterFunction("SetTargetScale", PapyrusClass, SetTargetScale);
		vm->RegisterFunction("GetTargetScale", PapyrusClass, GetTargetScale);
		vm->RegisterFunction("ModTargetScale", PapyrusClass, ModTargetScale);

		vm->RegisterFunction("SetMaxScale", PapyrusClass, SetMaxScale);
		vm->RegisterFunction("GetMaxScale", PapyrusClass, GetMaxScale);
		vm->RegisterFunction("ModMaxScale", PapyrusClass, ModMaxScale);

		vm->RegisterFunction("GetVisualScale", PapyrusClass, GetVisualScale);

		vm->RegisterFunction("ModTeammateScale", PapyrusClass, ModTeammateScale);

		return true;
	}
}
