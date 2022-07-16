#pragma once
using namespace RE;
using namespace SKSE;

namespace Hooks
{
	class Hook_Character
	{
		public:
			static void Hook();
		private:

			static void UpdateAnimation(RE::Actor* a_this, float a_delta);
			static inline REL::Relocation<decltype(UpdateAnimation)> _UpdateAnimation;
            
            static void ModifyAnimationUpdateData(RE::Actor* a_this, BSAnimationUpdateData& a_data);
            static inline REL::Relocation<decltype(ModifyAnimationUpdateData)> _ModifyAnimationUpdateData;
            
            static void Update(RE::Actor* a_this, float a_delta);
			static inline REL::Relocation<decltype(Update)> _Update;
    };
}
