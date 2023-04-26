#include "managers/damage/AccurateDamage.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/findActor.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "data/re.hpp"
#include "timer.hpp"
#include "node.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	void StartCombat(Actor* giant, Actor* tiny, bool Forced) {
		if (!Runtime::GetBool("HostileDamage")) {
			return;
		}
		static Timer tick = Timer(0.25);
		if (tick.ShouldRunFrame() || Forced == true) {
			if (tiny->IsInCombat() || tiny->IsDead()) {
				return;
			}
			if (Forced == true || GetAV(tiny, ActorValue::kHealth) < GetMaxAV(tiny, ActorValue::kHealth) * 0.90) {
				CallFunctionOn(tiny, "Actor", "StartCombat", giant);
			}
		}
	}

	void ScareActors(Actor* giant) {
		Profilers::Start("ActorUtils: ScareActors");
		for (auto tiny: find_actors()) {
			if (tiny != giant && tiny->formID != 0x14 && !IsTeammate(tiny)) {
				 if (tiny->IsDead()) {
					return;
				}
				float random = GetRandomBoost();
				static Timer runtimer = Timer(1.0);
				static Timer combattimer = Timer(2.0);
				float GiantScale = get_visual_scale(giant);
				float TinyScale = get_visual_scale(tiny);
				float sizedifference = std::clamp(GiantScale/TinyScale, 0.10f, 12.0f);
				float distancecheck = 226.0 * GetMovementModifier(giant);
				if (sizedifference >= 2.5) {
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
										if (runtimer.ShouldRunFrame()) {
											if (!combat) {
												tiny->InitiateFlee(TinyRef, true, true, true, cell, TinyRef, 100.0, 465.0 * sizedifference);
											} 
										} if (combat && combattimer.ShouldRunFrame() && GetRandomBoost() <= 0.050 * (sizedifference)) {
											std::vector<Actor*> FearList = {};
											FearList.push_back(tiny);
											if (!FearList.empty()) {
												int idx = rand() % FearList.size();
												Actor* FearReceiver = FearList[idx];
												if (FearReceiver) {
													auto ReceiverRef = skyrim_cast<TESObjectREFR*>(FearReceiver);
													if (ReceiverRef) {
														FearReceiver->InitiateFlee(ReceiverRef, true, true, true, cell, ReceiverRef, 100.0, 265.0 * sizedifference);
														combat->startedCombat = true;
														combat->ignoringCombat = false;
														combat->state->isFleeing = true;
														FearList = {};
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
			}
			Profilers::Stop("ActorUtils: ScareActors");
		}
	}
		

	void ReportCrime(Actor* giant, Actor* tiny, float value, bool combat) {
		Profilers::Start("ActorUtils: ReportCrime");
		if (!Runtime::GetBool("HostileDamage")) {
			return;
		}
		static Timer tick = Timer(0.10);
		bool SeeingOther;
		if (tick.ShouldRunFrame()) {
			for (auto otherActor: find_actors()) {
				float scale = std::clamp(get_visual_scale(giant), 0.10f, 12.0f);
				auto Ref = skyrim_cast<TESObjectREFR*>(tiny);
				if (Ref) {
					bool IsTrue = otherActor->HasLineOfSight(Ref, SeeingOther);
					NiPoint3 ObserverDist = otherActor->GetPosition();
					NiPoint3 VictimDist = tiny->GetPosition();
					float distance = (ObserverDist - VictimDist).Length();
					if (IsTrue || distance < 524 * scale) {
						if (otherActor != tiny && tiny->formID != 0x14) {
							auto Faction = tiny->GetCrimeFaction();
							auto OtherFaction = otherActor->GetCrimeFaction();
							auto CombatValue = giant->GetCrimeGoldValue(Faction);
							if (combat && OtherFaction == Faction) {
								StartCombat(giant, otherActor, true);
								tiny->GetActorRuntimeData().myKiller = giant->CreateRefHandle();
								tiny->GetActorRuntimeData().currentCombatTarget = giant->CreateRefHandle();
								tiny->UpdateCombatControllerSettings();
							} 
							else if (!combat) {
								if (giant->formID == 0x14 && CombatValue < 1000) {
									giant->ModCrimeGoldValue(Faction, true, value);
								}
							}
						}
					}
				}
			}
		}
		Profilers::Stop("ActorUtils: ReportCrime");
	}
}
