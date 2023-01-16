#pragma once
#include "hooks/hooks.hpp"

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_PlayerCharacter
	{
		public:
			static void Hook();
		private:
			static void HandleHealthDamage(PlayerCharacter* a_this, Actor* a_attacker, float a_damage);
			static inline REL::Relocation<decltype(HandleHealthDamage)> _HandleHealthDamage;

			static void AddPerk(PlayerCharacter* a_this, BGSPerk* a_perk, std::uint32_t a_rank);
			static inline REL::Relocation<decltype(AddPerk)> _AddPerk;

			static void RemovePerk(PlayerCharacter* a_this, BGSPerk* a_perk);
			static inline REL::Relocation<decltype(RemovePerk)> _RemovePerk;
	};
}
