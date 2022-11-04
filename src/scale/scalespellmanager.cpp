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
        if (!actor || !actor->Is3DLoaded() || actor->IsDead()) {
			return;
		}
        float actorscale = get_target_scale(actor);
        if (actorscale >= 1.25) {//((actor->formID == 0x14 || actor->IsPlayerTeammate() || actor->IsInFaction(Runtime::GetSingleton().FollowerFaction)) && actorscale >= 1.25) {
            ApplySpellBonus(actor, actorscale);
        }
	}

    void ScaleSpellManager::ApplySpellBonus(Actor* actor, float scale) {
		if (!actor) {
			return;
		}
        auto runtime = Runtime::GetSingleton();
        //log::info("Receiver Name: {}, Scale: {}", actor->GetDisplayFullName(), scale);

        //if (scale >= 40.0) {
            //actor->RemoveSpell(runtime.gtsSizeCloakSpellMedium);
            //actor->RemoveSpell(runtime.gtsSizeCloakSpellSmall);
            //actor->RemoveSpell(runtime.gtsSizeCloakSpellLarge);
            //actor->RemoveSpell(runtime.gtsSizeCloakSpellTiny);
            //actor->RemoveSpell(runtime.gtsSizeCloakSpellHuge);
            //actor->RemoveSpell(runtime.gtsSizeCloakSpellMassive);
           // actor->RemoveSpell(runtime.gtsSizeCloakSpellGigantic);
           // actor->AddSpell(runtime.gtsSizeCloakSpellImpossible);
        //}
         if (scale >= 32.0) { //scale <= 40.0) {       
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMedium);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellSmall);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellLarge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellTiny);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellHuge);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellMassive);
            actor->RemoveSpell(runtime.gtsSizeCloakSpellImpossible);
            actor->AddSpell(runtime.gtsSizeCloakSpellGigantic);
            log::info("Adding Spell №11 to {}", actor->GetDisplayFullName());
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
            log::info("Adding Spell №10 to {}", actor->GetDisplayFullName());
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
            log::info("Adding Spell №9 to {}", actor->GetDisplayFullName());
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
            log::info("Adding Spell №8 to {}", actor->GetDisplayFullName());
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
            log::info("Adding Spell №7 to {}", actor->GetDisplayFullName());
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
            log::info("Adding Spell №4 to {}", actor->GetDisplayFullName());
    
    if (scale >=3.25 && scale <= 8.0) {
        actor->AddSpell(runtime.gtsSizeCloakSpellMedium); 
        log::info("Adding Spell №5 to {}", actor->GetDisplayFullName());
    }
    if (scale >=5.0 && scale <= 8.0) {
        actor->AddSpell(runtime.gtsSizeCloakSpellMedium); 
        log::info("Adding Spell №6 to {}", actor->GetDisplayFullName());
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
            log::info("Adding Spell №2 to {}", actor->GetDisplayFullName());
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
            log::info("Adding Spell №1 to {}", actor->GetDisplayFullName());
      }
    }
}

