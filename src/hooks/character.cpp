#include "hooks/character.h"
#include "util.h"
#include "GtsManager.h"
#include "persistent.h"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace {
	float get_anim_delta(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		auto middlehighprocess = actor->currentProcess->middleHigh;
		if (!middlehighprocess) {
			return 0.0;
		}
		auto thisAGmanager = middlehighprocess->animationGraphManager.get();
		if (!thisAGmanager) {
			return 0.0;
		}

		for (auto boundChannel: thisAGmanager->boundChannels) {
			std::string channelName = boundChannel->channelName.c_str();
			if (channelName == "TimeDelta") {
				float& value = reinterpret_cast<float &>(boundChannel->value);
				return value;
			}
		}
		return 0.0;
	}
}
namespace Hooks
{
	void Hook_Character::Hook() {
		logger::info("Hooking Character");
		REL::Relocation<std::uintptr_t> ActorVtbl{ RE::VTABLE_Character[0] };

		_Update = ActorVtbl.write_vfunc(0xAD, Update);
		_UpdateAnimation = ActorVtbl.write_vfunc(0x7D, UpdateAnimation);
		_ModifyAnimationUpdateData = ActorVtbl.write_vfunc(0x79, ModifyAnimationUpdateData);

		_UpdateNoAI = ActorVtbl.write_vfunc(0xAE, UpdateNoAI);
		_UpdateNonRenderSafe = ActorVtbl.write_vfunc(0xB1, UpdateNonRenderSafe);
		_ProcessTracking = ActorVtbl.write_vfunc(0x122, ProcessTracking);
	}

	void Hook_Character::Update(RE::Character* a_this, float a_delta) {
		log::info("Hook Character Update: {} by {}", actor_name(a_this), a_delta);
		_Update(a_this, a_delta);
		float previous_delta = get_anim_delta(actor);
		if (previous_delta > 1e-5) {
			if (Gts::GtsManager::GetSingleton().enabled) {
				auto saved_data = Gts::Persistent::GetSingleton().GetActorData(a_this);
				if (saved_data) {
					float speed = 0.99; //saved_data->anim_speed;
					if ((speed > 1e-5) && (fabs(speed - 1.0) > 1e-5)) {
						float delta = (speed - 1.0) * previous_delta;
						log::info("Adjusting anim with a delta of {}", delta);
						a_this->UpdateAnimation(delta);
					}
				}
			}
		}
	}
}
