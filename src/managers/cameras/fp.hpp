#pragma once
#include "managers/cameras/state.hpp"

using namespace RE;

namespace Gts {
	class FirstPerson : public CameraState {
		public:
			virtual bool PermitManualEdit() override;
	};
}
