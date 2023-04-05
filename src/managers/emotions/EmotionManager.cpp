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

 namespace Gts {   
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
        if (this->AllowEmotionEdit) {
            auto fgen = giant->GetFaceGenAnimationData();
            log::info("Emotions True");
			if (fgen) { 
                fgen->phenomeKeyFrame.SetValue(0, this->Phenomes[0].value);
                fgen->phenomeKeyFrame.SetValue(1, this->Phenomes[1].value);
                fgen->phenomeKeyFrame.SetValue(2, this->Phenomes[2].value);
                fgen->phenomeKeyFrame.SetValue(5, this->Phenomes[3].value);

                fgen->modifierKeyFrame.SetValue(0, this->Modifiers[0].value);
                fgen->modifierKeyFrame.SetValue(1, this->Modifiers[1].value);
            }
        }
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