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
		JumpLand,
		Unknown,
	};

	Foot get_foot_kind(Actor* actor, std::string_view tag) {
		Foot foot_kind = Foot::Unknown;
		bool is_jumping = actor ? IsJumping(actor) : false;
		if (is_jumping) {
			log::info("Jumping detected");
		}
		bool in_air = actor ? actor->IsInMidair() : false;
		if (in_air) {
			log::info("InAir detected");
		}
		if (matches(tag, ".*Foot.*Left.*") && !is_jumping && !in_air) {
			foot_kind = Foot::Left;
		} else if (matches(tag, ".*Foot.*Right.*") && !is_jumping && !in_air) {
			foot_kind = Foot::Right;
		} else if (matches(tag, ".*Foot.*Front.*") && !is_jumping && !in_air) {
			foot_kind = Foot::Front;
		} else if (matches(tag, ".*Foot.*Back.*") && !is_jumping && !in_air) {
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

	struct VolumeParams {
		float a;
		float k;
		float n;
		float s;
	};

	float volume_function(float scale, const VolumeParams& params) {
		float k = params.k;
		float a = params.a;
		float n = params.n;
		float s = params.s;
		// https://www.desmos.com/calculator/ygoxbe7hjg
		return k*pow(s*(scale-a), n);
	}

	float frequency_function(float scale, const VolumeParams& params) {
		float a = params.a;
		return soft_core(scale, 0.01, 1.0, 1.0, a)*0.5+0.5;
	}

	float falloff_function(NiAVObject* source) {
		if (source) {
			float distance_to_camera = unit_to_meter(get_distance_to_camera(source));
			// Camera distance based volume falloff
			return soft_core(distance_to_camera, 0.024, 2.0, 0.8, 0.0);
		}
		return 1.0;
	}

	BSSoundHandle get_sound(NiAVObject* foot, const float& scale, BSISoundDescriptor* sound_descriptor, const VolumeParams& params, std::string_view tag) {
		BSSoundHandle result = BSSoundHandle::BSSoundHandle();
		auto audio_manager = BSAudioManager::GetSingleton();
		if (sound_descriptor && foot && audio_manager) {

			float volume = volume_function(scale, params);
			float frequency = frequency_function(scale, params);
			float falloff = falloff_function(foot);
			float intensity = volume * falloff;

			if (intensity > 1e-5) {
				log::info("  - Playing {} with volume: {}, falloff: {}, intensity: {}", tag, volume, falloff, intensity);
				audio_manager->BuildSoundDataFromDescriptor(result, sound_descriptor);
				result.SetVolume(intensity);
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

	BSISoundDescriptor* get_lFootstep_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().lFootstepL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().lFootstepR;
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_lJumpLand_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::JumpLand:
				return Runtime::GetSingleton().lJumpLand;
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlFootstep_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().xlFootstepL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().xlFootstepR;
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlRumble_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().xlRumbleL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().xlRumbleR;
				break;
			case Foot::JumpLand:
				return Runtime::GetSingleton().xlRumbleR;
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xlSprint_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().xlSprintL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().xlSprintR;
				break;
			case Foot::JumpLand:
				return Runtime::GetSingleton().xlSprintR;
				break;
		}
		return nullptr;
	}

	BSISoundDescriptor* get_xxlFootstep_sounddesc(const Foot& foot_kind) {
		switch (foot_kind) {
			case Foot::Left:
			case Foot::Front:
				return Runtime::GetSingleton().xxlFootstepL;
				break;
			case Foot::Right:
			case Foot::Back:
				return Runtime::GetSingleton().xxlFootstepR;
				break;
			case Foot::JumpLand:
				return Runtime::GetSingleton().xxlFootstepR;
				break;
		}
		return nullptr;
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
			log::info("- Foot Impact event: {}", tag);
			// Foot step time
			float scale = get_effective_scale(actor);

			float minimal_size = 1.2;
			if (scale > minimal_size && !actor->IsSwimming()) {
				log::info("Base Scale: {}", scale);
				float start_l = 1.2;
				float start_xl = 11.99;
				float start_xlJumpLand= 1.99;
				float start_xxl = 20.0;
				if (actor->IsSprinting()) {
					float sprint_factor = 1.2;
					scale *= sprint_factor; // Sprinting makes you sound bigger
					start_xl = 7.99 * sprint_factor;
					start_xxl = 15.99 * sprint_factor;
					log::info("Sprint Scale: {}", scale);
				} else if (actor->IsSneaking()) {
					scale *= 0.55; // Sneaking makes you sound quieter
					log::info("Sneak Scale: {}", scale);
				} else if (actor->IsWalking()) {
					scale *= 0.85; // Walking makes you sound quieter
					log::info("Walk Scale: {}", scale);
				}
				Foot foot_kind = get_foot_kind(actor, tag);
				if (foot_kind == Foot::JumpLand) {
					float jump_factor = 1.2;
					scale *= jump_factor; // Jumping makes you sound bigger
					start_xl = 6.99 * jump_factor;
					start_xxl = 14.99 * jump_factor;
					log::info("Jump Scale: {}", scale);
				}
				NiAVObject* foot = get_landing_foot(actor, foot_kind);

				BSSoundHandle lFootstep    = get_sound(foot, scale, get_lFootstep_sounddesc(foot_kind),   VolumeParams { .a = start_l,             .k = 0.6,  .n = 0.7, .s = 1.0}, "L Footstep");
				BSSoundHandle lJumpLand    = get_sound(foot, scale, get_lJumpLand_sounddesc(foot_kind),   VolumeParams { .a = start_l,             .k = 0.6,  .n = 0.7, .s = 1.0}, "L Jump");

				BSSoundHandle xlFootstep   = get_sound(foot, scale, get_xlFootstep_sounddesc(foot_kind),  VolumeParams { .a = start_xl,            .k = 0.65, .n = 0.5, .s = 1.0}, "XL: Footstep");
				BSSoundHandle xlRumble     = get_sound(foot, scale, get_xlRumble_sounddesc(foot_kind),    VolumeParams { .a = start_xl,            .k = 0.65, .n = 0.5, .s = 1.0}, "XL Rumble");
				BSSoundHandle xlSprint     = get_sound(foot, scale, get_xlSprint_sounddesc(foot_kind),    VolumeParams { .a = start_xl,            .k = 0.65, .n = 0.5, .s = 1.0}, "XL Sprint");

				BSSoundHandle xxlFootstepL = get_sound(foot, scale, get_xxlFootstep_sounddesc(foot_kind), VolumeParams { .a = start_xxl,           .k = 0.6,  .n = 0.5, .s = 1.0}, "XXL Footstep");


				if (lFootstep.soundID != BSSoundHandle::kInvalidID) {
					lFootstep.Play();
				}
				if (lJumpLand.soundID != BSSoundHandle::kInvalidID) {
					lJumpLand.Play();
				}
				if (xlFootstep.soundID != BSSoundHandle::kInvalidID) {
					xlFootstep.Play();
				}
				if (xlRumble.soundID != BSSoundHandle::kInvalidID) {
					xlRumble.Play();
				}
				if (xlSprint.soundID != BSSoundHandle::kInvalidID) {
					xlSprint.Play();
				}
				if (xxlFootstepL.soundID != BSSoundHandle::kInvalidID) {
					xxlFootstepL.Play();
				}

				shake_camera(actor, 1.0, 1.0);
				shake_controller(1.0, 1.0, 1.0);

				//do_shakes(actor, foot_kind, scale);
			}
		}
	}
}
