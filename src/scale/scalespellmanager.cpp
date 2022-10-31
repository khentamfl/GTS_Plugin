#include "scale/scale.hpp"
#include "scale/scalespellmanager.hpp"
#include "util.hpp"
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
        if (!actor || actor->IsDead() || !actor->Is3DLoaded()) {
			return;
		}
        float actorscale = get_visual_scale(actor);
        ApplySpellBonus(actor, actorscale);
	}

    void ScaleSpellManager::ApplySpellBonus(Actor* actor, float scale) {
		if (!actor) {
			return;
		}
        auto runtime = Runtime::GetSingleton();
        log::info("Receiver Name: {}, Scale: {}", actor->GetDisplayFullName(), scale);

        if (scale >= 40.0) {
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMedium);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellSmall);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellLarge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellTiny);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellHuge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMassive);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellGigantic);
            actor->AddSpell(runtime.gtsSizeCloakSpellImpossible);
        }
        else if (scale >= 32.0 && scale <= 40.0) {       
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMedium);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellSmall);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellLarge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellTiny);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellHuge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMassive);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellImpossible);
            actor->AddSpell(runtime.gtsSizeCloakSpellGigantic);
        }

        else if (scale >= 24.0 && scale <= 32.0) {       
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMedium);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellSmall);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellLarge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellTiny);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellHuge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellGigantic);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellImpossible);
            actor->AddSpell(runtime.gtsSizeCloakSpellMassive);
        }
        else if (scale >= 18.0 && scale <= 24.0) {       
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMedium);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellSmall);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellLarge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellTiny);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellHuge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMassive);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellGigantic);
            actor->AddSpell(runtime.gtsSizeCloakSpellMega);
        }
        else if (scale >= 12.0 && scale <= 18.0) {       
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMedium);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellSmall);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellLarge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellTiny);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMassive);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellGigantic);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMega);
            actor->AddSpell(runtime.gtsSizeCloakSpellHuge);
        }

        else if (scale >= 8.0 && scale <= 12.0) {
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMedium);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellSmall);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellTiny);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellHuge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMassive);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMega);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellGigantic);
            actor->AddSpell(runtime.gtsSizeCloakSpellLarge);   
        }
  else if (scale >= 2.5 && scale <= 8.0) {
            actor->RemoveSpell(runtime.gtsSizeCloakSpellSmall);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellLarge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellTiny);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellHuge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMassive);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellGigantic);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMega);
            actor->AddSpell(runtime.gtsSizeCloakSpellMedium);
    
    if (scale >=3.25 && scale <= 8.0) {
        actor->AddSpell(runtime.gtsSizeCloakSpellMedium); 
    }
    if (scale >=5.0 && scale <= 8.0) {
        actor->AddSpell(runtime.gtsSizeCloakSpellMedium); 
    }

  }
  else if (scale >= 1.1 && scale <= 2.5) {
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMedium);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellLarge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellTiny);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellHuge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMassive);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellGigantic);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMega);
            actor->AddSpell(runtime.gtsSizeCloakSpellSmall);
  }

  else if (scale <= 1.1) {
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMedium);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellLarge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellHuge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMassive);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellGigantic);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMega);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellSmall);
            actor->AddSpell(runtime.gtsSizeCloakSpellTiny);
      }
    }
}

