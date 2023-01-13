#pragma once
#include "managers/cameras/tp/foot.hpp"
#include "spring.hpp"

using namespace RE;

namespace Gts {
	class FootcameraL : public FootcameraL {
		protected:
			virtual NiPoint3 GetFootPos() override;
	};
}
