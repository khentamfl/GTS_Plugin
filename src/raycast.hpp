#pragma once
// Misc codes
#include <math.h>
#include <regex>

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	struct HitResult {
		const hkpShape* shape;
		float fraction;
	};

	class RayCollector : public hkpClosestRayHitCollector
	{
		public:
      // Must be a member of one of these groups
      void add_group_filter(COL_LAYER group) noexcept;

			void add_filter(NiObject* obj) noexcept;
			bool is_filtered(NiObject* obj);
			bool is_filtered_av(NiAVObject* obj);
			void AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) override;

			std::vector<NiObject*> object_filter;
      std::vector<HitResult> results;
			std::vector<COL_LAYER> groups;
			std::uint32_t filter = 0x40122716; // https://gitlab.com/Shrimperator/skyrim-mod-betterthirdpersonselection/-/blob/main/src/SmoothCamStuff/bhkLinearCastCollector.h
	};

  NiPoint3 CastRay(TESObjectREFR* ref, NiPoint3 origin, NiPoint3 direction, float length, std::vector<COL_LAYER> groups, bool& success);
	NiPoint3 CastRay(TESObjectREFR* ref, NiPoint3 origin, NiPoint3 direction, float length, bool& success);
}
