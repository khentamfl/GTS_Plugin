

#pragma once

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	class Animation_SneakSlam_FingerGrind {
		public:
			static void RegisterEvents();
	};

    void ApplyFingerDamage(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult, float crushmult, float Shrink, DamageSource Cause);
	
    void TrackMatchingHand(Actor* giant, CrawlEvent kind, bool enable);
    void StopStaminaDrain(Actor* giant);
}