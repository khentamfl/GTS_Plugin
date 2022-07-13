#include "hooks.h"
#include "GtsManager.h"
#include "persistent.h"
#include "footstep.h"

using namespace RE;

namespace Hooks
{
	void Install()
	{
		logger::info("Gts applying hooks...");

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.create(256);

		Hook_MainUpdate::Hook(trampoline);
		Hook_OnPlayerUpdate::Hook();
		Hook_OnActorUpdate::Hook();
		// Hook_bhkCharProxyController::Hook();
		// Hook_hkpCharacterProxyListener::Hook();
		// Hook_bhkCharacterController::Hook();
		Hook_BGSImpactManager::Hook();

		logger::info("Gts finished applying hooks...");

		REL::Relocation<uintptr_t> get_size_fun(REL::RelocationID(19238, 19664));
		logger::info("Get Scale is at {}", get_size_fun.address());
	}

	// Main
	void Hook_MainUpdate::Hook(Trampoline& trampoline)
	{
		REL::Relocation<uintptr_t> hook{REL::RelocationID(35551, 36544)};          // main loop
		logger::info("Gts applying Main Update Hook at {}", hook.address());
		_Update = trampoline.write_call<5>(hook.address() + REL::Relocate(0x11F, 0x160), Update);
	}

	void Hook_MainUpdate::Update(RE::Main* a_this, float a2)
	{
		_Update(a_this, a2);
		Gts::GtsManager::GetSingleton().poll();
	}

	// PLayer update
	void Hook_OnPlayerUpdate::Hook() {
		logger::info("Hooking player update");
		REL::Relocation<std::uintptr_t> PlayerCharacterVtbl{ RE::VTABLE_PlayerCharacter[0] };

		// _Update = PlayerCharacterVtbl.write_vfunc(0xAD, Update);
		_UpdateAnimation = PlayerCharacterVtbl.write_vfunc(0x7D, UpdateAnimation);
	}

	void Hook_OnPlayerUpdate::Update(RE::PlayerCharacter* a_this, float a_delta) {
		_Update(a_this, a_delta);

		Gts::GtsManager::GetSingleton().poll_actor(a_this);
	}

	void Hook_OnPlayerUpdate::UpdateAnimation(RE::PlayerCharacter* a_this, float a_delta) {
		float anim_speed = 1.0;
		if (Gts::GtsManager::GetSingleton().enabled) {
			auto saved_data = Gts::Persistent::GetSingleton().GetActorData(a_this);
			if (saved_data) {
				if (saved_data->anim_speed > 0.0) {
					anim_speed = saved_data->anim_speed;
				}
			}
		}
		_UpdateAnimation(a_this, a_delta * anim_speed);
	}

	// Actor update
	void Hook_OnActorUpdate::Hook() {
		logger::info("Hooking character update");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Actor[0] };

