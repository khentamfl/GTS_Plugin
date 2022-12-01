#pragma once
// Critically Damped Springs
//

using namespace SKSE;

namespace Gts {
	class Spring {
		public:
			float value = 0.0;
			float target = 0.0;
			float velocity = 0.0;
			float halflife = 1.0;

			void Update(float delta);
	}
}
