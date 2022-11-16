#include "managers/CrushManager.hpp"
#include "data/runtime.hpp"
#include "actorUtils.hpp"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {
	void ScareChance(Actor* actor) {
		int voreFearRoll = rand() % 5;
		if (Runtime::HasMagicEffect(actor, "gtsSmallMassiveThreatMe")) {
			voreFearRoll = rand() % 2;
			shake_camera(PlayerRef, 0.4, 0.25);
		}

		if (voreFearRoll <= 0) {
			Runtime::CastSpell(actor, actor, "GtsVoreFearSpell");
			KnockAreaEffect(actor, 6, 650);
		}
	}

	void FearChance(actor* giant)  {
		float size = get_visual_scale(giant);
		int MaxValue = (20 - (1.6 * size));

		// TODO: Ask about the max value thing here
		// If you have small massive threat then the max value is ALWAYS 4
		if (MaxValue <= 4 || giant->HasMagicEffect(GtsSmallMassiveThreatMe)) {
			MaxValue = 4
		}
		int JumpFearChance = rand() % MaxValue;
		if (JumpFearChance <= 0 ) {
			auto event = RegistrationSet("CastFear");
			event.SendEvent();
		}
	}

	void PleasureText(Actor* actor) {
		Pleasure = rand() % 5;
		if (Pleasure <= 0) {
			if (actor.formID == 0x14) {
				Notify("Crushing your foes feels good and makes you bigger");
			} else {
				Notify("Your companion grows bigger by crushing your foes");
			}
		}
	}

	void GrowAfterTheKill(Actor* actor) {
		if (!Runtime::GetBool(GtsDecideGrowth) || Runtime::HasMagicEffect(actor, "GtsSmallMassiveThreat")) {
			return;
		} else if (Runtime::HasPerk(actor, "GtsCrushGrowth") && Runtime::GetInt("GtsDecideGrowth") >= 1 ) {
			GtsSmallCrushGrowthSpell.Cast(actor)
		}
		PleasureText(actor)
	}

	void RandomMoan(Actor* actor) {
		auto randomInt = rand() % 10;
		if (randomInt < 1 ) {
			Runtime::PlaySound("GtsMoanSound", actor, 1.0, 1.0);
			GrowAfterTheKill(actor)
		}
	}
}

namespace Gts {
	CrushManager& CrushManager::GetSingleton() noexcept {
		static CrushManager instance;
		return instance;
	}

	std::string CrushManager::DebugName() {
		return "CrushManager";
	}

	void CrushManager::Update() {
		for (auto &[small, data]: this->data) {
			auto giant = data.giant;
			if (!small) {
				continue;
			}
			if (!giant) {
				continue;
			}
			if (!small->Is3DLoaded()) {
				continue;
			}
			if (!giant->Is3DLoaded()) {
				continue;
			}
			if (data.state == CrushState::Healthy) {
				RandomMoan(data.giant);
				data.state = CrushState::Crushing;
			} else if (data.state == CrushState::Crushing) {
				if (data.time.ShouldRun()) {
					data.state = CrushState::Crushed;

					// Do crush
					Runtime::PlaySound("GtsCrunchSound", actor, 1.0, 1.0);
					Runtime::PlaySound("GtsFallSound", actor, 1.0, 1.0);
					Runtime::CastSpell(small, small, "GtsBleedSpell");
					GrowAfterTheKill(giant);

					shake_camera(giant, 1.8, 1);
					if (giant->formID == 0x14) {
						TriggerScreenBlood(1);
					}
					Runtime::PlayImpactEffect(small, "GtsBloodSprayImpactSet", "NPC Head", NiPoint3{26, 0, 0}, 460, true, true);
					Runtime::PlayImpactEffect(giant, "GtsBloodSprayImpactSet", "NPC L Foot [Lft]", NiPoint3{0, 0, 0}, 0, true, false);
					Runtime::PlayImpactEffect(giant, "GtsBloodSprayImpactSet", "NPC R Foot [Rft]", NiPoint3{0, 0, 0}, 0, true, false);
					if (giant->formID == 0x14 && Runtime::GetBool("GtsEnableLoot")) {
						Actor* into = giant;
						TransferInventory(small, into, false, true);
					} else if (giant->formID != 0x14 && Runtime::GetBool("GtsNPCEnableLoot")) {
						Actor* into = giant;
						TransferInventory(small, into, false, true);
					}
					ScareChance(giant);

					if (small->formID == 0x14) {
						Disintegrate(small); // CTD if we Disintegrate the player
					}

					FearChance(giant);
				}
			}
		}
	}

	void CrushManager::Reset() {
		this->data.clear();
	}

	void CrushManager::ResetActor(Actor* actor) {
		if (actor) {
			UnDisintegrate(actor);
			SetUnRestrained(actor);
			SetMove(actor);
		}
		this->data.erase(actor);
	}

	void Crush(Actor* giant, Actor* small) {
		if (CrushManager::CanCrush(giant, small)) {
			this->data.try_emplace(small, giant, small);
		}
	}

	bool CrushManager::AlreadyCrushed(Acor* actor) {
		return !(CrushManager::GetSingleton().data.find(actor) == m.end());
	}

	bool CrushManager::CanCrush(Actor* giant, Actor* small) {
		if (CrushManager::AlreadyCrushed(small)) {
			return false;
		}

		// Check if they are immune
		const std::string_view CANT_CRUSH_EDITORID = "GtsCantStomp";
		if (small->HasKeywordString(CANT_CRUSH_EDITORID)) {
			// TODO: Check GtsCantStomp is a valid keyword
			return false;
		}

		// Check skin
		auto skin = small->GetSkin();
		if (skin) {
			if (skin->HasKeywordString(CANT_CRUSH_EDITORID)) {
				return false;
			}
		}
		const auto inv = small->GetInventory([](TESBoundObject& a_object) {
			return a_object.IsArmor();
		});

		// Check worn armor
		for (const auto& [item, invData] : inv) {
			const auto& [count, entry] = invData;
			if (count > 0 && entry->IsWorn()) {
				const auto armor = item->As<TESObjectARMO>();
				if (armor && armor->HasKeywordString(CANT_CRUSH_EDITORID)) {
					return false;
				}
			}
		}

		return true;
	}

	CrushData::CrushData(Actor* giant, Actor* small); {
		this->state = CrushState::Healthy;
		this->delay = Timer(0.01);
		this->giant = giant;
	}
}
