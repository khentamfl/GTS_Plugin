#pragma once

using namespace RE;

namespace Hooks
{
	void Install();

	class MainUpdateHook
	{
		public:
			static void Hook();

		private:
			static void Update(RE::Main* a_this, float a2);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class HookOnPlayerUpdate
	{
		public:
			static void Hook();
		private:

			static void Update(RE::PlayerCharacter* a_this, float a_delta);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class HookOnActorUpdate
	{
		public:
			static void Hook();
		private:

			static void Update(RE::Actor* a_this, float a_delta);
			static inline REL::Relocation<decltype(Update)> _Update;
	};

	class HookbhkCharProxyController
	{
		public:
			static void Hook();
		private:

			static void CharacterInteractionCallback(bhkCharProxyController* a_this, hkpCharacterProxy* a_proxy, hkpCharacterProxy* a_otherProxy, const hkContactPoint& a_contact);
			static inline REL::Relocation<decltype(CharacterInteractionCallback)> _Orig;
	};

	class HookhkpCharacterProxyListener
	{
		public:
			static void Hook();
		private:

			static void CharacterInteractionCallback(hkpCharacterProxyListener* a_this, hkpCharacterProxy* a_proxy, hkpCharacterProxy* a_otherProxy, const hkContactPoint& a_contact);
			static inline REL::Relocation<decltype(CharacterInteractionCallback)> _Orig;
	};

}
