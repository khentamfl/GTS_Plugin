#pragma once
// Misc codes
#include <SKSE/SKSE.h>
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
			void add_filter(NiAVObject* obj) noexcept;
			bool is_filtered(NiAVObject* obj);
			void AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) override;

			std::vector<NiAVObject*> object_filter;
			std::vector<HitResult> results;
			std::uint32_t filter = 0x40122716; // https://gitlab.com/Shrimperator/skyrim-mod-betterthirdpersonselection/-/blob/main/src/SmoothCamStuff/bhkLinearCastCollector.h
	};

	NiPoint3 CastRay(Actor* actor, NiPoint3 origin, NiPoint3 direction, float length, bool& success);
}
