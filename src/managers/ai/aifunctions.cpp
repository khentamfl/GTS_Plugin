#include "managers/damage/AccurateDamage.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "utils/findActor.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;

namespace {
	float GetScareThreshold(Actor* giant) {
		float threshold = 2.5;
		 if (giant->IsSneaking()) { // If we sneak/prone/crawl = make threshold bigger so it's harder to scare actors
			threshold += 0.8;
		} if (IsCrawling(giant)) {
			threshold += 1.45;
		} if (IsProning(giant)) {
			threshold += 1.45;
		} if (giant->AsActorState()->IsWalking()) { // harder to scare if we're approaching slowly
			threshold *= 1.35;
		} if (giant->IsRunning()) { // easier to scare 
			threshold *= 0.75;
		}
		return threshold;
	}
}

namespace Gts {
	void KillActor(Actor* giant, Actor* tiny) {
		
			tiny->KillImpl(giant, 1000, true, true);
			//tiny->SetLifeState(ACTOR_LIFE_STATE::kDead);
			//tiny->KillImmediate();
			//tiny->KillDying();
			RagdollTask(tiny); 
		
		//tiny->GetActorRuntimeData().boolBits.set(BOOL_BITS::kDead);
		
	}

	void RagdollTask(Actor* tiny) {
		auto tinyHandle = tiny->CreateRefHandle();
		std::string name = std::format("Ragdoll_{}", tiny->formID);
		TaskManager::RunOnce(name, [=](auto& update){
			if (!tinyHandle) {
				return;
			}
			auto tiny = tinyHandle.get().get();
			tiny->NotifyAnimationGraph("Ragdoll");
			ForceRagdoll(tiny, true);
			return;
		});
	}

	void StartCombat(Actor* giant, Actor* tiny, bool Forced) {
		/*if (!Persistent::GetSingleton().hostile_toggle) {
			return;
		}*/
		if (tiny->IsDead()) {
			return;
		}
		if (Forced == true || GetAV(tiny, ActorValue::kHealth) < GetMaxAV(tiny, ActorValue::kHealth) * 0.75) {
			Attacked(tiny, giant);
		}
	}

	void ScareActors(Actor* giant) {
		auto profiler = Profilers::Profile("ActorUtils: ScareActors");
		if (!Persistent::GetSingleton().actors_panic) {
			return; // Disallow Panic if bool is false.
		}
		for (auto tiny: FindSomeActors("AiActors", 2)) {
			if (tiny != giant && tiny->formID != 0x14 && !IsTeammate(tiny)) {
				if (tiny->IsDead()) {
					return;
				}
				if (IsBeingHeld(tiny)) {
					return;
				}
				float GiantScale = get_visual_scale(giant);
				float TinyScale = get_visual_scale(tiny);
				float sizedifference = std::clamp(GiantScale/TinyScale, 0.10f, 12.0f);
				float distancecheck = 164.0 * GetMovementModifier(giant);
				float threshold = GetScareThreshold(giant);
				if (sizedifference >= threshold) {
					NiPoint3 GiantDist = giant->GetPosition();
					NiPoint3 ObserverDist = tiny->GetPosition();
					float distance = (GiantDist - ObserverDist).Length();

					if (distance <= distancecheck * sizedifference) {
						auto combat = tiny->GetActorRuntimeData().combatController;
						
						tiny->GetActorRuntimeData().currentCombatTarget = giant->CreateRefHandle();
						auto TinyRef = skyrim_cast<TESObjectREFR*>(tiny);

						if (TinyRef) {
							auto GiantRef = skyrim_cast<TESObjectREFR*>(giant);
							if (GiantRef) {
								bool SeeingOther;
								bool IsTrue = tiny->HasLineOfSight(GiantRef, SeeingOther);
								if (IsTrue || distance < (distancecheck/1.5) * sizedifference) {
									auto cell = tiny->GetParentCell();
									if (cell) {
										if (!combat) {
											tiny->InitiateFlee(TinyRef, true, true, true, cell, TinyRef, 100.0, 465.0 * sizedifference);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}

	void ScareHostileActors(Actor* giant, float dist) {
		auto profiler = Profilers::Profile("ActorUtils: ScareHostileActors");
		for (auto tiny: find_actors()) {
			if (tiny != giant && tiny->formID != 0x14 && !IsTeammate(tiny)) {
				if (tiny->IsDead()) {
					return;
				}
				if (IsBeingHeld(tiny)) {
					return;
				}
				float GiantScale = get_visual_scale(giant);
				float TinyScale = get_visual_scale(tiny);
				float sizedifference = std::clamp(GiantScale/TinyScale, 0.10f, 24.0f);
				float distancecheck = dist; // default dist is 256
				int rng = 100 * sizedifference;
				int FleeChance = rand() % rng;
				float threshold = 2.0;
				if (sizedifference >= threshold) {
					auto combat = tiny->GetActorRuntimeData().combatController;
					if (!combat) {
						return;
					}
					NiPoint3 GiantDist = giant->GetPosition();
					NiPoint3 ObserverDist = tiny->GetPosition();
					float distance = (GiantDist - ObserverDist).Length();
					if (distance <= distancecheck * sizedifference) {
						auto TinyRef = skyrim_cast<TESObjectREFR*>(tiny);
						if (TinyRef) {
							auto GiantRef = skyrim_cast<TESObjectREFR*>(giant);
							if (GiantRef) {
								auto cell = tiny->GetParentCell();
								if (cell) {
									tiny->InitiateFlee(TinyRef, true, true, true, cell, GiantRef, 400.0, 465.0 * sizedifference);
								}
							}
						}
					}
				}
			}
		}
	}
}
