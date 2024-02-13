#include "utils/DifficultyUtils.hpp"

#include "node.hpp"

#include <vector>
#include <string>

using namespace RE;
using namespace Gts;

namespace Gts {
    float GetSettingValue(const char* setting) {
		float modifier = 1.0;
		auto GameSetting = GameSettingCollection::GetSingleton();
		if (GameSetting) {
			modifier = GameSetting->GetSetting(setting)->GetFloat();
		}
		log::info("Difficulty Modifier: {}", modifier);
		return modifier;
	}

	float GetDifficultyMultiplier(Actor* attacker, Actor* receiver) { // Credits to Doodlum for this method
		if (attacker && (attacker->IsPlayerRef() || IsTeammate(attacker))) {
			auto currentdiff = static_cast<Difficulty>(PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty);
			return GetSettingValue(Difficulty_ByPC[currentdiff]);
		} else if (receiver && (receiver->IsPlayerRef() || IsTeammate(attacker))) {
			auto currentdiff = static_cast<Difficulty>(PlayerCharacter::GetSingleton()->GetGameStatsData().difficulty);
			return GetSettingValue(Difficulty_ByNPC[currentdiff]);
		}
		return 1.0;
	}
}