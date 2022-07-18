#include "hooks/character.h"
#include "util.h"
#include "GtsManager.h"
#include "persistent.h"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace {
	float get_delta(Actor* actor) {
		float delta = 0.0;
		actor->GetGraphVariableFloat("TimeDelta", delta);
		return delta;
	}

	void experiment(Actor* actor) {
		float current_delta = *g_delta_time;
		if (current_delta > 1e-5) {
			log::info("Global Time Delta: {}", current_delta);
			actor->UpdateAnimation(-0.85*current_delta);
		}
	}
}

namespace Hooks
{
	void Hook_Character::Hook() {
		logger::info("Hooking Character");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Character[0] };

		_Update = ActorVtbl.write_vfunc(0xAD, Update);
		// _UpdateNonRenderSafe = ActorVtbl.write_vfunc(0xB1, UpdateNonRenderSafe);
		// _UpdateAnimation = ActorVtbl.write_vfunc(0x7D, UpdateAnimation);
		// _UpdateNoAI = ActorVtbl.write_vfunc(0xAE, UpdateNoAI);
		// _ModifyMovementData = ActorVtbl.write_vfunc(0x11A, ModifyMovementData);
		// _ProcessTracking = ActorVtbl.write_vfunc(0x112, ProcessTracking);
	}

	void Hook_Character::Update(RE::Character* a_this, float a_delta) {
		float before = get_delta(a_this);
		_Update(a_this, a_delta);
		float delta = get_delta(a_this) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in Update");
		}
		experiment(a_this);
	}

	void Hook_Character::UpdateNonRenderSafe(RE::Character* a_this, float a_delta) {
		float before = get_delta(a_this);
		_UpdateNonRenderSafe(a_this, a_delta);
		float delta = get_delta(a_this) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in UpdateNonRenderSafe");
		}
		experiment(a_this);
	}

	void Hook_Character::UpdateAnimation(RE::Character* a_this, float a_delta) {
		float before = get_delta(a_this);
		_UpdateAnimation(a_this, a_delta);
		float delta = get_delta(a_this) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in UpdateAnimation");
		}
		experiment(a_this);
	}

	void Hook_Character::UpdateNoAI(RE::Character* a_this, float a_delta) {
		float before = get_delta(a_this);
		_UpdateNoAI(a_this, a_delta);
		float delta = get_delta(a_this) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in UpdateNoAI");
		}
		experiment(a_this);
	}

	void Hook_Character::ModifyMovementData(RE::Character* a_this, float a_delta, NiPoint3& a_arg3, NiPoint3& a_arg4) {
		float before = get_delta(a_this);
		_ModifyMovementData(a_this, a_delta, a_arg3, a_arg4);
		float delta = get_delta(a_this) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in ModifyMovementData");
		}
		experiment(a_this);
	}

	void Hook_Character::ProcessTracking(RE::Character* a_this, float a_delta, NiAVObject* a_obj3D) {
		float before = get_delta(a_this);
		_ProcessTracking(a_this, a_delta, a_obj3D);
		float delta = get_delta(a_this) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in ProcessTracking");
		}
		experiment(a_this);
	}
}
