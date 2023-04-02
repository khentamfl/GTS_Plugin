// Animation: Stomp
//  - Stages
/*"GTSSandwich_EnterAnim"  // Animation was just fired
"GTSSandwich_SitStart" // When sit start happens
"GTSSandwich_EnableRune"  // Trigger the ture
"GTSSandwich_MoveBody_start" // Rumble for entire body
"GTSSandwich_MoveBody_end"   // Stop them (When body pretty much enters 'idle' state)
"GTSSandwich_ThighLoop_Enter" // Enter Thigh Idle Loop
"GTSSandwich_MoveLL_start"   // Left leg starts to move in space (When starting to crush)
"GTSSandwich_MoveLL_end"     // Left leg isn't moving anymore (These 2 should be repeated)
"GTSSandwich_ThighAttack_start" // When we trigger Thigh Attack
"GTSSandwich_ThighImpact"    // When Both thighs are supposed to deal damage to an actor (When 'Sandwiching') happens
"GTSSandwich_ThighLoop_Exit" // Exit thigh idle loop
"GTSSandwich_DisableRune"    // Remove Rune
"GTSSandwich_DropDown"      // When actor starts to 'jump off' from Rune
"GTSSandwich_FootImpact"    // When both feet collide with the ground
"GTSSandwich_ExitAnim"      // Animation is over
*/

#include "managers/animation/ThighSandwich.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
}

namespace Gts
{
	void AnimationThighSandwich::RegisterEvents() {
		/*AnimationManager::RegisterEvent("GTSstompimpactR", "Stomp", GTSstompimpactR);
		AnimationManager::RegisterEvent("GTSstompimpactL", "Stomp", GTSstompimpactL);
		AnimationManager::RegisterEvent("GTSstomplandR", "Stomp", GTSstomplandR);
		AnimationManager::RegisterEvent("GTSstomplandL", "Stomp", GTSstomplandL);
		AnimationManager::RegisterEvent("GTSstompstartR", "Stomp", GTSstompstartR);
		AnimationManager::RegisterEvent("GTSstompstartL", "Stomp", GTSstompstartL);
		AnimationManager::RegisterEvent("GTSStompendR", "Stomp", GTSStompendR);
		AnimationManager::RegisterEvent("GTSStompendL", "Stomp", GTSStompendL);
		AnimationManager::RegisterEvent("GTS_Next", "Stomp", GTS_Next);
		AnimationManager::RegisterEvent("GTSBEH_Exit", "Stomp", GTSBEH_Exit);

		InputManager::RegisterInputEvent("RightStomp", RightStompEvent);
		InputManager::RegisterInputEvent("LeftStomp", LeftStompEvent);*/
	}

	void AnimationThighSandwich::RegisterTriggers() {
		//AnimationManager::RegisterTrigger("StompRight", "Stomp", "GtsModStompAnimRight");
		//AnimationManager::RegisterTrigger("StompLeft", "Stomp", "GtsModStompAnimLeft");
	}
}