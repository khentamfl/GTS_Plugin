#pragma once
// Misc codes
#include <math.h>
#include <regex>

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	inline static float* g_delta_time = (float*)REL::RelocationID(523660, 410199).address();

	inline std::string_view actor_name(Actor* actor) {
		if (actor) {
			auto ba = actor->GetActorBase();
			if (ba) {
				return ba->GetFullName();
			}
		}
		return "";
	}

	inline bool starts_with(std::string_view arg, std::string_view prefix) {
		return arg.compare(0, prefix.size(), prefix);
	}

	inline bool matches(std::string_view str, std::string_view reg) {
		std::regex the_regex(std::string(reg).c_str());
		return std::regex_match(std::string(str), the_regex);
	}

	vector<Actor*> find_actors();
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
	// https://www.desmos.com/calculator/lnooldbscf
	inline float soft_power(const float x, const float k,const float n,const float s,const float o,const float a) {
		return pow(1.0+pow(k*(x-o),n*s),1.0/s) + a;
	}
	// https://www.desmos.com/calculator/pcoyowf7hn
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

	[[nodiscard]] std::atomic_bool& get_main_thread();

	inline bool on_mainthread() {
		return get_main_thread().load();
	}

	inline void activate_mainthread_mode() {
		get_main_thread().store(true);
	}
	inline void deactivate_mainthread_mode() {
		get_main_thread().store(false);
	}

	inline bool IsJumping(Actor* actor) {
		if (!actor) {
			return false;
		}
		bool result = false;
		actor->GetGraphVariableBool("bInJumpState", result);
		return result;
	}

	inline float GetStaminaPercentage(Actor* actor) {
		auto baseValue = actor->GetPermanentActorValue(ActorValue::kStamina);
		auto valueMod = actor->staminaModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary];
		auto currentValue = actor->GetActorValue(ActorValue::kStamina);

		return currentValue / (baseValue + valueMod);
	}

	inline float GetHealthPercentage(Actor* actor) {
		auto baseValue = actor->GetPermanentActorValue(ActorValue::kHealth);
		auto valueMod = actor->healthModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary];
		auto currentValue = actor->GetActorValue(ActorValue::kHealth);

		return currentValue / (baseValue + valueMod);
	}

	inline float GetMagikaPercentage(Actor* actor) {
		auto baseValue = actor->GetPermanentActorValue(ActorValue::kMagicka);
		auto valueMod = actor->magickaModifiers.modifiers[ACTOR_VALUE_MODIFIERS::kTemporary];
		auto currentValue = actor->GetActorValue(ActorValue::kMagicka);

		return currentValue / (baseValue + valueMod);
	}

	inline void DamageAV(Actor* actor, ActorValue av, float amount) {
		actor->RestoreActorValue(RE::ACTOR_VALUE_MODIFIER::kDamage, av, -amount);
	}
}
