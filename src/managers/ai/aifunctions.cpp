#include "managers/damage/CollisionDamage.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "utils/papyrusUtils.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/papyrusUtils.hpp"
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
		}
		if (IsCrawling(giant)) {
			threshold += 1.45;
		}
		if (IsProning(giant)) {
			threshold += 1.45;
		}
		if (giant->AsActorState()->IsWalking()) { // harder to scare if we're approaching slowly
			threshold *= 1.35;
		}
		if (giant->IsRunning()) { // easier to scare
			threshold *= 0.75;
		}
		return threshold;
	}

	/*template <class ... Args>
	   void SendDeathEvent(Actor* giant, Actor* tiny, bool dead) {
	        if (!tiny->IsDead()) {
	                return; // apply to dead actors only
	        }
	        const auto skyrimVM = RE::SkyrimVM::GetSingleton();
	        auto VM = skyrimVM ? skyrimVM->impl : nullptr;
	        if (VM) {
	                TESObjectREFR* DyingCast = skyrim_cast<TESObjectREFR*>(tiny);
	                TESObjectREFR* KillerCast = skyrim_cast<TESObjectREFR*>(giant);

	                auto Dying = DyingCast->CreateRefHandle();
	                auto Killer = KillerCast->CreateRefHandle();

	                if (!Dying) {
	                        return;
	                } if (!Killer) {
	                        return;
	                }

	                std::uint8_t pad11;
	                std::uint16_t pad12;
	                std::uint32_t pad14;
	                auto handle = Dying.get().get();
	                BSFixedString EventName = "TESDeathEvent";

	                auto args = RE::MakeFunctionArguments(std::forward<Args>(Dying, Killer, dead, pad11, pad12, pad14)...);

	                VM->RelayEvent(handle, &EventName, args, nullptr);
	                //SkyrimVM::RelayEvent(VMHandle a_handle, BSFixedString* a_event, BSScript::IFunctionArguments* a_args, SkyrimVM::ISendEventFilter* a_optionalFilter)
	                }
	        }*/

	void KillProperly_5d4700(Actor* dying, Actor* killer, uintptr_t param_2, uintptr_t silent, uintptr_t param_4, uintptr_t param_5) {
		// SE Address: 5d4700   AE: ??????
		// ---Works like this based on RE:---
		// 1) papyrus__Actor::KillSilent_14094B760 (No event?)
		// 1) papyrus__Actor::Kill_14094B790 (Alarm?)
		// -> call 2)
		// 2) Actor::Kill_Papyrus_impl_14095D0E0 -> 3)
		// 3) void Actor::Kill_impl2_1405D4700 (Current function that i'm trying to RE)
		// Actor *param_1,undefined param_2,undefined silent?,uint8 param_4,uint8 param_5
		typedef void (*DefKillProperly)(Actor* dying, Actor* killer, uintptr_t param_2, uintptr_t silent, uintptr_t param_4, uintptr_t param_5);
		REL::Relocation<DefKillProperly> SkyrimKillProperly{ RELOCATION_ID(36607, 37615) };
		SkyrimKillProperly(dying, killer, param_2, silent, param_4, param_5);
	}

	void KillProperly_Papyrus(Actor* dying, Actor* killer, uintptr_t value) {
		// 94B790 = 53902
		// param 1 = damage?
		typedef void (*DefKill_Papyrus)(Actor* dying, Actor* killer, uintptr_t value);
		REL::Relocation<DefKill_Papyrus> SkyrimKill_Papyrus{ RELOCATION_ID(53902, 37615) };
		SkyrimKill_Papyrus(dying, killer, value);
	}
}

namespace Gts {
	void KillActor(Actor* giant, Actor* tiny) {

		InflictSizeDamage(giant, tiny, GetAV(tiny, ActorValue::kHealth) * 2); // just to make sure
		tiny->KillImpl(giant, 1, true, true);
		auto* eventsource = ScriptEventSourceHolder::GetSingleton();
		if (eventsource) {
			auto event = TESDeathEvent();
			event.actorDying = skyrim_cast<TESObjectREFR*>(tiny)->CreateRefHandle().get();
			event.actorKiller = skyrim_cast<TESObjectREFR*>(giant)->CreateRefHandle().get();
			event.dead = true;
			eventsource->SendEvent(&event);
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
