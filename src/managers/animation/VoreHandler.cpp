	
#include "managers/animation/VoreHandler.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/CrushManager.hpp"
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

	void AdjustGiantessSkill(Actor* Caster, Actor* Target) { // Adjust Matter Of Size skill
		if (Caster->formID !=0x14) {
			return; //Bye
		}
		auto GtsSkillLevel = Runtime::GetGlobal("GtsSkillLevel");
		if (!GtsSkillLevel) {
			return;
		}
		auto GtsSkillRatio = Runtime::GetGlobal("GtsSkillRatio");
		if (!GtsSkillRatio) {
			return;
		}
		auto GtsSkillProgress = Runtime::GetGlobal("GtsSkillProgress");
		if (!GtsSkillProgress) {
			return;
		}

		int random = (100 + (rand()% 85 + 1)) / 100;

		if (GtsSkillLevel->value >= 100) {
			GtsSkillLevel->value = 100.0;
			GtsSkillRatio->value = 0.0;
			return;
		}

		float ValueEffectiveness = std::clamp(1.0 - GtsSkillLevel->value/100, 0.20, 1.0);

		float absorbedSize = (get_visual_scale(Target));
		float oldvaluecalc = 1.0 - GtsSkillRatio->value; //Attempt to keep progress on the next level
		float Total = (((0.68 * random) + absorbedSize/50) * ValueEffectiveness);
		GtsSkillRatio->value += Total;

		if (GtsSkillRatio->value >= 1.0) {
			float transfer = clamp(0.0, 1.0, Total - oldvaluecalc);
			GtsSkillLevel->value += 1.0;
			GtsSkillProgress->value = GtsSkillLevel->value;
			GtsSkillRatio->value = 0.0 + transfer;
		}
	}

	void VoreMessage(Actor* pred, Actor* prey) {
        int random = rand() % 2;
		if (!prey->IsDead() && !Runtime::HasPerk(pred, "SoulVorePerk") || random == 0) {
			ConsoleLog::GetSingleton()->Print("%s was Eaten Alive by %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (!prey->IsDead() && Runtime::HasPerk(pred, "SoulVorePerk") && random == 1) {
			ConsoleLog::GetSingleton()->Print("%s became one with %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (!prey->IsDead() && Runtime::HasPerk(pred, "SoulVorePerk") && random == 2) {
			ConsoleLog::GetSingleton()->Print("%s both body and soul were devoured by %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (prey->IsDead()) {
			ConsoleLog::GetSingleton()->Print("%s Was Eaten by %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		}
    }
}

namespace Gts {
	VoreHandler& VoreHandler::GetSingleton() noexcept {
		static VoreHandler instance;
		return instance;
	}

	std::string VoreHandler::DebugName() {
		return "VoreHandler";
	}

	void VoreHandler::Update() {
        //Attach actor to "AnimObjectA" node on Giant
        for (auto &[giant, data]: this->data) {
			if (!giant) {
				continue;
			}
			auto tiny = VoreHandler::GetSingleton().GetHeldVoreObj(giant);
			if (!tiny) {
				continue;
			}

			auto bone = find_node(giant, "AnimObjectA");
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
			}
        }
    }
    
    void VoreHandler::EatActors(Actor* giant) {
        for (auto &[giant, data]: VoreHandler::GetSingleton().data) {
            auto tiny = VoreHandler::GetSingleton().GetHeldVoreObj(giant);
			float rate = 1.0;
			if (Runtime::HasPerkTeam(giant, "AdditionalAbsorption")) {
				rate = 2.0;
			}
			AdjustGiantessSkill(giant, tiny);
			VoreMessage(giant, tiny);
			mod_target_scale(giant, 0.30 * get_visual_scale(tiny));
			if (tiny->formID != 0x14) {
				Disintegrate(tiny); // Player can't be disintegrated: simply nothing happens.
			} else if (tiny->formID == 0x14) {
				TriggerScreenBlood(50);
				tiny->SetAlpha(0.0); // Just make player Invisible
			}
            VoreHandler::GetSingleton().ClearData(giant);
        }
        ///Will do same stuff that the Scripts do here, mainly heal gainer and increase size, as well as other stuff i think.
        ///Would be nice to do stuff based on time passed, but that's probably too tedious to do (Since Script uses Utilit.wait(time) to do something based on delay)
    }

    void VoreHandler::GrabVoreActor(Actor* giant, Actor* tiny) {
        //Add Actor(s) to data so Update will manage it
        VoreHandler::GetSingleton().data.try_emplace(giant, tiny);
    }

    void VoreHandler::ClearData(Actor* giant) {
		VoreHandler::GetSingleton().data.erase(giant);
	}

    Actor* VoreHandler::GetHeldVoreActors(Actor* giant) {
        try {
			auto& me = VoreHandler::GetSingleton();
			return me.data.at(giant).tiny;
		} catch (std::out_of_range e) {
			return nullptr;
		}
	}

    VoreData::VoreData(Actor* tiny) : giant(giant) {
	}
}

