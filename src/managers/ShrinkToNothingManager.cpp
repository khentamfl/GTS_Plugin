#include "managers/ShrinkToNothingManager.hpp"
#include "data/runtime.hpp"
#include "data/time.hpp"
#include "scale/scale.hpp"

#include <random>

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace Gts {
	ShrinkToNothingManager& ShrinkToNothingManager::GetSingleton() noexcept {
		static ShrinkToNothingManager instance;
		return instance;
	}

	std::string ShrinkToNothingManager::DebugName() {
		return "ShrinkToNothingManager";
	}

	void ShrinkToNothingManager::Update() {
		for (auto &[tiny, data]: this->data) {
			auto giant = data.giant;
			if (!tiny) {
				continue;
			}
			if (!giant) {
				continue;
			}
			//if (!tiny->Is3DLoaded()) {
			//continue;
			//}
			//if (!giant->Is3DLoaded()) {
			//continue;
			//}
			if (data.state == ShrinkState::Healthy) {
				tiny->KillImmediate();
				data.state = ShrinkState::Shrinking;
			} else if (data.state == ShrinkState::Shrinking) {
				if (data.delay.ShouldRun()) {
					if (!tiny->IsDead()) {
						// They cannot be shrunked to nothing if they aint dead
						this->ResetActor(tiny);
						return;
					}

					// Do shrink
					float currentSize = get_target_scale(tiny);
					if (currentSize > 0.14) {
						set_target_scale(tiny, 0.14);
					} else {
						// Fully shrunk
						if (giant->formID == 0x14 && Runtime::GetBool("GtsEnableLooting")) {
							Actor* into = giant;
							TransferInventory(tiny, into, false, true);
						} else if (giant->formID != 0x14 && Runtime::GetBool("GtsNPCEnableLooting")) {
							Actor* into = giant;
							TransferInventory(tiny, into, false, true);
						}
						ShrinkToNothingManager::AdjustGiantessSkill(giant, tiny); // Adjust Size Matter skill

						Runtime::CreateExplosion(tiny, get_visual_scale(tiny),"BloodExplosion");

						if (giant->formID == 0x14 && IsDragon(tiny)) {
							CompleteDragonQuest();
						}
						std::random_device rd;
						std::mt19937 gen(rd());
						std::uniform_real_distribution<float> dis(-0.2, 0.2);

						Runtime::PlaySound("BloodGushSound", tiny, 1.0, 0.5);
						EventDispatcher::DoResetActor(tiny);

						Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC Head [Head]", NiPoint3{dis(gen), 0, -1}, 512, true, true);
						Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC L Foot [Lft ]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
						Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC R Foot [Rft ]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
						Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC Spine [Spn0]", NiPoint3{dis(gen), 0, -1}, 512, true, false);

						if (tiny->formID != 0x14) {
							Disintegrate(tiny); // Player can't be disintegrated: simply nothing happens.
						} else if (tiny->formID == 0x14) {
							TriggerScreenBlood(50);
							tiny->SetAlpha(0.0); // Just make player Invisible
						}

						data.state = ShrinkState::Shrinked;
					}
				}
			}
		}
	}

	void ShrinkToNothingManager::Reset() {
		this->data.clear();
	}

	void ShrinkToNothingManager::ResetActor(Actor* actor) {
		this->data.erase(actor);
	}

	void ShrinkToNothingManager::Shrink(Actor* giant, Actor* tiny) {
		if (ShrinkToNothingManager::CanShrink(giant, tiny)) {
			ShrinkToNothingManager::GetSingleton().data.try_emplace(tiny, giant, tiny);
		}
	}

	bool ShrinkToNothingManager::AlreadyShrinked(Actor* actor) {
		auto& m = ShrinkToNothingManager::GetSingleton().data;
		return !(m.find(actor) == m.end());
	}

	bool ShrinkToNothingManager::CanShrink(Actor* giant, Actor* tiny) {
		if (ShrinkToNothingManager::AlreadyShrinked(tiny)) {
			return false;
		}
		if (tiny->IsEssential() && Runtime::GetBool("ProtectEssentials")) {
			return false;
		}

		return true;
	}

	void ShrinkToNothingManager::AdjustGiantessSkill(Actor* Caster, Actor* Target) { // Adjust Matter Of Size skill on Shrink To Nothing
		if (Caster->formID !=0x14) {
			return; //Bye
		}
		auto GtsSkillLevel = Runtime::GetGlobal("GtsSkillLevel");
		if (!GtsSkillLevel) {
			return;
		}
		auto GtsSkillRatio = Runtime::GetGlobal("GtsSkillRatio");
		if (!GtsSkillRatio) {
			return;
		}
		auto GtsSkillProgress = Runtime::GetGlobal("GtsSkillProgress");
		if (!GtsSkillProgress) {
			return;
		}

		int random = (100 + (rand()% 85 + 1)) / 100;

		if (GtsSkillLevel->value >= 100) {
			GtsSkillLevel->value = 100.0;
			GtsSkillRatio->value = 0.0;
			return;
		}

		float ValueEffectiveness = std::clamp(1.0 - GtsSkillLevel->value/100, 0.10, 1.0);

		float absorbedSize = (get_visual_scale(Target));
		float oldvaluecalc = 1.0 - GtsSkillRatio->value; //Attempt to keep progress on the next level
		float Total = (((0.42 * random) + absorbedSize/50) * ValueEffectiveness);
		GtsSkillRatio->value += Total;

		if (GtsSkillRatio->value >= 1.0) {
			float transfer = clamp(0.0, 1.0, Total - oldvaluecalc);
			GtsSkillRatio->value = 0.0;
			GtsSkillLevel->value += 1.0;
			PerkPointCheck(GtsSkillLevel->value += 1.0);
			GtsSkillProgress->value = GtsSkillLevel->value;
		}
	}

	ShrinkData::ShrinkData(Actor* giant, Actor* tiny) :
		delay(Timer(0.01)),
		state(ShrinkState::Healthy),
		giant(giant) {
	}
}
