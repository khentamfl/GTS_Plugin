#include "data/runtime.h"
#include "Config.h"

using namespace SKSE;
using namespace RE;

namespace {
	template <class T>
	T* find_form(std::string_view lookup_id) {
		// From https://github.com/Exit-9B/MCM-Helper/blob/a39b292909923a75dbe79dc02eeda161763b312e/src/FormUtil.cpp
		std::string lookup_id_str(lookup_id);
		std::istringstream ss{ lookup_id_str };
		std::string plugin, id;

		std::getline(ss, plugin, '|');
		std::getline(ss, id);
		RE::FormID relativeID;
		std::istringstream{ id } >> std::hex >> relativeID;
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		return dataHandler ? dataHandler->LookupForm<T>(relativeID, plugin) : nullptr;
	}
}

namespace Gts {
	Runtime& Runtime::GetSingleton() noexcept {
		static Runtime instance;
		return instance;
	}

	void Runtime::Load() {
		this->FootstepSoundL = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetFootStepL());
		this->FootstepSoundR = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetFootStepR());

		this->RumbleSoundL = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetRumbleL());
		this->RumbleSoundR = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetRumbleR());

		this->SprintSoundL = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetSprintL());
		this->SprintSoundR = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetSprintR());

		this->ExtraLargeFeetSoundL = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXLFeetL());
		this->ExtraLargeFeetSoundR = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXLFeetR());

		this->ExtraExtraLargeFeetSoundL = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXXLFeetL());
		this->ExtraExtraLargeFeetSoundR = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetXXLFeetR());

		this->JumpLandSound = find_form<BGSSoundDescriptorForm>(Config::GetSingleton().GetSound().GetJumpLand());

		this->SmallMassiveThreat = find_form<EffectSetting>(Config::GetSingleton().GetSpellEffects().GetSmallMassiveThreat());
	}
}
