#include "hooks/hooks.hpp"
#include "hooks/main.hpp"
#include "hooks/actor.hpp"
#include "hooks/character.hpp"
#include "hooks/playercharacter.hpp"
#include "hooks/impact.hpp"
#include "hooks/vm.hpp"
#include "hooks/havok.hpp"
#include "hooks/hkbBehaviorGraph.hpp"

using namespace RE;

namespace Hooks
{
	void Install()
	{
		logger::info("Gts applying hooks...");

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.create(256);

		Hook_MainUpdate::Hook(trampoline);
		Hook_Actor::Hook();
		Hook_Character::Hook();
		Hook_Player::Hook();
		Hook_BGSImpactManager::Hook();
		Hook_VM::Hook();
		Hook_Havok::Hook(trampoline);
		Hook_hkbBehaviorGraph::Hook();

		logger::info("Gts finished applying hooks...");
	}

}
