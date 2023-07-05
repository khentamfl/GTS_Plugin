#include "magic/effects/smallmassivethreat.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "node.hpp"

#include <random>

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {
	void ScareChance(Actor* actor) {
		int voreFearRoll = rand() % 5;
		if (Runtime::HasMagicEffect(actor, "SmallMassiveThreat")) {
			voreFearRoll = rand() % 2;
			shake_camera(actor, 0.4, 0.25);
		}

		if (voreFearRoll <= 0) {
			Runtime::CastSpell(actor, actor, "GtsVoreFearSpell");
			KnockAreaEffect(actor, 6, 60 * get_visual_scale(actor));
		}
	}

	void FearChance(Actor* giant)  {
		float size = get_visual_scale(giant);
		int MaxValue = (20 - (1.6 * size));

		// TODO: Ask about the max value thing here
		// If you have small massive threat then the max value is ALWAYS 4

		// S.Answer: It's supposed to proc more often with SMT active, so having it always 4 is fine ^
		if (MaxValue <= 4 || Runtime::HasMagicEffect(giant, "SmallMassiveThreat")) {
			MaxValue = 4;
		}
		int FearChance = rand() % MaxValue;
		if (FearChance <= 0 ) {
			//auto event = RegistrationSet("CastFear");
			//event.SendEvent();
			Runtime::CastSpell(giant, giant, "GtsVoreFearSpell");
			// Should cast fear
		}
	}

	void PleasureText(Actor* actor) {
		int Pleasure = rand() % 5;
		if (Pleasure <= 0) {
			if (actor->formID == 0x14) {
				Notify("Crushing your foes feels good and makes you bigger");
			} else {
				Notify("Your companion grows bigger by crushing your foes");
			}
		}
	}

	void GrowAfterTheKill(Actor* caster, Actor* target) {
		if (!Runtime::GetBool("GtsDecideGrowth") || Runtime::HasMagicEffect(caster, "SmallMassiveThreat")) {
			return;
		} else if (Runtime::HasPerkTeam(caster, "GrowthPerk") && Runtime::GetInt("GtsDecideGrowth") >= 1) {
			float Rate = (0.00016 * get_visual_scale(target)) * 120;
			if (Runtime::HasPerkTeam(caster, "AdditionalGrowth")) {
				Rate *= 2.0;
			}
			CrushGrow(caster, 0, Rate);
		}
		PleasureText(caster);
	}
	void MoanOrLaugh(Actor* giant, Actor* target) {
		static Timer voicetimer = Timer(2.4);
		auto randomInt = rand() % 16;
		auto select = rand() % 3;
		if (randomInt <= 3.0) {
			if (voicetimer.ShouldRun()) {
				if (select >= 2.0) {
					Runtime::PlaySoundAtNode("MoanSound", giant, 1.0, 1.0, "NPC Head [Head]");
					GrowAfterTheKill(giant, target);
				} else {
					Runtime::PlaySoundAtNode("LaughSound_Part2", giant, 1.0, 1.0, "NPC Head [Head]");
				}
			}
		}
	}

	void AdjustGiantessSkill(Actor* Caster, Actor* Target) { // Adjust Matter Of Size skill on Crush
		if (Caster->formID != 0x14) {
			return; //Bye
		}
		auto GtsSkillLevel = Runtime::GetGlobal("GtsSkillLevel");
		auto GtsSkillRatio = Runtime::GetGlobal("GtsSkillRatio");
		auto GtsSkillProgress = Runtime::GetGlobal("GtsSkillProgress");


		int random = (100 + (rand()% 65 + 1)) / 100;

		if (GtsSkillLevel->value >= 100.0) {
			GtsSkillLevel->value = 100.0;
			GtsSkillRatio->value = 0.0;
			return;
		}

		float skill_level = GtsSkillLevel->value;

		float ValueEffectiveness = std::clamp(1.0 - GtsSkillLevel->value/100, 0.10, 1.0);

		float absorbedSize = std::clamp(get_visual_scale(Target), 1.0f, 4.0f);
		float oldvaluecalc = 1.0 - GtsSkillRatio->value; //Attempt to keep progress on the next level
		float Total = (((0.36 * random) + absorbedSize/40) * ValueEffectiveness);
		GtsSkillRatio->value += Total * GetXpBonus();

		if (GtsSkillRatio->value >= 1.0) {
			float transfer = clamp(0.0, 1.0, Total - oldvaluecalc);
			GtsSkillRatio->value = transfer;
			GtsSkillLevel->value = skill_level + 1.0;
			GtsSkillProgress->value = GtsSkillLevel->value;
			PerkPointCheck(GtsSkillLevel->value);
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
		for (auto &[tinyId, data]: this->data) {
			auto tiny = TESForm::LookupByID<Actor>(tinyId);
			auto giantHandle = data.giant;
			if (!tiny) {
				continue;
			}
			if (!giantHandle) {
				continue;
			}
			auto giant = giantHandle.get().get();
			if (!giant) {
				continue;
			}

			auto transient = Transient::GetSingleton().GetData(tiny);
			if (transient) {
				if (!transient->can_be_crushed) {
					return;
				}
			}

			if (data.state == CrushState::Healthy) {
				data.state = CrushState::Crushing;
			} else if (data.state == CrushState::Crushing) {
				if (data.delay.ShouldRun()) {
					data.state = CrushState::Crushed;

					// Do crush
					if (!tiny->IsDead()) {
						KillActor(giant, tiny);
					}
					if (!LessGore()) {
						Runtime::PlaySound("GtsCrushSound", tiny, 1.0, 1.0);
						Runtime::PlaySound("GtsFallSound", tiny, 1.0, 1.0);
					}
					Runtime::PlaySound("BloodGushSound", tiny, 1.0, 0.5);
					float currentSize = get_visual_scale(tiny);
					MoanOrLaugh(giant, tiny);
					GrowAfterTheKill(giant, tiny);
					if (giant->formID == 0x14 && IsDragon(tiny)) {
						CompleteDragonQuest();
					}
					Rumble::Once("CrushRumble", tiny, 1.4, 0.15);
					if (giant->formID == 0x14) {
						if (IsLiving(tiny)) {
							TriggerScreenBlood(50);
						}
					}
					std::random_device rd;
					std::mt19937 gen(rd());
					std::uniform_real_distribution<float> dis(-0.2, 0.2);

					if (giant->formID == 0x14 && Runtime::GetBool("GtsEnableLooting")) {
						Actor* into = giant;
						TransferInventory(tiny, into, false, true);
					} else if (giant->formID != 0x14 && Runtime::GetBool("GtsNPCEnableLooting")) {
						Actor* into = giant;
						TransferInventory(tiny, into, false, true);
					}

					AddSMTDuration(giant, 5.0);
					ReportCrime(giant, tiny, 1000.0, true);
					//StartCombat(giant, tiny, false);

					ScareChance(giant);

					if (!IsLiving(tiny)) {
						SpawnDustParticle(giant, tiny, "NPC L Hand [LHnd]", 3.0);
					} else {
						if (!LessGore()) {
							auto root = find_node(tiny, "NPC Root [Root]");
							if (root) {
								SpawnParticle(tiny, 0.60, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 2.5, 7, root);
								SpawnParticle(tiny, 0.60, "GTS/Damage/Explode.nif", root->world.rotate, root->world.translate, currentSize * 2.5, 7, root);
								SpawnParticle(tiny, 0.60, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 2.5, 7, root);
								SpawnParticle(tiny, 0.60, "GTS/Damage/Crush.nif", root->world.rotate, root->world.translate, currentSize * 2.5, 7, root);
								SpawnParticle(tiny, 1.20, "GTS/Damage/ShrinkOrCrush.nif", NiMatrix3(), root->world.translate, currentSize * 25, 7, root);
							}
							Runtime::CreateExplosion(tiny, get_visual_scale(tiny)/4,"BloodExplosion");
							Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC Head [Head]", NiPoint3{dis(gen), 0, -1}, 512, true, true);
							Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC L Foot [Lft ]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
							Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSet", "NPC R Foot [Rft ]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
						}
					}

					ActorHandle tinyHandle = tiny->CreateRefHandle();
					TaskManager::RunOnce([=](auto& update){
						if (tinyHandle) {
							EventDispatcher::DoResetActor(tinyHandle.get().get());
						}
					});

					if (tiny->formID != 0x14) {
						Disintegrate(tiny); // Player can't be disintegrated: simply nothing happens.
					} else if (tiny->formID == 0x14) {
						if (IsLiving(tiny)) {
							TriggerScreenBlood(50);
						}
						tiny->SetAlpha(0.0); // Fake crush effect, just make player invisible
					}

					AdjustGiantessSkill(giant, tiny); // Adjust Size Matter skill
					FearChance(giant);
				}
			}
		}
	}

	void CrushManager::Reset() {
		this->data.clear();
	}

	void CrushManager::ResetActor(Actor* actor) {
		/*if (actor) {
		        UnDisintegrate(actor);
		        SetUnRestrained(actor);
		        SetMove(actor);
		   }*/
		if (actor) {
			this->data.erase(actor->formID);
		}
	}

	void CrushManager::Crush(Actor* giant, Actor* tiny) {
		if (!giant) {
			return;
		}
		if (!tiny) {
			return;
		}
		if (CrushManager::CanCrush(giant, tiny)) {
			CrushManager::GetSingleton().data.try_emplace(tiny->formID, giant);
		}
	}

	bool CrushManager::AlreadyCrushed(Actor* actor) {
		if (!actor) {
			return false;
		}
		auto& m = CrushManager::GetSingleton().data;
		return (m.find(actor->formID) != m.end());
	}

	bool CrushManager::CanCrush(Actor* giant, Actor* tiny) {
		if (CrushManager::AlreadyCrushed(tiny)) {
			return false;
		}
		if (tiny->IsEssential() && Runtime::GetBool("ProtectEssentials")) {
			return false;
		}
		// Check if they are immune
		const std::string_view CANT_CRUSH_EDITORID = "GtsCantStomp";
		if (tiny->HasKeywordString(CANT_CRUSH_EDITORID)) {
			// TODO: Check GtsCantStomp is a valid keyword
			return false;
		}
		//Check for Essential

		// Check skin
		auto skin = tiny->GetSkin();
		if (skin) {
			if (skin->HasKeywordString(CANT_CRUSH_EDITORID)) {
				return false;
			}
		}
		const auto inv = tiny->GetInventory([](TESBoundObject& a_object) {
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
		//log::info("Can crush {}", tiny->GetDisplayFullName());
		return true;
	}

	CrushData::CrushData(Actor* giant) :
		delay(Timer(0.01)),
		state(CrushState::Healthy),
		giant(giant ? giant->CreateRefHandle() : ActorHandle()) {
	}
}
