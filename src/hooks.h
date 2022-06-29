#pragma once

using namespace RE;
using namespace SKSE;

namespace Hooks
{
	void Install();

	class Hook_MainUpdate
	{
		public:
			static void Hook(Trampoline& trampoline);

		private:
			static void Update(RE::Main* a_this, float a2);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_OnPlayerUpdate
	{
		public:
			static void Hook();
		private:

			static void Update(RE::PlayerCharacter* a_this, float a_delta);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_OnActorUpdate
	{
		public:
			static void Hook();
		private:

			static void Update(RE::Actor* a_this, float a_delta);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class Hook_bhkCharProxyController
	{
		public:
			static void Hook();
		private:

			static void CharacterInteractionCallback(bhkCharProxyController* a_this, hkpCharacterProxy* a_proxy, hkpCharacterProxy* a_otherProxy, const hkContactPoint& a_contact);
			static inline REL::Relocation<decltype(CharacterInteractionCallback)> _Orig;
	};

	class Hook_hkpCharacterProxyListener
	{
		public:
			static void Hook();
		private:

			static void CharacterInteractionCallback(hkpCharacterProxyListener* a_this, hkpCharacterProxy* a_proxy, hkpCharacterProxy* a_otherProxy, const hkContactPoint& a_contact);
			static inline REL::Relocation<decltype(CharacterInteractionCallback)> _Orig;
	};



}
