#include "hooks/hooks.hpp"
#include "hooks/main.hpp"
#include "hooks/actor.hpp"
#include "hooks/character.hpp"
#include "hooks/playercharacter.hpp"
#include "hooks/impact.hpp"
#include "hooks/vm.hpp"
#include "hooks/havok.hpp"

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
