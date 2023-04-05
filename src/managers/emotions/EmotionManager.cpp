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
            std::uint32_t Zero = 0;
            std::uint32_t One = 1;
            std::uint32_t Two = 2;
            std::uint32_t Five = 5;
			if (fgen) { 
                log::info("PHenome 0 value: {}, target: {}, halflife: {}", this->Phenomes[0].value, this->Phenomes[0].target, this->Phenomes[0].halflife);
                log::info("PHenome 1 value: {}, target: {}, halflife: {}", this->Phenomes[1].value, this->Phenomes[1].target, this->Phenomes[1].halflife);
                log::info("PHenome 2 value: {}, target: {}, halflife: {}", this->Phenomes[2].value, this->Phenomes[2].target, this->Phenomes[2].halflife);
                log::info("PHenome 3 value: {}, target: {}, halflife: {}", this->Phenomes[3].value, this->Phenomes[3].target, this->Phenomes[3].halflife);

                fgen->phenomeKeyFrame.SetValue(Zero, this->Phenomes[0].value);
                fgen->phenomeKeyFrame.SetValue(One, this->Phenomes[1].value);
                fgen->phenomeKeyFrame.SetValue(Two, this->Phenomes[2].value);
                fgen->phenomeKeyFrame.SetValue(Five, this->Phenomes[3].value);

                fgen->modifierKeyFrame.SetValue(Zero, this->Modifiers[0].value);
                fgen->modifierKeyFrame.SetValue(One, this->Modifiers[1].value);
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