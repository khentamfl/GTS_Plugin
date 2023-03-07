#include "managers/AnimationManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/GrowthTremorManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/impact.hpp"
#include "magic/effects/common.hpp"
#include "managers/GtsManager.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "events.hpp"
#include "timer.hpp"
#include "node.hpp"


using namespace RE;
using namespace Gts;


namespace Gts {
	AnimationManager& AnimationManager::GetSingleton() noexcept {
		static AnimationManager instance;
		return instance;
	}

	std::string AnimationManager::DebugName() {
		return "AnimationManager";
	}

	void AnimationManager::ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload) {
        if (actor->formID == 0x14) {
			auto scale = get_visual_scale(actor);
			float volume = scale * 0.20;
            if (tag == "GTSstompimpactR" || tag == "GTSstompimpactL") {
				//Call UnderFoot event here somehow with x scale bonus
				Runtime::PlaySound("lFootstepL", actor, volume, 1.0);
            } 
			if (tag == "GTSstomplandL" || tag == "GTSstomplandR") {
				Runtime::PlaySound("lFootstepL", actor, volume * 0.5, 1.0);
            }
        }
    }

	void AnimationManager::GrabActor(Actor* giant, Actor* tiny, std::string_view findbone) {
		if (giant == tiny) {
			return;
		}
		auto bone = find_node(giant, findbone);
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
	void AnimationManager::Test(Actor * giant, Actor* tiny) {
		if (giant != tiny) {
			return;
		}
		float giantScale = get_visual_scale(giant);
		NiPoint3 giantLocation = giant->GetPosition();
		NiPoint3 tinyLocation = tiny->GetPosition();
		auto charCont = tiny->GetCharController();
		if ((tinyLocation-giantLocation).Length() < 460*giantScale) {
			if (charCont) {
				hkVector4 vel;
			//auto tinyai = tiny->GetActorRuntimeData().currentProcess->high;
			//if (tinyai) {
				log::info("Velocity of {} = x{}, y{}, z{}, w{}", tiny->GetDisplayFullName(), charCont->GetLinearVelocityImpl(vel).quad.m128_f32[0], charCont->GetLinearVelocityImpl(vel).quad.m128_f32[1], charCont->GetLinearVelocityImpl(vel).quad.m128_f32[2], charCont->GetLinearVelocityImpl(vel).quad.m128_f32[3]);
			//}
			}
		}
	}
}