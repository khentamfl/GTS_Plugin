#include "managers/cameras/trans.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	TransState::TransState(CameraState* stateA, CameraState* stateB) : stateA(stateA), stateB(stateB) {
		this->smoothIn.value = 0.0;
		this->smoothIn.target = 1.0;
		this->smoothIn.velocity = 0.0;
	}
	float TransState::GetScale() {
		return this->smoothIn.value * this->stateA->GetScale() + (1.0 - this->smoothIn.value) * this->stateB->GetScale();
	}

	NiPoint3 TransState::GetOffset(const NiPoint3& cameraPosLocal) {
		return this->smoothIn.value * this->stateA->GetOffset(cameraPosLocal) + (1.0 - this->smoothIn.value) * this->stateB->GetOffset(cameraPosLocal);
	}
	NiPoint3 TransState::GetOffset(const NiPoint3& cameraPosLocal, bool isProne) {
		return this->smoothIn.value * this->stateA->GetOffset(cameraPosLocal, isProne) + (1.0 - this->smoothIn.value) * this->stateB->GetOffset(cameraPosLocal, isProne);
	}
	NiPoint3 TransState::GetOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->smoothIn.value * this->stateA->GetOffsetProne(cameraPosLocal) + (1.0 - this->smoothIn.value) * this->stateB->GetOffsetProne(cameraPosLocal);
	}

	NiPoint3 TransState::GetCombatOffset(const NiPoint3& cameraPosLocal) {
		return this->smoothIn.value * this->stateA->GetCombatOffset(cameraPosLocal) + (1.0 - this->smoothIn.value) * this->stateB->GetCombatOffset(cameraPosLocal);
	}
	NiPoint3 TransState::GetCombatOffset(const NiPoint3& cameraPosLocal, bool isProne) {
		return this->smoothIn.value * this->stateA->GetCombatOffset(cameraPosLocal, isProne) + (1.0 - this->smoothIn.value) * this->stateB->GetCombatOffset(cameraPosLocal, isProne);
	}
	NiPoint3 TransState::GetCombatOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->smoothIn.value * this->stateA->GetCombatOffsetProne(cameraPosLocal) + (1.0 - this->smoothIn.value) * this->stateB->GetCombatOffsetProne(cameraPosLocal);
	}

	NiPoint3 TransState::GetPlayerLocalOffset(const NiPoint3& cameraPosLocal) {
		return this->smoothIn.value * this->stateA->GetPlayerLocalOffset(cameraPosLocal) + (1.0 - this->smoothIn.value) * this->stateB->GetPlayerLocalOffset(cameraPosLocal);
	}
	NiPoint3 TransState::GetPlayerLocalOffset(const NiPoint3& cameraPosLocal, bool isProne) {
		return this->smoothIn.value * this->stateA->GetPlayerLocalOffset(cameraPosLocal, isProne) + (1.0 - this->smoothIn.value) * this->stateB->GetPlayerLocalOffset(cameraPosLocal, isProne);
	}
	NiPoint3 TransState::GetPlayerLocalOffsetProne(const NiPoint3& cameraPosLocal) {
		return this->smoothIn.value * this->stateA->GetPlayerLocalOffsetProne(cameraPosLocal) + (1.0 - this->smoothIn.value) * this->stateB->GetPlayerLocalOffsetProne(cameraPosLocal);
	}

	NiPoint3 TransState::GetPlayerLocalOffsetInstant() {
		return this->smoothIn.value * this->stateA->GetPlayerLocalOffsetInstant() + (1.0 - this->smoothIn.value) * this->stateB->GetPlayerLocalOffsetInstant();
	}

	bool TransState::PermitManualEdit() {
		return false;
	}

	bool TransState::IsDone() {
		return this->smoothIn.value > 0.95;
	}
}
