#include "managers/ShrinkToNothingManager.hpp"
#include "managers/ai/aifunctions.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "utils/looting.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "profiler.hpp"
#include "node.hpp"

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
		auto profiler = Profilers::Profile("ShrinkToNothing: Update");
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

			log::info("Starting STN");

			if (data.state == ShrinkState::Healthy) {
				SetReanimatedState(tiny);
				data.state = ShrinkState::Shrinking;
				log::info("Set state to shrinking");
			} else if (data.state == ShrinkState::Shrinking) {
				if (data.delay.ShouldRun()) {
					Attacked(tiny, giant);
					log::info("State is shrinking");
					if (giant->formID == 0x14 && IsDragon(tiny)) {
						CompleteDragonQuest(tiny, false, tiny->IsDead());
					}
					// Do shrink
					float currentSize = get_visual_scale(tiny);

					// Fully shrunk
					ModSizeExperience(0.24, giant); // Adjust Size Matter skill
					KillActor(giant, tiny);
					log::info("Killed Actor");

					if (!IsLiving(tiny)) {
						SpawnDustParticle(tiny, tiny, "NPC Root [Root]", 3.6);
					} else {
						if (!LessGore()) {
							std::random_device rd;
							std::mt19937 gen(rd());
							std::uniform_real_distribution<float> dis(-0.2, 0.2);
							auto root = find_node(tiny, "NPC Root [Root]");
							if (root) {
								SpawnParticle(tiny, 0.20, "GTS/Damage/Explode.nif", NiMatrix3(), root->world.translate, 2.0, 7, root);
								SpawnParticle(tiny, 0.20, "GTS/Damage/Explode.nif", NiMatrix3(), root->world.translate, 2.0, 7, root);
								SpawnParticle(tiny, 0.20, "GTS/Damage/Explode.nif", NiMatrix3(), root->world.translate, 2.0, 7, root);
								SpawnParticle(tiny, 1.20, "GTS/Damage/ShrinkOrCrush.nif", NiMatrix3(), root->world.translate, get_visual_scale(tiny) * 10, 7, root);
							}
							Runtime::CreateExplosion(tiny, get_visual_scale(tiny)/4, "BloodExplosion");
							Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC Head [Head]", NiPoint3{dis(gen), 0, -1}, 512, true, true);
							Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC L Foot [Lft ]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
							Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC R Foot [Rft ]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
							Runtime::PlayImpactEffect(tiny, "GtsBloodSprayImpactSetVoreMedium", "NPC Spine [Spn0]", NiPoint3{dis(gen), 0, -1}, 512, true, false);
						} else {
							Runtime::PlaySound("BloodGushSound", tiny, 1.0, 0.5);
						}
					}

					AddSMTDuration(giant, 5.0);

					ApplyShakeAtNode(tiny, 20, "NPC Root [Root]", 20.0);

					ActorHandle giantHandle = giant->CreateRefHandle();
					ActorHandle tinyHandle = tiny->CreateRefHandle();
					std::string taskname = std::format("STN {}", tiny->formID);

					log::info("Creating transfer task");

					TaskManager::RunOnce(taskname, [=](auto& update){
						if (!tinyHandle) {
							return;
						}
						if (!giantHandle) {
							return;
						}
						auto giant = giantHandle.get().get();
						auto tiny = tinyHandle.get().get();
						float scale = get_visual_scale(tiny);
						TransferInventory(tiny, giant, scale, false, true, DamageSource::Crushed, true);
						log::info("Transfer task succeeded");
						// Actor reset is done within TransferInventory
					});
					if (tiny->formID != 0x14) {
						Disintegrate(tiny, true); // Set critical stage 4 on actors
						log::info("SetCritStage 4");
					} else {
						TriggerScreenBlood(50);
						tiny->SetAlpha(0.0); // Player can't be disintegrated, so we make player Invisible
						log::info("Setting alpha to 0");
					}

					if (tinyHandle) {
						Runtime::PlaySound("ShrinkToNothingSound", tinyHandle.get().get(), 1.0, 0.5);
					}
					log::info("Shrinking finished");
					data.state = ShrinkState::Shrinked;
				}
			}
		}
	}


	void ShrinkToNothingManager::Reset() {
		this->data.clear();
	}

	void ShrinkToNothingManager::ResetActor(Actor* actor) {
		if (actor) {
			this->data.erase(actor->formID);
		}
	}

	void ShrinkToNothingManager::Shrink(Actor* giant, Actor* tiny) {
		if (!tiny) {
			return;
		}
		if (!giant) {
			return;
		}
		if (ShrinkToNothingManager::CanShrink(giant, tiny)) {
			ShrinkToNothingManager::GetSingleton().data.try_emplace(tiny->formID, giant);
		}
	}

	bool ShrinkToNothingManager::AlreadyShrinked(Actor* actor) {
		if (!actor) {
			return false;
		}
		auto& m = ShrinkToNothingManager::GetSingleton().data;
		return !(m.find(actor->formID) == m.end());
	}

	bool ShrinkToNothingManager::CanShrink(Actor* giant, Actor* tiny) {
		if (ShrinkToNothingManager::AlreadyShrinked(tiny)) {
			return false;
		}
		if (IsEssential(tiny)) {
			return false;
		}

		return true;
	}

	ShrinkData::ShrinkData(Actor* giant) :
		delay(Timer(0.01)),
		state(ShrinkState::Healthy),
		giant(giant ? giant->CreateRefHandle() : ActorHandle()) {
	}
}
