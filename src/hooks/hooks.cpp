#include "hooks/hooks.h"
#include "hooks/main.h"
#include "hooks/actor.h"
#include "hooks/character.h"
#include "hooks/playercharacter.h"
#include "hooks/impact.h"
#include "hooks/vm.h"
#include "hooks/havok.h"

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

		using func_t = decltype(&IAnimationGraphManagerHolder::SetGraphVariableBool);
		REL::Relocation<func_t> func{ RELOCATION_ID(32141, 32885) };
		logger::info("SetGraphVariableFloat is at: {}", func.address());

		logger::info("Gts finished applying hooks...");
	}

}
