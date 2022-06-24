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

}
