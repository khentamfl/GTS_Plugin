#pragma once
// Module that handles footsteps
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	enum ExplosionKind {
		Footstep,
		JumpLand,
	};

	class ExplosionManager {
		public:
			[[nodiscard]] static ExplosionManager& GetSingleton() noexcept;

			void make_explosion(ExplosionKind kind, Actor* actor, NiAVObject* node, NiPoint3 offset, float scale);
	};
}
