#pragma once

#include "profiler.hpp"

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
//-----------------------------------------Default Trigger Threshold Values
    const float Action_Sandwich = 6.0f; // used for sandwich only
	const float Action_Crush = 10.0f;
	const float Action_Booty = 2.0f; // for butt and cleavage crush
	const float Action_Vore = 8.0f;
	const float Action_Grab = 8.0f;
	const float Action_Hug = 0.9f;

//-----------------------------------------Default Damage Values

    const float Damage_Grab_Attack = 1.6f;
    ////////////////Defaults

    const float Damage_Default_Idle = 0.04f; // when we just stand still

    const float Damage_Walk_Defaut = 6.0f; // when we walk around normally
    const float Damage_Jump_Default = 16.0f; // when we jump land

	const float Damage_Stomp = 8.0f;
	const float Damage_Stomp_Strong = 18.0f;

    const float Damage_Foot_Grind_Impact = 6.8f;
    const float Damage_Foot_Grind_DOT = 0.04f;

    ////////////////Trample

	const float Damage_Trample = 5.0f;
	const float Damage_Trample_Repeat = 6.0f;
	const float Damage_Trample_Finisher = 24.0f;

    ////////////////Butt Crush

	const float Damage_ButtCrush_ButtImpact = 50.0f;
    const float Damage_ButtCrush_HandImpact = 6.0f;

    const float Damage_ButtCrush_FootImpact = 6.0f;

    ////////////////Thigh Crush
    const float Damage_ThighCrush_Stand_Up = 7.0f;
    const float Damage_ThighCrush_Butt_DOT = 0.06f;

    ////////////////breast

    const float Damage_BreastCrush_Body = 40.0f; // for body impact
	const float Damage_BreastCrush_Impact = 50.0f; // when doing impact
    const float Damage_BreastCrush_BodyDOT = 0.04f; // damage under body
    const float Damage_BreastCrush_BreastDOT = 0.06f; // damage under breasts

    ////////////////Knee

	const float Damage_KneeCrush = 44.0f;

    ////////////////kick

	const float Damage_Kick = 8.0f;
	const float Damage_Kick_Strong = 16.0f;

    ////////////////crawl

    const float Damage_Crawl_Idle = 0.04f;

    const float Damage_Crawl_KneeImpact_Drop = 14.0f;
    const float Damage_Crawl_HandImpact_Drop = 10.0f;

    const float Damage_Crawl_KneeImpact = 8.0f;
    const float Damage_Crawl_HandImpact = 6.0f;

	const float Damage_Crawl_HandSwipe = 7.0f;
	const float Damage_Crawl_HandSwipe_Strong = 14.0f;

    const float Damage_Crawl_HandSlam = 10.0f;
    const float Damage_Crawl_HandSlam_Strong = 18.0f;

    const float Damage_Crawl_Vore_Butt_Impact = 50.0f;

    ////////////////sneak

	const float Damage_Sneak_HandSwipe = 5.0f;
	const float Damage_Sneak_HandSwipe_Strong = 14.0f;

    

//-----------------------------------------Default effect radius variables
    const float Radius_Default_Idle = 7.0f;

    const float Radius_Walk_Default = 7.0f;
    const float Radius_Jump_Default = 12.0f; 

    const float Radius_Stomp = 7.4f;
    const float Radius_Stomp_Strong = 7.4f;

    const float Radius_Foot_Grind_Impact = 7.4f;
    const float Radius_Foot_Grind_DOT = 7.2f;

    /////////Foot Trample
    const float Radius_Trample = 7.0f;
    const float Radius_Trample_Repeat = 7.4f;
    const float Radius_Trample_Finisher = 8.0f;

    /////////Butt Crush

    const float Radius_ButtCrush_Impact = 22.0f;
    const float Radius_ButtCrush_HandImpact = 8.0f;
    const float Radius_ButtCrush_FootImpact = 7.2f;

    /////////Thigh Crush
    const float Radius_ThighCrush_Butt_DOT = 13.0f;

    const float Radius_BreastCrush_BodyImpact = 21.0f;
    const float Radius_BreastCrush_BreastImpact = 20.0f;
    const float Radius_BreastCrush_BodyDOT = 15.0f; 
    const float Radius_BreastCrush_BreastDOT = 14.0f; 

    ////////Crawling
    const float Radius_Crawl_HandSwipe = 18.0f;
    const float Radius_Crawl_KneeImpact = 14.0f;
    const float Radius_Crawl_HandImpact = 12.0f;

    const float Radius_Crawl_KneeImpact_Fall = 18.0f;
    const float Radius_Crawl_HandImpact_Fall = 14.0f;

    const float Radius_Crawl_Slam = 12.0f;
    const float Radius_Crawl_Slam_Strong = 13.0f;

    const float Radius_Crawl_KneeIdle = 9.0f;
    const float Radius_Crawl_HandIdle = 8.8f;

    const float Radius_Crawl_Vore_ButtImpact = 20.0f;

    ///////Sneaking
    const float Radius_Sneak_HandSwipe = 18.0f;
    const float Radius_Sneak_KneeCrush = 20.0f;
    const float Radius_Sneak_HandSlam = 12.0f;
    const float Radius_Sneak_HandSlam_Strong = 13.0f;

    ///////Kicks
    const float Radius_Kick = 16.0f;

    //////Thigh Crush
    const float Radius_ThighCrush_ButtImpact = 16.0f;
    const float Radius_ThighCrush_Stand_Up = 7.0f;

    //////Sneak Vore
    
}