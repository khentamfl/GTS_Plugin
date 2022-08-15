#pragma once
// Module that handles footsteps
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	enum Foot {
		Left,
		Right,
		Front,
		Back,
		JumpLand,
		Unknown,
	};

	struct Impact {
		Actor* actor;
		Foot kind;
		float scale;
		float effective_scale;
		std::vector<NiAVObject*> nodes;
	};
	class ImpactManager {
		public:
			[[nodiscard]] static ImpactManager& GetSingleton() noexcept;

			void HookProcessEvent(BGSImpactManager* impact, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource);
	};
}
