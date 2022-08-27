#include "managers/GtsQuest.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "magic/common.hpp"

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
    auto GtsQuest = runtime.MainQuest;
    float QuestStage = GtsQuest->GetCurrentStageID();
   
    if (QuestStage > 100.0 || QuestStage < 20.0)
    {return;} //Do not calculate if true

    float SizeLimit = runtime.sizeLimit->value;
    float SelectedFormula = runtime.SelectedSizeFormula->value;

    float PCScale = get_visual_scale(Player);
    if (SelectedFormula >= 2.0)
    {SizeLimit = runtime.MassBasedSizeLimit->value;}

    if (QuestStage >= 40 && QuestStage < 60 && Player->IsInCombat() == false && PCScale > 1.00)
        {mod_target_scale(Player, -0.00040 * TimeScale());} // Shrink Stage 1 (Normal)

    if (QuestStage >= 60 && QuestStage < 70 && Player->IsInCombat() == false && PCScale > 1.00)
        {mod_target_scale(Player, -0.00020 * TimeScale());} // Shrink Stage 2 (Less)
    }
}