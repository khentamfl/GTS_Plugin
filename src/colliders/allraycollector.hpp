#pragma once

namespace RE
{
	class hkpAllRayHitCollector : public hkpRayHitCollector
	{
	public:
		inline static constexpr auto RTTI = RTTI_hkpAllRayHitCollector;
		inline static constexpr auto VTABLE = VTABLE_hkpAllRayHitCollector;

		// override (hkpRayHitCollector)
		void AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) override;  // 01

		~hkpClosestRayHitCollector() override;  // 00

    constexpr hkArray<hkpWorldRayCastOutput>& GetHits() noexcept
    {
    	return m_hits;
    }

		constexpr void Reset() noexcept
		{
      m_hits.clear();
			hkpRayHitCollector::Reset();
		}

		// members
		hkInplaceArray<hkpWorldRayCastOutput,8> m_hits;
	};
}
