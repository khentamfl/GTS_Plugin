#include "hooks/cameraState.hpp"
#include "util.hpp"

using namespace RE;
using namespace Gts;

namespace Hooks
{
	void Hook_CameraState::Hook() {
		logger::info("Hooking ThirdPersonState");
		REL::Relocation<std::uintptr_t> Vtbl{ ThirdPersonState::VTABLE[0] };
		_GetRotation = Vtbl.write_vfunc(0x04, GetRotation);
		_GetTranslation = Vtbl.write_vfunc(0x05, GetTranslation);
	}

	void Hook_CameraState::GetRotation(ThirdPersonState* a_this, NiQuaternion& a_rotation) {
		log::info("GetRotation");
		_GetRotation(a_this, a_rotation);
	}

	void Hook_CameraState::GetTranslation(ThirdPersonState* a_this, NiPoint3& a_translation) {
		log::info("GetTranslation");
		_GetTranslation(a_this, a_translation);
	}
}
