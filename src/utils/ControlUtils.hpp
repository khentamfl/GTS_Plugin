#pragma once

#include "events.hpp"
#include <SKSE/Version.h>

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace SKSE
{
	constexpr REL::Version RUNTIME_SSE_1_1_47(1, 1, 47, 0);
	constexpr REL::Version RUNTIME_SSE_1_1_51(1, 1, 51, 0);
	constexpr REL::Version RUNTIME_SSE_1_2_36(1, 2, 36, 0);
	constexpr REL::Version RUNTIME_SSE_1_2_39(1, 2, 39, 0);
	constexpr REL::Version RUNTIME_SSE_1_3_5(1, 3, 5, 0);
	constexpr REL::Version RUNTIME_SSE_1_3_9(1, 3, 9, 0);
	constexpr REL::Version RUNTIME_SSE_1_4_2(1, 4, 2, 0);
	constexpr REL::Version RUNTIME_SSE_1_5_3(1, 5, 3, 0);
	constexpr REL::Version RUNTIME_SSE_1_5_16(1, 5, 16, 0);
	constexpr REL::Version RUNTIME_SSE_1_5_23(1, 5, 23, 0);
	constexpr REL::Version RUNTIME_SSE_1_5_39(1, 5, 39, 0);
	constexpr REL::Version RUNTIME_SSE_1_5_50(1, 5, 50, 0);
	constexpr REL::Version RUNTIME_SSE_1_5_53(1, 5, 53, 0);
	constexpr REL::Version RUNTIME_SSE_1_5_62(1, 5, 62, 0);
	constexpr REL::Version RUNTIME_SSE_1_5_73(1, 5, 73, 0);
	constexpr REL::Version RUNTIME_SSE_1_5_80(1, 5, 80, 0);
	constexpr REL::Version RUNTIME_SSE_1_5_97(1, 5, 97, 0);
	constexpr REL::Version RUNTIME_SSE_1_6_317(1, 6, 317, 0);
	constexpr REL::Version RUNTIME_SSE_1_6_318(1, 6, 318, 0);
	constexpr REL::Version RUNTIME_SSE_1_6_323(1, 6, 323, 0);
	constexpr REL::Version RUNTIME_SSE_1_6_342(1, 6, 342, 0);
	constexpr REL::Version RUNTIME_SSE_1_6_353(1, 6, 353, 0);
	constexpr REL::Version RUNTIME_SSE_1_6_629(1, 6, 629, 0);
	constexpr REL::Version RUNTIME_SSE_1_6_640(1, 6, 640, 0);
	constexpr REL::Version RUNTIME_SSE_1_6_659(1, 6, 659, 0);
	constexpr REL::Version RUNTIME_SSE_1_6_678(1, 6, 678, 0);
	constexpr REL::Version RUNTIME_SSE_1_6_1130(1, 6, 1130, 0);

	constexpr auto RUNTIME_SSE_LATEST_AE = RUNTIME_SSE_1_6_1130;
	constexpr auto RUNTIME_SSE_LATEST_SE = RUNTIME_SSE_1_5_97;
	constexpr auto RUNTIME_SSE_LATEST = RUNTIME_SSE_LATEST_AE;

	constexpr REL::Version RUNTIME_VR_1_4_15(1, 4, 15, 0);
	constexpr auto         RUNTIME_LATEST_VR = RUNTIME_VR_1_4_15;
}

namespace RE {
    class Control_Map :
		public BSTSingletonSDM<Control_Map>      // 00
        {
            public:
                using UEFlag = UserEvents::USER_EVENT_FLAG
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