#pragma once

#include "profiler.hpp"

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
    const float Action_Sandwich = 6.0f; // used for sandwich only
	const float Action_Crush = 10.0f;
	const float Action_Booty = 2.0f; // for butt and cleavage crush
	const float Action_Vore = 8.0f;
	const float Action_Grab = 8.0f;
	const float Action_Hug = 0.9f;

///////Trigger Thresholds ^
////// Default damage values
    const float Damage_Default_Idle = 0.4f; // when we just stand still

    const float Damage_Walk_Defaut = 6.0f; // when we walk around normally
    const float Damage_Jump_Default = 16.0f; // when we jump land

	const float Damage_Stomp = 10.0f;
	const float Damage_Stomp_Strong = 20.0f;

	const float Damage_Trample = 8.0f;
	const float Damage_Trample_Repeat = 10.0f;
	const float Damage_Trample_Finisher = 45.0f;

	const float Damage_ButtCrush = 100.0f;

	const float Damage_BreastCrush = 100.0f;
    const float Damage_BreastCrush_Body = 50.0f; // for body impact
    const float Damage_BreastCrush_DOT = 0.6f;

	const float Damage_KneeCrush = 100.0f;

	const float Damage_Kick = 8.0f;
	const float Damage_Kick_Strong = 18.0f;

	const float Damage_Crawl_HandSwipe = 6.0f;
	const float Damage_Crawl_HandSwipe_Strong = 10.0f;

    const float Damage_Crawl_HandSlam = 8.0f;
    const float Damage_Crawl_HandSlam_Strong = 16.0f;

	const float Damage_Sneak_HandSwipe = 5.0f;
	const float Damage_Sneak_HandSwipe_Strong = 14.0f;

//////Default effect radius variables

    const float Radius_Walk_Default = 9.8f;
    const float Radius_Jump_Default = 22.0f; 

    const float Radius_Stomp = 12.0f;
    const float Radius_Stomp_Strong = 13.2f;

    const float Radius_Trample = 10.8f;
    const float Radius_Trample_Repeat = 12.0f;
    const float Radius_Trample_Finisher = 13.2f;

    const float Radius_ButtCrush = 22.0f;
    const float Radius_ButtCrush_HandImpact = 8.0f;

    const float Radius_BreastCrush_BodyImpact = 20.0f;
    const float Radius_BreastCrush_BreastImpact = 20.0f;
    const float Radius_BreastCrush_DOT = 14.0f; 

    const float Radius_Crawl_HandSwipe = 18.0f;
    const float Radius_Crawl_KneeImpact = 14.0f;
    const float Radius_Crawl_HandImpact = 12.0f;

    const float Radius_Crawl_KneeImpact_Fall = 18.0f;
    const float Radius_Crawl_HandImpact_Fall = 14.0f;

    const float Radius_Crawl_Slam = 12.0f;
    const float Radius_Crawl_Slam_Strong = 13.0f;

    const float Radius_Sneak_HandSwipe = 18.0f;
    const float Radius_Sneak_KneeCrush = 20.0f;
    const float Radius_Sneak_HandSlam = 12.0f;
    const float Radius_Sneak_HandSlam_Strong = 13.0f;

    const float Radius_Kick = 22.0f;

    const float Radius_ThighCrush_ButtImpact = 16.0f;

    const float Radius_Sneak_Vore_ButtImpact = 20.0f;
}