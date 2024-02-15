#pragma once

#include "events.hpp"
#include <SKSE/Version.h>
#include <assert.h>

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace SKSE
{
	constexpr REL::Version RUNTIME_SSE_1_6_1130(1, 6, 1130, 0);
}

namespace RE {
    class Control_Map :
		public BSTSingletonSDM<Control_Map>      // 00
        {
            using UEFlag = UserEvents::USER_EVENT_FLAG;
            static Control_Map* GetSingleton();

            struct RUNTIME_DATA
            {
                #define RUNTIME_DATA_CONTENT                                                                        \
                    stl::enumeration<UEFlag, std::uint32_t>          enabledControls;              /* 118, VR 138*/ \
                    stl::enumeration<UEFlag, std::uint32_t>          unk11C;                       /* 11C, VR 13C*/ \
                RUNTIME_DATA_CONTENT
            };
            static_assert(sizeof(RUNTIME_DATA) == 0x40);

            //members

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
            static_assert(sizeof(Control_Map) == 0x130);
        #	elif !defined(ENABLE_SKYRIM_SE)
            static_assert(sizeof(Control_Map) == 0x128);
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