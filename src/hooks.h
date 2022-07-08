#pragma once
// Hooks into skyrim engine
//
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

	class Hook_bhkCharacterController
	{
		public:
			static void Hook();
		private:

			static void GetPositionImpl(bhkCharacterController* a_this, hkVector4& a_pos, bool a_applyCenterOffset);
			static inline REL::Relocation<decltype(GetPositionImpl)> _GetPositionImpl;

			static void SetPositionImpl(bhkCharacterController* a_this, const hkVector4& a_pos, bool a_applyCenterOffset, bool a_forceWarp);
			static inline REL::Relocation<decltype(SetPositionImpl)> _SetPositionImpl;

			static void  GetTransformImpl(bhkCharacterController* a_this, hkTransform& a_tranform);
			static inline REL::Relocation<decltype(GetTransformImpl)> _GetTransformImpl;

			static void  SetTransformImpl(bhkCharacterController* a_this, const hkTransform& a_tranform);
			static inline REL::Relocation<decltype(SetTransformImpl)> _SetTransformImpl;
	};

	class Hook_BGSImpactManager
	{
		public:
			static void Hook();
		private:

			static BSEventNotifyControl ProcessEvent(BGSImpactManager* a_this, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource);
			static inline REL::Relocation<decltype(ProcessEvent)> _ProcessEvent;
	};


}
