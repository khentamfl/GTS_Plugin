#include "hooks/character.h"
#include "util.h"
#include "GtsManager.h"
#include "persistent.h"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace {
	void experiment003(Actor* actor) {
		if (!actor) {
			return;
		}
		auto middlehighprocess = actor->currentProcess->middleHigh;
		if (!middlehighprocess) {
			return;
		}
		auto thisAGmanager = middlehighprocess->animationGraphManager.get();
		if (!thisAGmanager) {
			return;
		}

		log::info("Actor {} bound channels", actor_name(actor));
		for (auto boundChannel: thisAGmanager->boundChannels) {
			std::string channelName = boundChannel->channelName.c_str();
			log::info("  - channelName: {}", channelName);
			log::info("  - Value (int): {}", boundChannel->value);
			log::info("  - Value (float): {}", reinterpret_cast<float &>(boundChannel->value));
			if (channelName == "TimeDelta") {
				float& value = reinterpret_cast<float &>(boundChannel->value);
				value *= 0.12;
				log::info("  - New Value (float) multipled by 0.12: {}", reinterpret_cast<float &>(boundChannel->value));
			}
		}
		log::info("Actor {} bumped channels", actor_name(actor));
		for (auto bumpedChannel: thisAGmanager->bumpedChannels) {
			std::string channelName = bumpedChannel->channelName.c_str();
			log::info("  - channelName: {}", channelName);
			log::info("  - Value (int): {}", bumpedChannel->value);
			log::info("  - Value (float): {}", reinterpret_cast<float &>(bumpedChannel->value));
		}
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
		a_this->SetGraphVariableBool("bAnimationDriven", true);
	}

	void Hook_Character::UpdateAnimation(RE::Character* a_this, float a_delta) {
		log::info("Hook Character Anim: {} by {}", actor_name(a_this), a_delta);
		_UpdateAnimation(a_this, a_delta);
	}

	void Hook_Character::ModifyAnimationUpdateData(RE::Character* a_this, BSAnimationUpdateData& a_data) {
		log::info("Hook Character Anim Update Modify: {}", actor_name(a_this));
		_ModifyAnimationUpdateData(a_this, a_data);
	}

	void Hook_Character::UpdateNoAI(RE::Character* a_this, float a_delta) {
		log::info("Hook Character UpdateNoAI: {} by {}", actor_name(a_this), a_delta);
		_UpdateNoAI(a_this, a_delta);
	}

	void Hook_Character::UpdateNonRenderSafe(RE::Character* a_this, float a_delta) {
		log::info("Hook Character UpdateNonRenderSafe: {} by {}", actor_name(a_this), a_delta);
		_UpdateNonRenderSafe(a_this, a_delta);
		// NPC have their update time done here lets see what happens if we queue
		// an anim update
		// if (a_delta > 1e-5) {
		// 	log::info("  - Anim driven: {} for {}", a_this->IsAnimationDriven(), actor_name(a_this));
		// 	a_this->UpdateAnimation(a_delta);
		// }
	}

	void Hook_Character::ProcessTracking(RE::Character* a_this, float a_delta, NiAVObject* a_obj3D) {
		log::info("Hook Character ProcessTracking: {} by {}", actor_name(a_this), a_delta);
		_ProcessTracking(a_this, a_delta, a_obj3D);
	}

}
