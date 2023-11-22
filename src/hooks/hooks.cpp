#include "hooks/hooks.hpp"
#include "hooks/main.hpp"
#include "hooks/headTracking.hpp"
#include "hooks/impact.hpp"
#include "hooks/vm.hpp"
#include "hooks/havok.hpp"
#include "hooks/magicTarget.hpp"
#include "hooks/hkbBehaviorGraph.hpp"
#include "hooks/cameraState.hpp"
#include "hooks/playerCamera.hpp"
#include "hooks/playerCharacter.hpp"
#include "hooks/actor.hpp"
#include "hooks/character.hpp"
#include "hooks/sink.hpp"
#include "hooks/jump.hpp"
#include "hooks/scale.hpp"

using namespace RE;

namespace Hooks
{
	void Install()
	{
		log::info("Gts applying hooks...");

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.create(512);

		Hook_MainUpdate::Hook(trampoline);
		Hook_BGSImpactManager::Hook();
		Hook_VM::Hook();
		Hook_Havok::Hook(trampoline);
		Hook_HeadTracking::Hook(trampoline);
		//Hook_MagicTarget::Hook();
		Hook_hkbBehaviorGraph::Hook();
		Hook_PlayerCharacter::Hook();
		Hook_Actor::Hook(trampoline);
		Hook_Character::Hook();
		Hook_Sinking::Hook(trampoline);
		Hook_Jumping::Hook(trampoline);
		Hook_Scale::Hook(trampoline);
		HookCameraStates();

		log::info("Gts finished applying hooks...");
	}
}
