#include "hooks/thirdPersonState.hpp"
#include "util.hpp"
#include "scale/scale.hpp"
#include "node.hpp"
#include "managers/altcamera.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	void Hook_ThirdPersonState::Hook() {
		logger::info("Hooking ThirdPersonState");
		REL::Relocation<std::uintptr_t> Vtbl{ ThirdPersonState::VTABLE[0] };
		_Begin = Vtbl.write_vfunc(0x01, Begin);
		_End = Vtbl.write_vfunc(0x02, End);
		_Update = Vtbl.write_vfunc(0x03, Update);
		_GetRotation = Vtbl.write_vfunc(0x04, GetRotation);
		_GetTranslation = Vtbl.write_vfunc(0x05, GetTranslation);
		_UpdateRotation = Vtbl.write_vfunc(0x0E, UpdateRotation);
	}


	void Hook_ThirdPersonState::Begin(ThirdPersonState* a_this) {
		log::info("ThirdPersonState: Begin");
		_Begin(a_this);
	}

	void Hook_ThirdPersonState::End(ThirdPersonState* a_this) {
		log::info("ThirdPersonState: End");
		_End(a_this);
	}

	void Hook_ThirdPersonState::Update(ThirdPersonState* a_this, BSTSmartPointer<TESCameraState>& a_nextState) {
		log::info("ThirdPersonState: Update");
		_Update(a_this, a_nextState);
		CameraManager::GetSingleton().Update();
	}

	void Hook_ThirdPersonState::GetRotation(ThirdPersonState* a_this, NiQuaternion& a_rotation) {
		log::info("ThirdPersonState: GetRotation");
		_GetRotation(a_this, a_rotation);
	}

	void Hook_ThirdPersonState::GetTranslation(ThirdPersonState* a_this, NiPoint3& a_translation) {
		log::info("ThirdPersonState: GetTranslation");
		_GetTranslation(a_this, a_translation);
	}

	void Hook_ThirdPersonState::UpdateRotation(ThirdPersonState* a_this) {
		log::info("ThirdPersonState: UpdateRotation");
		_UpdateRotation(a_this);
		CameraManager::GetSingleton().Update();
	}
}
