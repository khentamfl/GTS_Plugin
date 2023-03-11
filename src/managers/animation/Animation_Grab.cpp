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

using namespace RE;
using namespace Gts;
using namespace std;

namespace Gts {
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
            if (!this->GrabData.tiny) {
                return;
            } else 
            {
                if (this->GrabData.tiny->formId == 0x14) {
                    return;
                }
                auto giant = PC;
                auto bone = GrabData.node;
		        if (!bone) {
			        return;
		        }
		        float giantScale = get_visual_scale(giant);
		        NiAVObject* attach = bone;
		        NiPoint3 giantLocation = giant->GetPosition();
		        NiPoint3 tinyLocation = tiny->GetPosition();
		        if ((tinyLocation-giantLocation).Length() < 460*giantScale) {
			        TESObjectREFR* ref = static_cast<TESObjectREFR*>(tiny);
			        ref->SetPosition(attach->world.translate);
			        tiny->SetPosition(attach->world.translate, false);
			        auto charcont = tiny->GetCharController();
			        if (charcont) {
				        if (charcont->gravity > 0.0) {
					    //log::info("Gravity of {} = {}", tiny->GetDisplayFullName(), charcont->gravity);
					    //charcont->gravity = 0.0;
					    charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0));
				        }
			        }
		        }
            }
        }
	}

	void Grab::GrabActor(Actor* tiny, std::string_view findbone) {
        if (!this->data.tiny) {
            data.try_emplace(tiny, findbone);
        }
    }

    void Grab::CrushActors() {
        if (this->data.tiny) {
            auto player == PlayerCharacter::GetSingleton();
            for (auto victims: data.tiny) {
                CrushManager::GetSingleton().Crush(player, victims);
            }
            data.clear();
        }
    }

    void Grab::Clear() {
        if (this->data.tiny) {
            data.clear();
        }
    }

    GrabData::GrabData(Actor* tiny) :
		std::string_view(node),
		tiny(tiny) {
	}
}

