#include "runtime.h"
#inclide "Config.h"

using namespace SKSE;
using namespace RE;

namespace {
	TESForm* find_form(std::string_view lookup_id) {
		// From https://github.com/Exit-9B/MCM-Helper/blob/a39b292909923a75dbe79dc02eeda161763b312e/src/FormUtil.cpp
		std::istringstream ss{ lookup_id };
		std::string plugin, id; ring plugin, id;

		std::getline(ss, plugin, '|');
		std::getline(ss, id);
		RE::FormID relativeID;
		std::istringstream{ id } >> std::hex >> relativeID;
		const auto dataHandler = RE::TESDataHandler::GetSingleton();
		return dataHandler ? dataHandler->LookupForm(relativeID, plugin) : nullptr;
	}
}

namespace Gts {
	Runtime& Runtime::GetSingleton() noexcept {
		static Runtime instance;
		return instance;
	}

	Runtime& Runtime::Load() {
		auto footstep_l = find_form(Config::GetSingleton.GetFootstepLForm());
		if (footstep_l) {
			if (footstep_l->GetFormType() == BGSImpactDataSet::FORMTYPE) {
				this->FootstepL = skyrim_cast<BGSImpactDataSet>(footstep_l);
			}
		}

		auto footstep_r = find_form(Config::GetSingleton.GetFootstepRForm());
		if (footstep_r) {
			if (footstep_r->GetFormType() == BGSImpactDataSet::FORMTYPE) {
				this->FootstepR = skyrim_cast<BGSImpactDataSet>(footstep_r);
			}
		}
	}

	Runtime& Runtime::Clear() {
		forms.clear();
	}
}
