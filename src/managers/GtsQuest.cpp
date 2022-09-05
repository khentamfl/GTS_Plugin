#include "managers/GtsQuest.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "magic/effects/common.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	QuestManager& QuestManager::GetSingleton() noexcept {
		static QuestManager instance;
		return instance;
	}


    void QuestManager::Update() {

    auto& runtime = Runtime::GetSingleton();
    auto Player = PlayerCharacter::GetSingleton();
    float QuestStage = runtime.MainQuest->GetCurrentStageID();
   
    if (QuestStage > 100.0 || QuestStage < 20.0 || Player->HasMagicEffect(runtime.EffectGrowthPotion))
    {return;} //Do not calculate if true

    float SizeLimit = runtime.sizeLimit->value;
    float ShrinkPower = 0.00046;
    if (Player->HasMagicEffect(runtime.ResistShrinkPotion)) {
        ShrinkPower *= 0.25;
    }

    float PCScale = get_visual_scale(Player);

    log::info("Quest Stage: {}, IsInCombat: {}", QuestStage, Player->IsInCombat());

    if (QuestStage >= 40 && QuestStage < 60 && Player->IsInCombat() == false && PCScale > 1.00) {
        mod_target_scale(Player, -ShrinkPower * TimeScale());
        log::info("Quest Stage is > 40, < 60");
        } // Shrink Stage 1 (Normal)

    if (QuestStage >= 60 && QuestStage < 70 && Player->IsInCombat() == false && PCScale > 1.00) {
        mod_target_scale(Player, -ShrinkPower/1.5 * TimeScale());
        log::info("Quest Stage is > 60, < 70");
        } // Shrink Stage 2 (Less)
    }
}