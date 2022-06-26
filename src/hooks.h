#pragma once

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

}
