#include "managers/GtsSizeManager.hpp"
#include "scale/scalespellmanager.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "timer.hpp"

using namespace SKSE;
using namespace Gts;


namespace Gts {
	ScaleSpellManager& ScaleSpellManager::GetSingleton() noexcept {
		static ScaleSpellManager instance;
		return instance;
	}

	void ScaleSpellManager::CheckSize(Actor* actor) {
		if (!actor || !actor->Is3DLoaded() || actor->IsDead()) {
			return;
		}
		if (actor->formID != 0x14) {
			float actorscale = get_visual_scale(actor);
			ApplySpellBonus(actor, actorscale);
		}
	}

	void ScaleSpellManager::ApplySpellBonus(Actor* actor, float scale) {
		if (!actor) {
			return;
		}
		//log::info("Receiver Name: {}, Scale: {}", actor->GetDisplayFullName(), scale);

		// TODO: Improve this
		if (scale >= 32.0) { //scale <= 40.0) {
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMedium");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellSmall");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellLarge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellTiny");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellHuge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMassive");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellImpossible");
			Runtime::AddSpell(actor, "gtsSizeCloakSpellGigantic");
		} else if (scale >= 24.0 && scale <= 32.0) {
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMedium");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellSmall");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellLarge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellTiny");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellHuge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellGigantic");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellImpossible");
			Runtime::AddSpell(actor, "gtsSizeCloakSpellMassive");
		} else if (scale >= 18.0 && scale <= 24.0) {
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMedium");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellSmall");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellLarge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellTiny");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellHuge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMassive");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellGigantic");
			Runtime::AddSpell(actor, "gtsSizeCloakSpellMega");
		} else if (scale >= 12.0 && scale <= 18.0) {
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMedium");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellSmall");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellLarge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellTiny");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMassive");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellGigantic");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMega");
			Runtime::AddSpell(actor, "gtsSizeCloakSpellHuge");
		} else if (scale >= 8.0 && scale <= 12.0) {
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMedium");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellSmall");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellTiny");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellHuge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMassive");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMega");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellGigantic");
			Runtime::AddSpell(actor, "gtsSizeCloakSpellLarge");
		} else if (scale >= 2.5 && scale <= 8.0) {
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellSmall");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellLarge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellTiny");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellHuge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMassive");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellGigantic");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMega");
			Runtime::AddSpell(actor, "gtsSizeCloakSpellMedium");

			if (scale >=3.25 && scale <= 8.0) {
				Runtime::AddSpell(actor, "gtsSizeCloakSpellMedium");
			}
			if (scale >=5.0 && scale <= 8.0) {
				Runtime::AddSpell(actor, "gtsSizeCloakSpellMedium");
			}

		} else if (scale >= 1.1 && scale <= 2.5) {
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMedium");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellLarge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellTiny");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellHuge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMassive");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellGigantic");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMega");
			Runtime::AddSpell(actor, "gtsSizeCloakSpellSmall");
		} else if (scale <= 1.1) {
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMedium");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellLarge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellHuge");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMassive");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellGigantic");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellMega");
			Runtime::RemoveSpell(actor, "gtsSizeCloakSpellSmall");
			Runtime::AddSpell(actor, "gtsSizeCloakSpellTiny");
		}
	}
}
