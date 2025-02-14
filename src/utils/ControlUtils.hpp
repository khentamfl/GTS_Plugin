#pragma once

#include "events.hpp"
#include <SKSE/Version.h>


using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace SKSE {
    constexpr REL::Version RUNTIME_SSE_1_6_1130(1, 6, 1130, 0);
}

namespace RE { // alandtse/CommonLibVR/tree/ng  copy-paste
	class UserEventEnabled;

	class GTSControlMap :
		public BSTSingletonSDM<GTSControlMap>,      // 00
		public BSTEventSource<UserEventEnabled>  // 08
	{
	public:
		using InputContextID = UserEvents::INPUT_CONTEXT_ID;
		using UEFlag = UserEvents::USER_EVENT_FLAG;

		enum : std::uint32_t
		{
			kInvalid = static_cast<std::uint8_t>(-1)
		};

		struct UserEventMapping
		{
		public:
			// members
			BSFixedString                           eventID;             // 00
			std::uint16_t                           inputKey;            // 08
			std::uint16_t                           modifier;            // 08
			std::int8_t                             indexInContext;      // 0C
			bool                                    remappable;          // 0D
			bool                                    linked;              // 0E
			stl::enumeration<UEFlag, std::uint32_t> userEventGroupFlag;  // 10
			std::uint32_t                           pad14;               // 14
		};
		static_assert(sizeof(UserEventMapping) == 0x18);

		struct LinkedMapping
		{
		public:
			// members
			BSFixedString  linkedMappingName;     // 00
			InputContextID linkedMappingContext;  // 08
			INPUT_DEVICE   device;                // 0C
			InputContextID linkFromContext;       // 10
			std::uint32_t  pad14;                 // 14
			BSFixedString  linkFromName;          // 18
		};
		static_assert(sizeof(LinkedMapping) == 0x20);

		static GTSControlMap* GetSingleton();

		struct RUNTIME_DATA
		{
    #define RUNTIME_DATA_CONTENT                                                                        \
	BSTArray<LinkedMapping>                          linkedMappings;               /* 0E8, VR 108*/ \
	BSTArray<InputContextID>                         contextPriorityStack;         /* 100, VR 120*/ \
	stl::enumeration<UEFlag, std::uint32_t>          enabledControls;              /* 118, VR 138*/ \
	stl::enumeration<UEFlag, std::uint32_t>          unk11C;                       /* 11C, VR 13C*/ \
	std::int8_t                                      textEntryCount;               /* 120, VR 140*/ \
	bool                                             ignoreKeyboardMouse;          /* 121, VR 141*/ \
	bool                                             ignoreActivateDisabledEvents; /* 122, VR 142*/ \
	std::uint8_t                                     pad123;                       /* 123, VR 143*/ \
	stl::enumeration<PC_GAMEPAD_TYPE, std::uint32_t> gamePadMapType;               /* 124, VR 144*/
			RUNTIME_DATA_CONTENT
		};
		static_assert(sizeof(RUNTIME_DATA) == 0x40);

		//members

		// members
        #if !defined(ENABLE_SKYRIM_VR)                                   //flat
        #	if !defined(ENABLE_SKYRIM_AE) && defined(ENABLE_SKYRIM_SE)  // SSE
                RUNTIME_DATA_CONTENT;                                    // 0E8
        #	else                                                        // AE
                RUNTIME_DATA_CONTENT;  // 0F8
        #	endif
        #elif !defined(ENABLE_SKYRIM_AE) && defined(ENABLE_SKYRIM_SE)  // VR
                RUNTIME_DATA_CONTENT;  // 108
        #else                                                          // ALL
                // controlMap can be accessed up to kTotal, kAETotal, or kVRTotal based on runtime
        #endif

		[[nodiscard]] inline RUNTIME_DATA& GetRuntimeData() noexcept
		{
			if SKYRIM_REL_CONSTEXPR (REL::Module::IsAE()) {
				if (REL::Module::get().version().compare(SKSE::RUNTIME_SSE_1_6_1130) != std::strong_ordering::less) {
					return REL::RelocateMember<RUNTIME_DATA>(this, 0xf0);
				}
			}
			return REL::RelocateMember<RUNTIME_DATA>(this, 0xE8, 0x108);
		}

		[[nodiscard]] inline const RUNTIME_DATA& GetRuntimeData() const noexcept
		{
			if SKYRIM_REL_CONSTEXPR (REL::Module::IsAE()) {
				if (REL::Module::get().version().compare(SKSE::RUNTIME_SSE_1_6_1130) != std::strong_ordering::less) {
					return REL::RelocateMember<RUNTIME_DATA>(this, 0xf0);
				}
			}
			return REL::RelocateMember<RUNTIME_DATA>(this, 0xE8, 0x108);
		}
	};
#if !defined(ENABLE_SKYRIM_VR)
#	if !defined(ENABLE_SKYRIM_AE)
	static_assert(sizeof(GTSControlMap) == 0x130);
#	elif !defined(ENABLE_SKYRIM_SE)
	static_assert(sizeof(GTSControlMap) == 0x128);
#	endif
#elif !defined(ENABLE_SKYRIM_SE) && !defined(ENABLE_SKYRIM_AE)
	//static_assert(sizeof(ControlMap) == 0x148);  // VS seems to choke even though this should be right
#endif
}
#undef RUNTIME_DATA_CONTENT


namespace Gts {
    using UEFlag = UserEvents::USER_EVENT_FLAG;
    void ToggleControls(UEFlag a_flag, bool a_enable);
    void ManageControls();
}