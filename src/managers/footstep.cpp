#include "managers/footstep.h"
#include "scale/scale.h"
#include "managers/modevent.h"
#include "data/persistent.h"
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
		JumpLand,
		Unknown,
	};

	Foot get_foot_kind(std::string_view tag) {
		Foot foot_kind = Foot::Unknown;
		if (matches(tag, ".*Foot.*Left.*")) {
			foot_kind = Foot::Left;
		} else if (matches(tag, ".*Foot.*Right.*")) {
			foot_kind = Foot::Right;
		} else if (matches(tag, ".*Foot.*Front.*")) {
			foot_kind = Foot::Front;
		} else if (matches(tag, ".*Foot.*Back.*")) {
			foot_kind = Foot::Back;
		} else if (matches(tag, ".*Jump.*(Down|Land).*")) {
			foot_kind = Foot::JumpLand;
		}
		return foot_kind;
	}

	NiAVObject* get_landing_foot(Actor* actor, const Foot& foot_kind) {
		NiAVObject* result = nullptr;
		switch (foot_kind) {
			case Foot::Left:
				result = find_node_regex_any(actor, ".*(L.*Foot|L.*Leg.*Tip).*");
				break;
			case Foot::Right:
				result = find_node_regex_any(actor, ".*(R.*Foot|R.*Leg.*Tip).*");
				break;
			case Foot::Front:
				result = find_node_regex_any(actor, ".*((R|L).*Hand|(R|L)b.*Arm.*Tip).*");
				break;
			case Foot::Back:
				result = find_node_regex_any(actor, ".*((R|L).*Foot|(R|L)b.*Leg.*Tip).*");
				break;
			case Foot::JumpLand:
				// Whatever it find first
				result = find_node_regex_any(actor, ".*((R|L).*(Foot|Hand)|(R|L)b.*(Leg|Arm).*Tip).*");
				break;
		}
		return result;
	}



	BSISoundDescriptor* get_footstep_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().FootstepSoundL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().FootstepSoundR;
				break;
		}
		return nullptr;
	}

	BSSoundHandle get_footstep_sound(NiAVObject* foot, const Foot& foot_kind, const float& scale) {
		BSSoundHandle result = BSSoundHandle::BSSoundHandle();
		auto sound_descriptor = get_footstep_sounddesc(foot_kind);
		auto audio_manager = BSAudioManager::GetSingleton();
		if (sound_descriptor && foot && audio_manager) {

			// https://www.desmos.com/calculator/ygoxbe7hjg
			float k = 1.08;
			float n = 0.39;
			float a = 1.2;

			float volume = pow(k*(scale-a), n);
			float frequency = soft_core(scale, 0.01, 1.0, 1.0, a)*0.5+0.5;
			if (volume > 1e-5) {
				audio_manager->BuildSoundDataFromDescriptor(result, sound_descriptor);
				result.SetVolume(volume);
				result.SetFrequency(frequency);
				NiPoint3 pos;
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				result.SetPosition(pos);
				result.SetObjectToFollow(foot);
			}
		}
		return result;
	}

	BSISoundDescriptor* get_rumble_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().RumbleSoundL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().RumbleSoundR;
				break;
		}
		return nullptr;
	}

	BSSoundHandle get_rumble_sound(NiAVObject* foot, const Foot& foot_kind, const float& scale) {
		BSSoundHandle result = BSSoundHandle::BSSoundHandle();
		auto sound_descriptor = get_rumble_sounddesc(foot_kind);
		auto audio_manager = BSAudioManager::GetSingleton();
		if (sound_descriptor && foot && audio_manager) {
			float k = 1.08;
			float n = 0.39;
			float a = 12.99;

			float volume = pow(k*(scale-a), n);
			float frequency = soft_core(scale, 0.01, 1.0, 1.0, a)*0.5+0.5;
			if (volume > 1e-5) {
				audio_manager->BuildSoundDataFromDescriptor(result, sound_descriptor);
				float volume = pow(k*(scale-a), n);
				result.SetVolume(volume);
				result.SetFrequency(frequency);
				NiPoint3 pos;
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				result.SetPosition(pos);
				result.SetObjectToFollow(foot);
			}
		}
		return result;
	}

	BSISoundDescriptor* get_sprint_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().SprintSoundL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().SprintSoundR;
				break;
		}
		return nullptr;
	}

	BSSoundHandle get_sprint_sound(NiAVObject* foot, const Foot& foot_kind, const float& scale) {
		BSSoundHandle result = BSSoundHandle::BSSoundHandle();
		auto sound_descriptor = get_sprint_sounddesc(foot_kind);
		auto audio_manager = BSAudioManager::GetSingleton();
		if (sound_descriptor && foot && audio_manager) {
			float k = 1.08;
			float n = 0.39;
			float a = 12.99;

			float volume = pow(k*(scale-a), n);
			float frequency = soft_core(scale, 0.01, 1.0, 1.0, a)*0.5+0.5;
			if (volume > 1e-5) {
				audio_manager->BuildSoundDataFromDescriptor(result, sound_descriptor);
				float volume = pow(k*(scale-a), n);
				result.SetVolume(volume);
				result.SetFrequency(frequency);
				NiPoint3 pos;
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				result.SetPosition(pos);
				result.SetObjectToFollow(foot);
			}
		}
		return result;
	}

	BSISoundDescriptor* get_xlfeet_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().ExtraLargeFeetSoundL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().ExtraLargeFeetSoundR;
				break;
		}
		return nullptr;
	}

	BSSoundHandle get_xlfeet_sound(NiAVObject* foot, const Foot& foot_kind, const float& scale) {
		BSSoundHandle result = BSSoundHandle::BSSoundHandle();
		auto sound_descriptor = get_xlfeet_sounddesc(foot_kind);
		auto audio_manager = BSAudioManager::GetSingleton();
		if (sound_descriptor && foot && audio_manager) {
			float k = 1.08;
			float n = 0.39;
			float a = 12.99;

			float volume = pow(k*(scale-a), n);
			float frequency = soft_core(scale, 0.01, 1.0, 1.0, a)*0.5+0.5;
			if (volume > 1e-5) {
				audio_manager->BuildSoundDataFromDescriptor(result, sound_descriptor);
				float volume = pow(k*(scale-a), n);
				result.SetVolume(volume);
				result.SetFrequency(frequency);
				NiPoint3 pos;
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				result.SetPosition(pos);
				result.SetObjectToFollow(foot);
			}
		}
		return result;
	}

	BSISoundDescriptor* get_jumpland_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::JumpLand:
				return Runtime::GetSingleton().JumpLandSound;
				break;
		}
		return nullptr;
	}

	BSSoundHandle get_jumpland_sound(NiAVObject* foot, const Foot& foot_kind, const float& scale) {
		BSSoundHandle result = BSSoundHandle::BSSoundHandle();
		auto sound_descriptor = get_jumpland_sounddesc(foot_kind);
		auto audio_manager = BSAudioManager::GetSingleton();
		if (sound_descriptor && foot && audio_manager) {
			float k = 1.08;
			float n = 0.39;
			float a = 1.2;

			float volume = pow(k*(scale-a), n);
			float frequency = soft_core(scale, 0.01, 1.0, 1.0, a)*0.5+0.5;
			if (volume > 1e-5) {
				audio_manager->BuildSoundDataFromDescriptor(result, sound_descriptor);
				float volume = pow(k*(scale-a), n);
				result.SetVolume(volume);
				result.SetFrequency(frequency);
				NiPoint3 pos;
				pos.x = 0;
				pos.y = 0;
				pos.z = 0;
				result.SetPosition(pos);
				result.SetObjectToFollow(foot);
			}
		}
		return result;
	}

	void do_shakes(Actor* actor, const Foot& foot_kind, const float& scale) {
		float power_multi = 1.0;
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Right:
			case Foot::Front:
			case Foot::Back:
				break;
			case Foot::JumpLand:
				power_multi = 2.0;
				break;
			default:
				return;
				break;
		}
		float distance_to_camera = unit_to_meter(get_distance_to_camera(actor));

		// Camera shakes
		// 1.0 Meter ~= 20% Power
		// 0.5 Meter ~= 50% Power
		float falloff = soft_core(distance_to_camera, 0.024, 2.0, 0.8, 0.0);
		// Power increases cubically with scale (linearly with volume)
		float n = 3.0;
		float min_shake_scale = 1.2; // Before this no shaking
		float max_shake_scale = 20.0; // After this we have full power shaking
		float a = min_shake_scale;
		float k = 1.0/pow(scale - a, n);
		float power = k*pow(scale - a, n) * power_multi;

		float intensity = power * falloff;
		float duration_power = 0.25 * power;
		float duration = duration_power * falloff;
		if (intensity > 0.05 && duration > 0.05) {
			shake_camera(actor, intensity, duration);
			float left_shake = intensity;
			float right_shake = intensity;
			if (actor->formID == 0x14) {
				switch (foot_kind) {
					case Foot::Left:
					case Foot::Front:
						right_shake = 0.0;
						break;
					case Foot::Right:
					case Foot::Back:
						left_shake = 0.0;
						break;
				}
			}
			shake_controller(left_shake, right_shake, duration);
		}
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
			// Foot step time
			auto actor_data = Persistent::GetSingleton().GetActorData(actor);
			if (!actor_data) return;
			float scale = actor_data->visual_scale;

			float minimal_size = 1.2;
			if (scale > minimal_size && !actor->IsSwimming()) {
				if (actor->IsSprinting()) {
					scale *= 1.75; // Sprinting makes you sound bigger
				} else if (actor->IsSneaking()) {
					scale *= 0.55; // Sneaking makes you sound quieter
				} else if (actor->IsWalking()) {
					scale *= 0.85; // Walking makes you sound quieter
				}
				Foot foot_kind = get_foot_kind(tag);
				NiAVObject* foot = get_landing_foot(actor, foot_kind);

				BSSoundHandle footstep_sound = get_footstep_sound(foot, foot_kind, scale);
				BSSoundHandle rumble_sound = get_rumble_sound(foot, foot_kind, scale);
				BSSoundHandle sprint_sound = get_sprint_sound(foot, foot_kind, scale);
				BSSoundHandle xlfeet_sound = get_xlfeet_sound(foot, foot_kind, scale);
				BSSoundHandle jumpland_sound = get_jumpland_sound(foot, foot_kind, scale);

				if (footstep_sound.soundID != BSSoundHandle::kInvalidID) {
					footstep_sound.Play();
				}
				if (rumble_sound.soundID != BSSoundHandle::kInvalidID) {
					rumble_sound.Play();
				}
				if (sprint_sound.soundID != BSSoundHandle::kInvalidID) {
					sprint_sound.Play();
				}
				if (xlfeet_sound.soundID != BSSoundHandle::kInvalidID) {
					xlfeet_sound.Play();
				}
				if (jumpland_sound.soundID != BSSoundHandle::kInvalidID) {
					jumpland_sound.Play();
				}

				shake_camera(actor, 1.0, 1.0);
				shake_controller(1.0, 1.0, 1.0);

				//do_shakes(actor, foot_kind, scale);
			}
		}
	}
}
