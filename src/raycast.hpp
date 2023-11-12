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
			void add_filter(NiObject* obj) noexcept;
			bool is_filtered(NiObject* obj);
			bool is_filtered_av(NiAVObject* obj);
			void AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) override;

			std::vector<NiObject*> object_filter;
			std::vector<HitResult> results;
			std::uint32_t filter = 0x40122716; // https://gitlab.com/Shrimperator/skyrim-mod-betterthirdpersonselection/-/blob/main/src/SmoothCamStuff/bhkLinearCastCollector.h
	};

	NiPoint3 CastRay(Actor* actor, NiPoint3 origin, NiPoint3 direction, float length, bool& success);
	NiPoint3 CastRay(TESObjectREFR* ref, NiPoint3 origin, NiPoint3 direction, float length, bool& success);
}
