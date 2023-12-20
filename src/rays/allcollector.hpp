#pragma once

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
  class AllRayCollectorOutput {
    public:
      float        hitFraction{ 1.0F };
      const hkpCollidable* rootCollidable{ nullptr };
      NiPoint3 position;
  };

	class AllRayCollector : public hkpClosestRayHitCollector
	{
		public:
      static unique_ptr<AllRayCollector> Create();

			// Must be a member of one of these groups
			void AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) override;

      constexpr void Reset() noexcept
  		{
        hits.clear();
  			hkpClosestRayHitCollector::Reset();
  		};

      bool HasHit() {
        return !hits.empty();
      }

      std::vector<AllRayCollectorOutput>& GetHits();

			std::vector<AllRayCollectorOutput> hits;
      std::uint32_t filterInfo{ 0 };
	};
}
