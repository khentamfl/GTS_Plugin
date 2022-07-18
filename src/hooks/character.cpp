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
}

namespace Hooks
{
	void Hook_Character::Hook() {
		logger::info("Hooking Character");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Character[0] };

		_Update = ActorVtbl.write_vfunc(0xAD, Update);
		_UpdateNonRenderSafe = ActorVtbl.write_vfunc(0xB1, UpdateNonRenderSafe);
		_UpdateAnimation = ActorVtbl.write_vfunc(0x7D, UpdateAnimation);
		_UpdateNoAI = ActorVtbl.write_vfunc(0xAE, UpdateNoAI);
		_ModifyMovementData = ActorVtbl.write_vfunc(0x11A, ModifyMovementData);
		_ProcessTracking = ActorVtbl.write_vfunc(0x112, ProcessTracking);
	}

	void Hook_Character::Update(RE::Character* a_this, float a_delta) {
		float before = get_delta(actor);
		_Update(a_this, a_delta);
		float delta = get_delta(actor) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in Update");
		}
	}

	void Hook_Character::UpdateNonRenderSafe(RE::Character* a_this, float a_delta) {
		float before = get_delta(actor);
		_UpdateNonRenderSafe(a_this, a_delta);
		float delta = get_delta(actor) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in UpdateNonRenderSafe");
		}
	}

	void Hook_Character::UpdateAnimation(RE::Character* a_this, float a_delta) {
		float before = get_delta(actor);
		_UpdateAnimation(a_this, a_delta);
		float delta = get_delta(actor) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in UpdateAnimation");
		}
	}

	void Hook_Character::UpdateNoAI(RE::Character* a_this, float a_delta) {
		float before = get_delta(actor);
		_UpdateNoAI(a_this, a_delta);
		float delta = get_delta(actor) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in UpdateNoAI");
		}
	}

	void Hook_Character::ModifyMovementData(RE::Character* a_this, float a_delta, NiPoint3& a_arg3, NiPoint3& a_arg4) {
		float before = get_delta(actor);
		_ModifyMovementData(a_this, a_delta, a_arg3, a_arg4);
		float delta = get_delta(actor) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in ModifyMovementData");
		}
	}

	void Hook_Character::ProcessTracking(RE::Character* a_this, float a_delta, NiAVObject* a_obj3D) {
		float before = get_delta(actor);
		_ProcessTracking(a_this, a_delta, a_obj3D);
		float delta = get_delta(actor) - before;
		if (fabs(delta) > 1e-5) {
			log::info("Non zero in ProcessTracking");
		}
	}
}
