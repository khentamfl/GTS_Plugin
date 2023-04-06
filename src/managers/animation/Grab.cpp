#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/SizeHitEffects.hpp"
#include "managers/CrushManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"

#include <random>

using namespace RE;
using namespace REL;
using namespace Gts;
using namespace std;

namespace {
	bool Escaped(Actor* giant, Actor* tiny, float strength) {
		float tiny_chance = ((rand() % 100000) / 100000.0f) * get_visual_scale(tiny);
		float giant_chance = ((rand() % 100000) / 100000.0f) * strength * get_visual_scale(giant);
		return (tiny_chance > giant_chance);
	}

	void GrabEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			for (auto otherActor: find_actors()) {
				if (otherActor != player) {
					float playerscale = get_visual_scale(player);
					float victimscale = get_visual_scale(otherActor);
					float sizedifference = playerscale/victimscale;
					NiPoint3 giantLocation = player->GetPosition();
					NiPoint3 tinyLocation = otherActor->GetPosition();
					if ((tinyLocation-giantLocation).Length() < 160*get_visual_scale(player) && sizedifference >= 6.2) {
						Grab::GrabActor(player, otherActor);
						break;
					}
				}
			}
		}
	}

	void GrabKillEvent(const InputEventData& data) {
		auto& sizemanager = SizeManager::GetSingleton();
		auto player = PlayerCharacter::GetSingleton();
		auto grabbedActor = Grab::GetHeldActor(player); 
		if (Runtime::HasPerk(player, "DestructionBasics")) {
		if (grabbedActor) {
			float sd = get_visual_scale(player)/get_visual_scale(grabbedActor);
			float Health = GetAV(grabbedActor, ActorValue::kHealth);
			float power = std::clamp(sizemanager.GetSizeAttribute(player, 0), 1.0f, 999999.0f);
			float additionaldamage = 1.0 + sizemanager.GetSizeVulnerability(grabbedActor);
			float damage = (0.025 * sd) * power * additionaldamage;
			DamageAV(grabbedActor, ActorValue::kHealth, damage);
			SizeHitEffects::GetSingleton().BreakBones(player, grabbedActor, damage * 0.5, 25);
			if (damage > Health * 1.5) {
				CrushManager::Crush(player, grabbedActor);
				PrintDeathSource(player, grabbedActor, "HandCrushed");
				Grab::Release(player);
				}
			}
		}
	}

	void GrabSpareEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (Runtime::HasPerk(player, "DestructionBasics")) {
			Grab::Release(player);
		}
	}
}

///Plans:
///-Commission Grab animation with various events and actions, such as:
/// _ Damage Tiny with fingers (leading to crush), Release (put down), Throw Tiny, Eat Tiny, Escape from Grab

namespace Gts {
	Grab& Grab::GetSingleton() noexcept {
		static Grab instance;
		return instance;
	}

	std::string Grab::DebugName() {
		return "Grab";
	}

	void Grab::Update() {
		for (auto &[giant, data]: this->data) {
			if (!giant) {
				continue;
			}
			auto tiny = data.tiny;
			if (!tiny) {
				continue;
			}

			auto bone = find_node(giant, "NPC L Finger02 [LF02]");
			if (!bone) {
				return;
			}

			float giantScale = get_visual_scale(giant);

			NiPoint3 giantLocation = giant->GetPosition();
			NiPoint3 tinyLocation = tiny->GetPosition();

			tiny->SetPosition(bone->world.translate);
			Actor* tiny_is_actor = skyrim_cast<Actor*>(tiny);
			if (tiny_is_actor) {
				auto charcont = tiny_is_actor->GetCharController();
				if (charcont) {
					charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
				}
			} else {
				// TODO: Work out method for generic objects
			}

			// TODO: Add escape

			////////////////////// Sermit's Note: please add it to Grab only, not Vore. And even then im not sure if we really need it.

			// if Escaped(giant, tiny, data.strength) {
			//   this->data.erase(giant);
			// }
		}
	}


	void Grab::GrabActor(Actor* giant, TESObjectREFR* tiny, float strength) {
		Grab::GetSingleton().data.try_emplace(giant, tiny, strength);
	}
	void Grab::GrabActor(Actor* giant, TESObjectREFR* tiny) {
		// Default strength 1.0: normal grab for actor of their size
		//
		Grab::GrabActor(giant, tiny, 1.0);
	}

	void Grab::Reset() {
		this->data.clear();
	}

	void Grab::ResetActor(Actor* actor) {
		this->data.clear();
	}

	void Grab::Release(Actor* giant) {
		Grab::GetSingleton().data.erase(giant);
	}

	TESObjectREFR* Grab::GetHeldObj(Actor* giant) {
		try {
			auto& me = Grab::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}

	}
	Actor* Grab::GetHeldActor(Actor* giant) {
		auto obj = Grab::GetHeldObj(giant);
		Actor* actor = skyrim_cast<Actor*>(obj);
		if (actor) {
			return actor;
		} else {
			return nullptr;
		}
	}

	void Grab::DataReady()  {
		InputManager::RegisterInputEvent("Grab", GrabEvent);
		InputManager::RegisterInputEvent("GrabKill", GrabKillEvent);
		InputManager::RegisterInputEvent("GrabSpare", GrabSpareEvent);
	}

	GrabData::GrabData(TESObjectREFR* tiny, float strength) : tiny(tiny), strength(strength) {
	}

}
