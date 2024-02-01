#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/emotions/EmotionManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "spring.hpp"
#include "node.hpp"

namespace {

	BSFaceGenAnimationData* GetFacialData(Actor* giant) {
		auto fgen = giant->GetFaceGenAnimationData();
		if (fgen) {
			return fgen;
		}
		return nullptr;
	}

	void Phenome_ManagePhenomes(BSFaceGenAnimationData* data, std::uint32_t Phenome, float Value) {
		log::info("Applied Phenome. Current value: {}", Value);
		data->phenomeKeyFrame.SetValue(Phenome, Value);
	}

	void Phenome_ManageModifiers(BSFaceGenAnimationData* data, std::uint32_t Modifier, float Value) {
		log::info("Applied Modifier. Current value: {}", Value);
		data->phenomeKeyFrame.SetValue(Modifier, Value);
	}

	void Task_UpdatePhenome(Actor* giant, int phenome, float halflife, float target) {

		std::string name = std::format("Phenome_{}_{}_{}", giant->formID, phenome, target);
		float AnimSpeed = AnimationManager::GetSingleton().GetAnimSpeed(giant);
		Spring defspring = Spring(0.0, 0.08 * halflife);
		
		Spring& PhenomeSpring = defspring;
		ActorHandle giantHandle = giant->CreateRefHandle();

		PhenomeSpring.target = target;

		TaskManager::Run(name, [=](auto& progressData) {
			if (!giantHandle) {
				return false;
			}
			auto giantref = giantHandle.get().get();
			PhenomeSpring.halflife = halflife/AnimSpeed;

			if (!giantref->Is3DLoaded()) {
				return false;
			}
			auto FaceData = GetFacialData(giantref);
			if (FaceData) {
				log::info("Running Phenome Spring. value: {}, target: {}", PhenomeSpring.value, PhenomeSpring.target);
				if (PhenomeSpring.value >= target) {
					return false;
				}

				Phenome_ManagePhenomes(FaceData, phenome, PhenomeSpring.value);
				return true;
			}

			return false;
		});
	}

	void Task_UpdateModifier(Actor* giant, int modifier, float halflife, float target) {

		std::string name = std::format("Modifier_{}_{}_{}", giant->formID, modifier, target);
		float AnimSpeed = AnimationManager::GetSingleton().GetAnimSpeed(giant);

		Spring defspring = Spring(0.0, 0.25 * halflife);

		Spring& ModifierSpring = defspring;
		ActorHandle giantHandle = giant->CreateRefHandle();

		ModifierSpring.target = target;
		ModifierSpring.halflife = halflife;

		TaskManager::Run(name, [=](auto& progressData) {
			if (!giantHandle) {
				return false;
			}

			auto giantref = giantHandle.get().get();
			ModifierSpring.halflife = halflife/AnimSpeed;

			if (!giantref->Is3DLoaded()) {
				return false;
			}

			auto FaceData = GetFacialData(giantref);
			if (FaceData) {
				log::info("Running Modifier Spring. value: {}, target: {}", ModifierSpring.value, ModifierSpring.target);
				if (ModifierSpring.value >= target) {
					return false;
				}

				Phenome_ManageModifiers(FaceData, modifier, ModifierSpring.value);
				return true;
			}

			return false;
		});
	}
}

namespace Gts {

	EmotionManager& EmotionManager::GetSingleton() noexcept {
		static EmotionManager instance;
		return instance;
	}

	std::string EmotionManager::DebugName() {
		return "EmotionManager";
	}

	void EmotionManager::OverridePhenome(Actor* giant, int number, float power, float hl, float tg) {
		Task_UpdatePhenome(giant, number, hl, tg);
	}

	void EmotionManager::OverrideModifier(Actor* giant, int number, float power, float hl, float tg) {
		Task_UpdateModifier(giant, number, hl, tg);
	}

}