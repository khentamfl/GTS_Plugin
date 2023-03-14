#include "managers/animation/Animation_Grab.hpp"
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

/*namespace Gts {
        Grab& Grab::GetSingleton() noexcept {
                static Grab instance;
                return instance;
        }

        std::string Grab::DebugName() {
                return "Grab";
        }

        void Grab::Update() {
        auto PC = PlayerCharacter::GetSingleton();
        if (PC) {
            for (auto &[tiny, data]: this->data) {
                if (!tiny) {
                    return;
                }
                if (tiny->formID == 0x14) {
                    return;
                }
                auto giant = PC;
                auto bone = find_node(PC, "NPC L Finger02 [LF02]");
                        if (!bone) {
                                return;
                        }
                        float giantScale = get_visual_scale(giant);
                        NiAVObject* attach = bone;
                        NiPoint3 giantLocation = giant->GetPosition();
                        NiPoint3 tinyLocation = tiny->GetPosition();
                            TESObjectREFR* ref = static_cast<TESObjectREFR*>(tiny);
                            ref->SetPosition(attach->world.translate);
                            tiny->SetPosition(attach->world.translate, false);
                            auto charcont = tiny->GetCharController();
                        if (charcont) {
                                charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down. 
                    }
            }
        }
    }


        void Grab::GrabActor(Actor* tiny) {
        auto& check = Grab::GetSingleton().data;
        check.try_emplace(tiny);
    }

    void Grab::CrushActors() {
        auto& check = Grab::GetSingleton().data;
        auto player = PlayerCharacter::GetSingleton();
        for (auto &[tiny, data]: this->data) {
            CrushManager::GetSingleton().Crush(player, tiny);
            CrushBonuses(player, tiny, 2);
        }
        check.clear();
    }

    void Grab::Clear() {
        auto& check = Grab::GetSingleton().data;
        check.clear();
    }

    GrabData::GrabData(Actor* tiny) :
                tiny(tiny) {
        }
   }*/

