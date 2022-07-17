#include "hooks.h"
#include "hooks/main.h"
#include "hooks/actor.h"
#include "hooks/character.h"
#include "hooks/playercharacter.h"
#include "hooks/impact.h"
#include "hooks/timedelta.h"

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

		logger::info("SetGraphVariableFloat is at: {}", (REL::Relocation<func_t> func{ RELOCATION_ID(32141, 32885) }).address());

		logger::info("Gts finished applying hooks...");
	}

}