		// _Update = ActorVtbl.write_vfunc(0xAD, Update);
		_UpdateAnimation = ActorVtbl.write_vfunc(0x7D, UpdateAnimation);
	}

	void Hook_OnActorUpdate::Update(RE::Actor* a_this, float a_delta) {
		_Update(a_this, a_delta);

		Gts::GtsManager::GetSingleton().poll_actor(a_this);
	}

	void Hook_OnActorUpdate::UpdateAnimation(RE::Actor* a_this, float a_delta) {
		float anim_speed = 1.0;
		if (Gts::GtsManager::GetSingleton().enabled) {
			auto saved_data = Gts::Persistent::GetSingleton().GetActorData(a_this);
			if (saved_data) {
				if (saved_data->anim_speed > 0.0) {
					anim_speed = saved_data->anim_speed;
				}
			}
		}
		_UpdateAnimation(a_this, a_delta * anim_speed);
	}

	// bhkCharProxyController
	void Hook_bhkCharProxyController::Hook() {
		logger::info("Hooking character proxy controller");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_bhkCharProxyController[0] };

		// CharacterInteractionCallback
		_Orig = ActorVtbl.write_vfunc(0x04, CharacterInteractionCallback);

	}

	void Hook_bhkCharProxyController::CharacterInteractionCallback(bhkCharProxyController* a_this, hkpCharacterProxy* a_proxy, hkpCharacterProxy* a_otherProxy, const hkContactPoint& a_contact) {
		logger::info("Char char collision");
		_Orig(a_this, a_proxy, a_otherProxy, a_contact);
	}

	// hkpCharacterProxyListener
	void Hook_hkpCharacterProxyListener::Hook() {
		logger::info("Hooking character proxy listener");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_hkpCharacterProxyListener[0] };

		// CharacterInteractionCallback
		_Orig = ActorVtbl.write_vfunc(0x04, CharacterInteractionCallback);

	}

	void Hook_hkpCharacterProxyListener::CharacterInteractionCallback(hkpCharacterProxyListener* a_this, hkpCharacterProxy* a_proxy, hkpCharacterProxy* a_otherProxy, const hkContactPoint& a_contact) {
		logger::info("Char char collision listener");
		_Orig(a_this, a_proxy, a_otherProxy, a_contact);
	}

	// hkpCharacterProxyListener
	void Hook_bhkCharacterController::Hook() {
		logger::info("Hooking Hook_bhkCharacterController");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_bhkCharacterController[0] };

		_GetPositionImpl = Vtbl.write_vfunc(0x02, GetPositionImpl);
		_SetPositionImpl = Vtbl.write_vfunc(0x03, SetPositionImpl);
		_GetTransformImpl = Vtbl.write_vfunc(0x04, GetTransformImpl);
		_SetTransformImpl = Vtbl.write_vfunc(0x05, SetTransformImpl);
	}

	void Hook_bhkCharacterController::GetPositionImpl(bhkCharacterController* a_this, hkVector4& a_pos, bool a_applyCenterOffset) {
		logger::info("GetPositionImpl");
		_GetPositionImpl(a_this, a_pos, a_applyCenterOffset);
		float output[4];
		_mm_storeu_ps(&output[0], a_pos.quad);
		logger::info("pos={},{},{},{}", output[0], output[1], output[2], output[3]);
	}

	void Hook_bhkCharacterController::SetPositionImpl(bhkCharacterController* a_this, const hkVector4& a_pos, bool a_applyCenterOffset, bool a_forceWarp) {
		logger::info("SetPositionImpl");
		_SetPositionImpl(a_this, a_pos, a_applyCenterOffset, a_forceWarp);
		float output[4];
		_mm_storeu_ps(&output[0], a_pos.quad);
		logger::info("pos={},{},{},{}", output[0], output[1], output[2], output[3]);
	}

	void Hook_bhkCharacterController::GetTransformImpl(bhkCharacterController* a_this, hkTransform& a_tranform) {
		logger::info("GetTransformImpl");
		_GetTransformImpl(a_this, a_tranform);
		float col_a[4];
		float col_b[4];
		float col_c[4];
		float col_d[4];
		_mm_storeu_ps(&col_a[0], a_tranform.rotation.col0.quad);
		_mm_storeu_ps(&col_b[0], a_tranform.rotation.col1.quad);
		_mm_storeu_ps(&col_c[0], a_tranform.rotation.col2.quad);
		_mm_storeu_ps(&col_d[0], a_tranform.translation.quad);
		logger::info("pos={},{},{},{}", col_a[0], col_b[0], col_c[0], col_d[0]);
		logger::info("pos={},{},{},{}", col_a[1], col_b[1], col_c[1], col_d[1]);
		logger::info("pos={},{},{},{}", col_a[2], col_b[2], col_c[2], col_d[2]);
		logger::info("pos={},{},{},{}", col_a[3], col_b[3], col_c[3], col_d[3]);
	}

	void Hook_bhkCharacterController::SetTransformImpl(bhkCharacterController* a_this, const hkTransform& a_tranform) {
		logger::info("GetTransformImpl");
		_SetTransformImpl(a_this, a_tranform);
		float col_a[4];
		float col_b[4];
		float col_c[4];
		float col_d[4];
		_mm_storeu_ps(&col_a[0], a_tranform.rotation.col0.quad);
		_mm_storeu_ps(&col_b[0], a_tranform.rotation.col1.quad);
		_mm_storeu_ps(&col_c[0], a_tranform.rotation.col2.quad);
		_mm_storeu_ps(&col_d[0], a_tranform.translation.quad);
		logger::info("pos={},{},{},{}", col_a[0], col_b[0], col_c[0], col_d[0]);
		logger::info("pos={},{},{},{}", col_a[1], col_b[1], col_c[1], col_d[1]);
		logger::info("pos={},{},{},{}", col_a[2], col_b[2], col_c[2], col_d[2]);
		logger::info("pos={},{},{},{}", col_a[3], col_b[3], col_c[3], col_d[3]);
	}

	// BGSImpactManager
	void Hook_BGSImpactManager::Hook() {
		logger::info("Hooking Hook_BGSImpactManager");
		REL::Relocation<std::uintptr_t> Vtbl{ RE::VTABLE_BGSImpactManager[0] };

		_ProcessEvent = Vtbl.write_vfunc(0x01, ProcessEvent);
	}

	BSEventNotifyControl Hook_BGSImpactManager::ProcessEvent(BGSImpactManager* a_this, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		Gts::FootStepManager::GetSingleton().HookProcessEvent(a_event, a_eventSource);
		auto result = _ProcessEvent(a_this, a_event, a_eventSource);
		return result;
	}

}
