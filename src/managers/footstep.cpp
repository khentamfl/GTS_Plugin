#include "managers/footstep.h"
#include "scale/scale.h"
#include "managers/modevent.h"
#include "util.h"
#include "node.h"
#include "data/runtime.h"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	enum Foot {
		Left,
		Right,
		Front,
		Back,
		Unknown,
	};

	BSISoundDescriptor* get_footstep_sounddesc(Foot foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().FootstepSoundL;
				break;
			case Foot::Right:
			case Foot::Back:
			case Foot::Unknown:
				return Runtime::GetSingleton().FootstepSoundR;
				break;
		}
		return nullptr;
	}
}
namespace Gts {
	FootStepManager& FootStepManager::GetSingleton() noexcept {
		static FootStepManager instance;
		return instance;
	}

	void FootStepManager::HookProcessEvent(BGSImpactManager* impact, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		if (a_event) {
			auto actor = a_event->actor.get().get();
			if (actor) {
				auto base_actor = actor->GetActorBase();
			}
			std::string tag = a_event->tag.c_str();
			auto event_manager = ModEventManager::GetSingleton();
			event_manager.m_onfootstep.SendEvent(actor,tag);

			if (actor->formID != 0x14) return;

			log::info("{} for {}", tag, actor_name(actor));
			// Foot step time
			float scale = get_scale(actor);
			// https://www.desmos.com/calculator/ygoxbe7hjg
			float k = 1.08;
			float n = 0.39;
			float a = 1.2;
			if (scale >= a) {
				float volume = pow(k*(scale-a), n);

				NiAVObject* foot = nullptr;
				Foot foot_kind = Foot::Unknown;
				if (matches(tag, ".*Foot.*Left.*")) {
					foot = find_node_regex_any(actor, ".*(L.*Foot|L.*Leg.*Tip).*");
					foot_kind = Foot::Left;
				} else if (matches(tag, ".*Foot.*Right.*")) {
					foot = find_node_regex_any(actor, ".*(R.*Foot|R.*Leg.*Tip).*");
					foot_kind = Foot::Right;
				} else if (matches(tag, ".*Foot.*Front.*")) {
					foot = find_node_regex_any(actor, ".*((R|L).*Hand|(R|L)b.*Arm.*Tip).*");
					foot_kind = Foot::Front;
				} else if (matches(tag, ".*Foot.*Back.*")) {
					foot = find_node_regex_any(actor, ".*((R|L).*Foot|(R|L)b.*Leg.*Tip).*");
					foot_kind = Foot::Back;
				}
				if (foot && impact) {
					auto audio_manager = BSAudioManager::GetSingleton();
					if (!audio_manager) return;
					auto sound_descriptor = get_footstep_sounddesc(foot_kind);
					if (!sound_descriptor) return;
					BSSoundHandle sound_handle = BSSoundHandle::BSSoundHandle();

					bool sound_sucess = false;
					sound_sucess = audio_manager->BuildSoundDataFromDescriptor(sound_handle, sound_descriptor);

					if (sound_sucess) {
						sound_handle.SetVolume(volume);
						sound_handle.SetFrequency(-volume);
						NiPoint3 pos;
						pos.x = 0;
						pos.y = 0;
						pos.z = 0;
						sound_handle.SetPosition(pos);
						sound_handle.SetObjectToFollow(foot);
						sound_handle.Play();
						shake_camera(actor, 1.5, 0.25);
					}
				}
			}
		}
	}
}
