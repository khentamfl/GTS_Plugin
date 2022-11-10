#pragma once
// Misc codes
#include <math.h>
#include <regex>

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {

	template< typename ... Args >
	void Notify(std::string_view rt_fmt_str, Args&&... args) {
		try {
			DebugNotification(std::vformat(rt_fmt_str, std::make_format_args(args ...)).c_str());
		} catch (const std::format_error &e) {
			log::info("Could not format notification, check valid format string: {}", e.what());
		}
	}

	inline std::string_view actor_name(Actor* actor) {
		if (actor) {
			auto ba = actor->GetActorBase();
			if (ba) {
				return ba->GetFullName();
			}
		}
		return "";
	}

	inline std::string Vector2Str(const hkVector4& vector) {
		return std::format("{:.2f},{:.2f},{:.2f},{:.2f}", vector.quad.m128_f32[0], vector.quad.m128_f32[1], vector.quad.m128_f32[2], vector.quad.m128_f32[3]);
	}

	inline std::string Vector2Str(const hkVector4* vector) {
		if (vector) {
			return std::format("{:.2f},{:.2f},{:.2f},{:.2f}", vector->quad.m128_f32[0], vector->quad.m128_f32[1], vector->quad.m128_f32[2], vector->quad.m128_f32[3]);
		} else {
			return "";
		}
	}

	inline std::string Vector2Str(const NiPoint3& vector) {
		return std::format("{:.2f},{:.2f},{:.2f}", vector.x, vector.y, vector.z);
	}

	inline std::string Vector2Str(const NiPoint3* vector) {
		if (vector) {
			return std::format("{:.2f},{:.2f},{:.2f}", vector->x, vector->y, vector->z);
		} else {
			return "";
		}
	}

	inline bool starts_with(std::string_view arg, std::string_view prefix) {
		return arg.compare(0, prefix.size(), prefix);
	}

	inline bool matches(std::string_view str, std::string_view reg) {
		std::regex the_regex(std::string(reg).c_str());
		return std::regex_match(std::string(str), the_regex);
	}

	vector<Actor*> find_actors();
	vector<Actor*> find_actors_high();
	vector<Actor*> find_actors_middle_high();
	vector<Actor*> find_actors_middle_low();
	vector<Actor*> find_actors_low();

	float unit_to_meter(const float& unit);
	float meter_to_unit(const float& meter);
	NiPoint3 unit_to_meter(const NiPoint3& unit);
	NiPoint3 meter_to_unit(const NiPoint3& meter);
	void critically_damped(
		float& x,
		float& v,
		float x_goal,
		float halflife,
		float dt);

	inline bool logit(Actor* actor) {
		return (actor->formID == 0x14 || actor->IsPlayerTeammate());
	}

	struct SoftPotential {
		float k;
		float n;
		float s;
		float o;
		float a;
	};
	// https://www.desmos.com/calculator/pqgliwxzi2
	inline float soft_power(const float x, const float k,const float n,const float s,const float o,const float a) {
		return pow(1.0+pow(k*(x),n*s),1.0/s)/pow(1.0+pow(k*o,n*s), 1.0/s) + a;
	}
	// https://www.desmos.com/calculator/vyofjrqmrn
	inline float soft_core(const float x, const float k, const float n, const float s, const float o, const float a) {
		return 1.0/soft_power(x, k, n, s, o, 0.0) + a;
	}
	inline float soft_core(const float x, const SoftPotential& soft_potential) {
		return soft_core(x, soft_potential.k, soft_potential.n, soft_potential.s, soft_potential.o, soft_potential.a);
	}
	inline float soft_power(const float x, const SoftPotential& soft_potential) {
		return soft_power(x, soft_potential.k, soft_potential.n, soft_potential.s, soft_potential.o, soft_potential.a);
	}

	// https://en.wikipedia.org/wiki/Smoothstep
	inline float clamp(float lowerlimit, float upperlimit, float x) {
		if (x < lowerlimit) {
			x = lowerlimit;
		}
		if (x > upperlimit) {
			x = upperlimit;
		}
		return x;
	}
	inline float smootherstep(float edge0, float edge1, float x) {
		// Scale, and clamp x to 0..1 range
		x = clamp(0.0, 1.0, (x - edge0) / (edge1 - edge0));
		// Evaluate polynomial
		return x * x * x * (x * (x * 6 - 15) + 10);
	}
	inline float smoothstep (float edge0, float edge1, float x) {
		// Scale/bias into [0..1] range
		x = clamp(0.0, 1.0, (x - edge0) / (edge1 - edge0));

		return x * x * (3 - 2 * x);
	}

	inline void shake_camera(TESObjectREFR* actor, float intensity, float duration) {
		const auto skyrimVM = RE::SkyrimVM::GetSingleton();
		auto vm = skyrimVM ? skyrimVM->impl : nullptr;
		if (vm) {
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			auto args = RE::MakeFunctionArguments(std::move(actor), std::move(intensity), std::move(duration));
			vm->DispatchStaticCall("Game", "ShakeCamera", args, callback);
		}
	}

	inline void shake_controller(float left_intensity, float right_intensity, float duration) {
		const auto skyrimVM = RE::SkyrimVM::GetSingleton();
		auto vm = skyrimVM ? skyrimVM->impl : nullptr;
		if (vm) {
			RE::BSTSmartPointer<RE::BSScript::IStackCallbackFunctor> callback;
			auto args = RE::MakeFunctionArguments(std::move(left_intensity), std::move(right_intensity), std::move(duration));
			vm->DispatchStaticCall("Game", "ShakeController", args, callback);
		}
	}

	inline float get_distance_to_camera(const NiPoint3& point) {
		auto camera = PlayerCamera::GetSingleton();
		if (camera) {
			auto point_a = point;
			auto point_b = camera->pos;
			auto delta = point_a - point_b;
			return delta.Length();
		}
		return 3.4028237E38; // Max float
	}

	inline float get_distance_to_camera(NiAVObject* node) {
		if (node) {
			return get_distance_to_camera(node->world.translate);
		}
		return 3.4028237E38; // Max float
	}

	inline float get_distance_to_camera(Actor* actor) {
		if (actor) {
			return get_distance_to_camera(actor->GetPosition());
		}
		return 3.4028237E38; // Max float
	}


	inline float get_distance_to_actor(Actor* receiver, Actor* target) {
		if (target) {
			auto point_a = receiver->GetPosition();
			auto point_b = target->GetPosition();
			auto delta = point_a - point_b;
			return delta.Length();
		}
		return 3.4028237E38; // Max float
	}

	inline bool IsJumping(Actor* actor) {
		//if (!actor) {
			return false;
		//}
		//bool result = false;
		//actor->GetGraphVariableBool("bInJumpState", result);
		//return result;
	}

	inline float GetMaxAV(Actor* actor, ActorValue av) {
		auto baseValue = actor->GetPermanentActorValue(av);
		auto tempMod = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIERS::kTemporary, av);
		return baseValue + tempMod;
	}
	inline float GetAV(Actor* actor, ActorValue av) {
		// actor->GetActorValue(av); returns a cached value so we calc directly from mods
		float max_av = GetMaxAV(actor, av);
		auto damageMod = actor->GetActorValueModifier(ACTOR_VALUE_MODIFIERS::kDamage, av);
		return max_av + damageMod;
	}
	inline void ModAV(Actor* actor, ActorValue av, float amount) {
		actor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kTemporary, av, amount);
	}
	inline void SetAV(Actor* actor, ActorValue av, float amount) {
		float currentValue = GetAV(actor, av);
		float delta = amount - currentValue;
		ModAV(actor, av, delta);
	}

	inline void DamageAV(Actor* actor, ActorValue av, float amount) {
		actor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, av, -amount);
	}

	inline float GetPercentageAV(Actor* actor, ActorValue av) {
		return GetAV(actor, av)/GetMaxAV(actor, av);
	}

	inline void SetPercentageAV(Actor* actor, ActorValue av, float target) {
		auto currentValue = GetAV(actor, av);
		auto maxValue = GetMaxAV(actor, av);
		auto percentage = currentValue/maxValue;
		auto targetValue = target * maxValue;
		float delta = targetValue - currentValue;
		actor->RestoreActorValue(ACTOR_VALUE_MODIFIER::kDamage, av, delta);
	}

	inline float GetStaminaPercentage(Actor* actor) {
		return GetPercentageAV(actor, ActorValue::kStamina);
	}

	inline void SetStaminaPercentage(Actor* actor, float target) {
		SetPercentageAV(actor, ActorValue::kStamina, target);
	}

	inline float GetHealthPercentage(Actor* actor) {
		return GetPercentageAV(actor, ActorValue::kHealth);
	}

	inline void SetHealthPercentage(Actor* actor, float target) {
		SetPercentageAV(actor, ActorValue::kHealth, target);
	}

	inline float GetMagikaPercentage(Actor* actor) {
		return GetPercentageAV(actor, ActorValue::kMagicka);
	}

	inline void SetMagickaPercentage(Actor* actor, float target) {
		SetPercentageAV(actor, ActorValue::kMagicka, target);
	}

	inline void PlaySound(BSISoundDescriptor* soundDescriptor, Actor* Receiver, float Volume, float Frequency) {
		if (!soundDescriptor) {
			log::error("Sound invalid");
			return;
		}
		auto audioManager = BSAudioManager::GetSingleton();
		if (!audioManager) {
			log::error("Audio Manager invalid");
			return;
		}
		BSSoundHandle soundHandle;
		bool success = audioManager->BuildSoundDataFromDescriptor(soundHandle, soundDescriptor);
		if (success) {
			//soundHandle.SetFrequency(Frequency);
			soundHandle.SetVolume(Volume);
			NiAVObject* follow = nullptr;
			if (Receiver) {
				NiAVObject* current_3d = Receiver->GetCurrent3D();
				if (current_3d) {
					follow = current_3d;
				}
			}
			soundHandle.SetObjectToFollow(follow);
			soundHandle.Play();
		} else {
			log::error("Could not build sound");
		}
	}

	inline void PlaySound_Frequency(BSISoundDescriptor* soundDescriptor, Actor* Receiver, float Volume, float Frequency) {
		if (!soundDescriptor) {
			log::error("Sound invalid");
			return;
		}
		auto audioManager = BSAudioManager::GetSingleton();
		if (!audioManager) {
			log::error("Audio Manager invalid");
			return;
		}
		BSSoundHandle soundHandle;
		bool success = audioManager->BuildSoundDataFromDescriptor(soundHandle, soundDescriptor);
		if (success) {
			soundHandle.SetFrequency(Frequency);
			soundHandle.SetVolume(Volume);
			NiAVObject* follow = nullptr;
			if (Receiver) {
				NiAVObject* current_3d = Receiver->GetCurrent3D();
				if (current_3d) {
					follow = current_3d;
				}
			}
			soundHandle.SetObjectToFollow(follow);
			soundHandle.Play();
		} else {
			log::error("Could not build sound");
		}
	}
}
