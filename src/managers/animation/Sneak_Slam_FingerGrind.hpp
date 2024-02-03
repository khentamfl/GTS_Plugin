

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

    void Finger_DamageAndShrink(Actor* giant, float radius, float damage, NiAVObject* node, float random, float bbmult, float crushmult, float Shrink, DamageSource Cause);
	void Laugh_Chance(Actor* giant, Actor* otherActor, float multiply, std::string_view name);
	void Laugh_Chance(Actor* giant, float multiply, std::string_view name);
    void TrackMatchingHand(Actor* giant, CrawlEvent kind, bool enable);
    void StopStaminaDrain(Actor* giant);
}