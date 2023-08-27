#include "utils/DeathReport.hpp"

using namespace RE;
using namespace Gts;


namespace Gts {
	void ReportDeath(Actor* giant, Actor* tiny, DamageSource cause) {
        int random = rand()% 8;
		float sizedifference = get_visual_scale(giant)/get_visual_scale(tiny);
		if (cause == DamageSource::Crushed) { // Default crush by the feet
			if (random < 2) {
				Cprint("{} became a bloody stain under {} foot.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} was crushed by the feet of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("Feet of {} crushed {} into nothing", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} step on {} too hard", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} got crushed by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} ended up being crushed by the {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} relentlessly crushed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::HandCrushed) { // when Grab -> Crush happens
			if (random == 1) {
				Cprint("{} life was squeezed out in {} grip", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} Squeezed her palm, unexpectedly crushing {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} was transformed into bloody mist by the tight grip of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} has been crushed inside the hand of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} applied too much pressure to her hand, crushing {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} was turned into pulp by the palm of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::Collision) { // Through Collision
			if (random <= 2) {
				Cprint("{} exploded after colliding with {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} completely absorbed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} was absorbed by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} was shrinkned to nothing by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} size was completely drained by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} stole all the size from {}, exploding {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
		} 
		else if (cause == DamageSource::Shrinked) { // Shrink to nothing
			if (random <= 2) {
				Cprint("{} greedily absorbed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} completely absorbed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} was absorbed by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} was shrinkned to nothing by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} size was completely drained by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} stole all the size from {}, exploding {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::Vored) {
			///nothing for now
			return;
		} else if (cause == "ThighCrushed") { // During thigh crush
			if (random == 1) {
				Cprint("{} was crushed to death between {} thighs.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			if (random <= 3) {
				Cprint("{} crushed {} during leg stretch", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} ended life of {} between legs", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} applied too much leg pressure to {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} was shrinkned to nothing by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::ThighSandwiched) { // Self explanatory
			if (random <= 3) {
				Cprint("{} was crushed by the thighs of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("Thighs of {} gently crushed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} has disappeared between the thighs of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} was smothered to nothing between the thighs of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("Thighs of {} sandwiched {} to nothing", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::Overkill) {  // When we hit actor with too much weapon damage while being huge
			if (random == 1) {
				Cprint("{} body exploded because of massive size difference with {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} hit {} with so much force that {} exploded into bloody mist", giant->GetDisplayFullName(), tiny->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} was pulverized into nothing by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("All that's left from {} after being hit bit {} is a bloody mist", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} couldn't handle enormous hit from {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} put so much force into attack that {} turned into red mist", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::HitSteal) { // Hit Growth perk
			if (random <= 2) {
				Cprint("{} body exploded after trying to hit {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("Protective magic of {} made {} absorb {}", giant->GetDisplayFullName(), giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random > 3) {
				Cprint("{} Tried to kill {}, but ended up being absorbed by the size magic of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::Explode) { // Poison Of Shrinking
			if (random <= 2) {
				Cprint("{} exploded into bloody dust", tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} suddenly exploded", tiny->GetDisplayFullName());
			} else if (random > 3) {
				Cprint("{} was turned into nothing", tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::BlockDamage) { // WHen tiny in hand receives too much damage
			if (random == 1) {
				Cprint("{} received too much damage and was automatically crushed in the hands of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} was crushed from receiving too much damage {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} stopped to be useful, so he was turned into bloody mist in the hands of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} took extreme amounts of damage and exploded inside the hands of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} took a little more damage than intended, so her fingers ended up squeezing {} into nothing", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} blocked too much damage and was squeezed into bloody stain by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::Breasts) { // Someone died between breasts
			if (random == 1) {
				Cprint("{} was weakened and got accidentally crushed by {} breasts", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} got unintentionally crushed by the breasts of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} left this world by being crushed between the cleavage of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("Breasts of {} squeezed all life out of {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 6) {
				Cprint("{} took some damage and ended up crushing {} between her breasts", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} got smothered by soft breasts of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::FootGrinded) { // Grinded by the foot. Currently doesn't exist. It is planned to add it.
			if (random < 2) {
				Cprint("{} became a bloody stain under {} foot.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} was crushed by the feet of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3 || random == 4) {
				Cprint("Feet of {} crushed {} into nothing", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5 || random == 6) {
				Cprint("{} got crushed by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} relentlessly crushed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::Melted) { // Melted by tongue. Currently doesn't exist. Possible Vore variation with melting actors with tongue instead.
			if (random < 2) {
				Cprint("{} was melted by the tongue of {}.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} got absorbed by the tongue of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3 || random == 4) {
				Cprint("Hot tongue of {} melted {} like a candy", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5 || random == 6) {
				Cprint("{} was (un)forunate enough to be melted by the tongue of {} ", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("Tongue of {} sucked all life out of {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::Booty) { // Butt Crushed. Currently doesn't exist. It is planned though.
			if (random < 2) {
				Cprint("{} got crushed by the butt of {}.", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} was overwhelmed by the booty of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("Bootie of {} completely pulverized {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("Booty of {} completely pulverized {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} has been squashed by butt attack of {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} cheeks dropped on {}, turning {} into red paste", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}else if (random >= 7) {
				Cprint("{} relentlessly crushed {} with butt attack", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::Hugs) { // For Hug Crush
			if (random < 2) {
				Cprint("{} was hugged to death by {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} got too carried away hugging {}, crushing {} as a result", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} applied too much force to the hugs, killing {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} couldn't resist hug crushing {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("{} failed to escape hugs of death with {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} got greedy and stole all size from {} through hugs", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} gently hug crushed {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			}
			return;
		} else if (cause == DamageSource::Crawl) { // For being crushed while GTS crawls
			if (random < 2) {
				Cprint("{} was accidentally crushed by {} that was crawling around", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 2) {
				Cprint("{} was crawling around and ended up killing {}", giant->GetDisplayFullName(), tiny->GetDisplayFullName());
			} else if (random == 3) {
				Cprint("{} ended up being in a wrong place and in the wrong time", tiny->GetDisplayFullName());
			} else if (random == 4) {
				Cprint("{} couldn't survive being under crawling {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random == 5) {
				Cprint("For some reason something has crunched under {}", giant->GetDisplayFullName());
			} else if (random == 6) {
				Cprint("{} couldn't survive under {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			} else if (random >= 7) {
				Cprint("{} didn't realize that it's extremely dangerous to be under {}", tiny->GetDisplayFullName(), giant->GetDisplayFullName());
			}
			return;
		}
    }
}