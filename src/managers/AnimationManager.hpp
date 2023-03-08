#pragma once
#include "events.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts
{
	enum Anim_Stomp {
		GTSstompimpactR, 			// stomp impacts, strongest effect
 		GTSstompimpactL,
		GTSstomplandR, 				// when landing after stomping, decreased power
 		GTSstomplandL,
 		GTSstompstartR, 			// For starting loop of camera shake, air rumble sounds
 		GTSstompstartL,
 		GTSstompendR, 				// disable loop of camera shake, air rumble sounds
 		GTSstompendL,
	};
	
	enum Anim_ThighCrush {
		GTStosit, 					// start air rumble and camera shake
		GTSsitloopenter, 			// Sit down completed
		GTSsitloopstart, 			// Start to spread legs, call air rumble and camera shake. Enable feet damage and knockdown.
 		GTSsitloopend, 				// unused
		GTSsitcrushstart,			// Start strong air rumble and camera shake. Enable strong feet damage and knockdown
		GTSsitcrushend, 			// end it
		GTSsitloopexit, 			// stand up, small air rumble and camera shake
		GTSstandR, 					//feet collides with ground when standing up
		GTSstandL,
		GTStoexit, 					// Leave animation, disable air rumble and such
	};

	enum Anim_Compatibility {
		GTScrush_caster,            //For compatibility with other mods. The gainer.
		GTScrush_victim,            //The one to crush
	};

	class AnimationManager : public EventListener
	{
		public:
			[[nodiscard]] static AnimationManager& GetSingleton() noexcept;

			virtual std::string DebugName() override;
			virtual void Update() override;
			void ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) override;
			void GrabActor(Actor* giant, Actor* tiny, std::string_view findbone);
			void Test(Actor * giant, Actor* tiny);
	};
}
