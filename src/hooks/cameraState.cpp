#include "hooks/cameraState.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "managers/altcamera.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	void Hook_CameraState::Hook() {
		logger::info("Hooking TESCameraState");
		REL::Relocation<std::uintptr_t> Vtbl{ TESCameraState::VTABLE[0] };
		_Update = Vtbl.write_vfunc(0x03, Update);
		_GetRotation = Vtbl.write_vfunc(0x04, GetRotation);
		_GetTranslation = Vtbl.write_vfunc(0x05, GetTranslation);
		_UpdateRotation = Vtbl.write_vfunc(0x0E, UpdateRotation);
	}

	void Hook_CameraState::Update(TESCameraState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		log::info("TESCameraState: Update");
		_Update(a_this, a_nextState);
	}

	void Hook_CameraState::GetRotation(TESCameraState* a_this, NiQuaternion& a_rotation) {
		log::info("TESCameraState: GetRotation");
		_GetRotation(a_this, a_rotation);
	}

	void Hook_CameraState::GetTranslation(TESCameraState* a_this, NiPoint3& a_translation) {
		log::info("TESCameraState: GetTranslation");
		_GetTranslation(a_this, a_translation);
	}
}
