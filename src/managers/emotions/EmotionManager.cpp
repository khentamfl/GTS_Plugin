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
#include "spring.hpp"
#include "node.hpp"

namespace Gts { // To-do: rework it into Task manager. Not sure how to do it.
	EmotionData::EmotionData(Actor* giant) : giant(giant) {
	}

	EmotionManager& EmotionManager::GetSingleton() noexcept {
		static EmotionManager instance;
		return instance;
	}

	std::string EmotionManager::DebugName() {
		return "EmotionManager";
	}

	void EmotionData::UpdateEmotions(Actor* giant) {
		auto fgen = giant->GetFaceGenAnimationData();
		std::uint32_t Zero = 0;
		std::uint32_t One = 1;
		std::uint32_t Two = 2;
		std::uint32_t Five = 5;
		if (fgen) {
			if (this->Phenome0.value != this->Phenome0.target) {
				fgen->phenomeKeyFrame.SetValue(Zero, this->Phenome0.value);
			}
			if (this->Phenome1.value != this->Phenome1.target) {
				fgen->phenomeKeyFrame.SetValue(One, this->Phenome1.value);
			}
			//fgen->phenomeKeyFrame.SetValue(Two, this->Phenomes[2].value);
			if (this->Phenome5.value != this->Phenome5.target) {
				fgen->phenomeKeyFrame.SetValue(Five, this->Phenome5.value);
			}

			if (this->Modifier0.value != this->Modifier0.target) {
				fgen->modifierKeyFrame.SetValue(Zero, this->Modifier0.value);
			}
			if (this->Modifier1.value != this->Modifier1.target) {
				fgen->modifierKeyFrame.SetValue(One, this->Modifier1.value);
			}
		}
	}
	void EmotionData::OverridePhenome(int number, float power, float hl, float tg) {
		// this->Phenomes[number].value = power;
		if (number == 0) {
			this->Phenome0.target = tg;
			this->Phenome0.halflife = hl;
		} else if (number == 1) {
			this->Phenome1.target = tg;
			this->Phenome1.halflife = hl;
		} else if (number == 5) {
			this->Phenome5.target = tg;
			this->Phenome5.halflife = hl;
		}
		//this->Phenomes[number].halflife = hl;
		//this->Phenomes[number].target = tg;
	}

	void EmotionData::OverrideModifier(int number, float power, float hl, float tg) {
		if (number == 0) {
			this->Modifier0.target = tg;
			this->Modifier0.halflife = hl;
		} else if (number == 1) {
			this->Modifier1.target = tg;
			this->Modifier1.halflife = hl;
		}
		//this->Modifiers[number].value = power;
		//this->Modifiers[number].halflife = hl;
		//this->Modifiers[number].target = tg;
	}

	void EmotionData::Update() {
		auto giant = this->giant;
		if (!giant) {
			return;
		}
		EmotionData::UpdateEmotions(giant);
	}

	void EmotionManager::Update() {
		for (auto& [key, Emotions]: this->data) {
			Emotions.Update();
		}
	}

	EmotionData& EmotionManager::GetGiant(Actor* giant) {
		// Create it now if not there yet
		this->data.try_emplace(giant, giant);
		return this->data.at(giant);
	}
}