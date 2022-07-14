#include "hooks.h"
#include "hooks/main.h"
#include "hooks/actor.h"
#include "hooks/character.h"
#include "hooks/playercharacter.h"
#include "hooks/impact.h"

using namespace RE;

namespace Hooks
{
	void Install()
	{
		logger::info("Gts applying hooks...");

		auto& trampoline = SKSE::GetTrampoline();
		trampoline.create(256);

		Hook_MainUpdate::Hook(trampoline);
		Hook_Player::Hook();
		Hook_Actor::Hook();
		Hook_Character::Hook();
		Hook_BGSImpactManager::Hook();

		logger::info("Gts finished applying hooks...");
	}

}
