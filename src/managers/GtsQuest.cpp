#include "managers/GtsQuest.hpp"

using namespace SKSE;
using namespace RE;

namespace Gts {
	QuestManager& QuestManager::GetSingleton() noexcept {
		static QuestManager instance;
		return instance;
	}
    void QuestManager::Update() {

    auto Player = PlayerCharacter::GetSingleton();
    auto GtsQuest = QuestManager::GetSingleton();
    float SizeLimit = runtime.SizeLimit->value;
    float SelectedFormula = runtime.SelectedSizeFormula->value;

    float PlayerSize = get_visual_scale(Player);
    log::info("Quest Stage is: {}", GtsQuest->GetCurrentStageID());
    if (SelectedFormula >= 2.0)
    {SizeLimit = runtime.AdjustMaxSize_MassBased->value;}

    if (GtsQuest->GetCurrentStageID() >= 40 && GtsQuest->GetCurrentStageID() < 60 && Player->IsInCombat() == false && PCScale > 1.00)
        {mod_target_scale(Player, -0.00040)} // Shrink Stage 1 (Normal)

    if (GtsQuest->GetCurrentStageID() >= 60 && GtsQuest->GetCurrentStageID() < 70 && Player->IsInCombat() == false && PCScale > 1.00)
        {mod_target_scale(Player, -0.00020)} // Shrink Stage 2 (Less)
    }
}