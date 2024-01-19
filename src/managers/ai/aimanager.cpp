#include "managers/animation/Controllers/HugController.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/ThighSandwichController.hpp"
#include "managers/animation/HugShrink.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/ai/AiManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "spring.hpp"
#include "node.hpp"

namespace {

	const float MINIMUM_STOMP_DISTANCE = 50.0;
	const float MINIMUM_STOMP_SCALE_RATIO = 1.75;
	const float STOMP_ANGLE = 50;
	const float PI = 3.14159;

	void HealOrShrink(Actor* giant, Actor* tiny, int rng) {
		bool hostile = IsHostile(giant, tiny);
		
		if (hostile || rng <= 1) { // chance to get drained by follower
			AnimationManager::StartAnim("Huggies_Shrink", giant);
			AnimationManager::StartAnim("Huggies_Shrink_Victim", tiny);
		} else { // else heal
			StartHealingAnimation(giant, tiny);
		}
	}

	bool CanHugCrush(Actor* giant, Actor* tiny, int rng) {
		int crush_rng = rand() % 4;

		float health = GetHealthPercentage(tiny);
		float HpThreshold = GetHugCrushThreshold(giant);

		bool low_hp = (health <= HpThreshold);
		bool allow_perform = (tiny->formID != 0x14 && IsHostile(giant, tiny)) || (rng <= 1);
		bool Can_HugCrush = (low_hp && allow_perform);

		float stamina = GetStaminaPercentage(giant);
		bool Can_Force = Runtime::HasPerkTeam(giant, "HugCrush_MightyCuddles") && IsHostile(giant, tiny);

		if (Can_Force && crush_rng <= 1 && stamina >= 0.50) {
			return true;
		}
		if (Can_HugCrush) {
			return true;
		}
		return false;
	}

	void DoSandwich(Actor* pred) {
		if (!Persistent::GetSingleton().Sandwich_Ai || IsCrawling(pred)) {
			return;
		}
		auto& Sandwiching = ThighSandwichController::GetSingleton();
		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerkTeam(pred, "MassVorePerk")) {
			numberOfPrey = 1 + (get_visual_scale(pred)/3);
		}
		std::vector<Actor*> preys = Sandwiching.GetSandwichTargetsInFront(pred, numberOfPrey);
		for (auto prey: preys) {
			if (CanPerformAnimationOn(pred, prey)) { // player check is done inside CanSandwich()
				Sandwiching.StartSandwiching(pred, prey);
				auto node = find_node(pred, "GiantessRune", false);
				if (node) {
					node->local.scale = 0.01;
					update_node(node);
				}
			}
		}
	}

	void StartHugsTask(Actor* giant, Actor* tiny) {
		std::string name = std::format("Huggies_Forced_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		static Timer ActionTimer = Timer(2.5);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();

			bool AllyHugged;
			bool IsDead = (tinyref->IsDead() || giantref->IsDead());
			tinyref->GetGraphVariableBool("GTS_IsFollower", AllyHugged);

			if (!HugShrink::GetHuggiesActor(giantref)) {
				if (!AllyHugged) {
					PushActorAway(giantref, tinyref, 1.0);
				}
				return false;
			}

			if (ActionTimer.ShouldRunFrame()) {
				int rng = rand() % 20;
				if (rng < 12) {
					if (!Runtime::HasPerkTeam(giantref, "HugCrush_LovingEmbrace")) {
						rng = 1; // always force crush and always shrink
					}	
					
					if (CanHugCrush(giantref, tinyref, rng)) {
						AnimationManager::StartAnim("Huggies_HugCrush", giantref);
						AnimationManager::StartAnim("Huggies_HugCrush_Victim", tinyref);
					} else {
						HealOrShrink(giant, tiny, rng);
					}
				}
			}
			if (IsDead) {
				return false;
			}
			return true;
		});
	}

	void StartHugs(Actor* pred, Actor* prey) {
		auto& hugging = HugAnimationController::GetSingleton();
		auto& persist = Persistent::GetSingleton();
		if (!hugging.CanHug(pred, prey)) {
			return;
		}
		if (prey->formID != 0x14 && !IsHostile(pred, prey) || !IsTeammate(pred) || (!pred->IsInCombat() && persist.vore_combatonly)) {
			return;
		}
		if (prey->formID == 0x14 && !persist.vore_allowplayervore || !CanPerformAnimationOn(pred, prey)) {
			return;
		}
		HugShrink::GetSingleton().HugActor(pred, prey);
		AnimationManager::StartAnim("Huggies_Try", pred);
		AnimationManager::StartAnim("Huggies_Try_Victim", prey);
		StartHugsTask(pred, prey);
	}

	void DoHugs(Actor* pred) {
		if (!Persistent::GetSingleton().Hugs_Ai || IsCrawling(pred)) {
			return;
		}
		if (IsGtsBusy(pred)) {
			return;
		}
		int rng = rand() % 7;
		if (rng >= 6) {
			if (CanDoPaired(pred) && !IsSynced(pred) && !IsTransferingTiny(pred)) {
				auto& hugs = HugAnimationController::GetSingleton();
				std::size_t numberOfPrey = 1;
				std::vector<Actor*> preys = hugs.GetHugTargetsInFront(pred, numberOfPrey);
				for (auto prey: preys) {
					float sizedifference = get_visual_scale(pred)/get_visual_scale(prey);
					if (sizedifference > 0.98 && sizedifference < GetHugShrinkThreshold(pred)) {
						StartHugs(pred, prey);
					}
				}
			}
		}
	}

	void StrongStomp(Actor* pred, int rng) {
		if (!Persistent::GetSingleton().Stomp_Ai) {
			return; // don't check any further if it is disabled
		}
		if (rng <= 5) {
			AnimationManager::StartAnim("StrongStompRight", pred);
		} else {
			AnimationManager::StartAnim("StrongStompLeft", pred);
		}
	}
	void LightStomp(Actor* pred, int rng) {
		if (!Persistent::GetSingleton().Stomp_Ai) {
			return; // don't check any further if it is disabled
		}
		if (rng <= 5) {
			AnimationManager::StartAnim("StompRight", pred);
		} else {
			AnimationManager::StartAnim("StompLeft", pred);
		}
	}

	void Tramples(Actor* pred, int rng) {
		if (!Persistent::GetSingleton().Stomp_Ai) {
			return;
		}
		if (rng <= 5) {
			AnimationManager::StartAnim("TrampleL", pred);
		} else {
			AnimationManager::StartAnim("TrampleR", pred);
		}
	}

	void Kicks(Actor* pred, int rng) {
		if (!Persistent::GetSingleton().Kick_Ai) {
			return;
		}
		if (rng <= 3) {
			AnimationManager::StartAnim("SwipeHeavy_Right", pred);
		} else if (rng <= 4) {
			AnimationManager::StartAnim("SwipeHeavy_Left", pred);
		} else if (rng <= 6) {
			AnimationManager::StartAnim("SwipeLight_Left", pred);
		} else {
			AnimationManager::StartAnim("SwipeLight_Right", pred);
		}
	}

	void FastButtCrush(Actor* pred) {
		if (!Persistent::GetSingleton().Butt_Ai) {
			return;
		}
		AnimationManager::StartAnim("ButtCrush_StartFast", pred);
	}

	void DoStompAndButtCrush(Actor* pred) {
		int butt_rng = rand() % 10;
		int random = rand() % 10;
		int actionrng = rand() % 10;
		std::size_t amount = 6;
		std::vector<Actor*> preys = AiManager::GetSingleton().RandomStomp(pred, amount);
		for (auto prey: preys) {
			if (AiManager::GetSingleton().CanStomp(pred, prey)) {
				if (random <= 2 && butt_rng <= 2 && Persistent::GetSingleton().Butt_Ai) {
					FastButtCrush(pred);
					return;
				} else if (random <= 3) {
					StrongStomp(pred, actionrng);
					return;
				} else if (random <= 6) {
					LightStomp(pred, actionrng);
					return;
				} else if (random <= 8) {
					Kicks(pred, actionrng);
					return;
				} else if (random <= 9) {
					Tramples(pred, actionrng);
					return;
				}
			}
		}
	}

	void AnimationAttempt(Actor* actor) {
		float scale = std::clamp(get_visual_scale(actor), 1.0f, 6.0f);
		int rng = rand() % 100;
		if (rng > 7 && rng < 33 * scale) {
			DoStompAndButtCrush(actor);
			return;
		} else if (rng > 2 && rng < 7) {
			DoSandwich(actor);
			return;
		} else if (rng <= 1) {
			DoHugs(actor);
			return;
		}

		// Random Vore is managed inside Vore.cpp, RandomVoreAttempt(Actor* pred) function
	}
}


namespace Gts {
	AiData::AiData(Actor* giant) : giant(giant? giant->CreateRefHandle() : ActorHandle()) {
	}

	AiManager& AiManager::GetSingleton() noexcept {
		static AiManager instance;
		return instance;
	}

	std::string AiManager::DebugName() {
		return "AiManager";
	}

	void AiManager::Update() {
		auto profiler = Profilers::Profile("Ai: Update");
		static Timer ActionTimer = Timer(0.80);
		if (ActionTimer.ShouldRun()) {
			auto& persist = Persistent::GetSingleton();
			for (auto actor: find_actors()) {
				std::vector<Actor*> AbleToAct = {};
				for (auto actor: find_actors()) {
					if (IsTeammate(actor) && actor->formID != 0x14 && IsFemale(actor) || (EffectsForEveryone(actor) && IsFemale(actor))) {
						if (actor->IsInCombat() || !persist.vore_combatonly) {
							AbleToAct.push_back(actor);
						}
					}
				}
				if (!AbleToAct.empty()) {
					int idx = rand() % AbleToAct.size();
					Actor* Performer = AbleToAct[idx];
					if (Performer) {
						AnimationAttempt(Performer);
					}
				}
			}
		}
	}


	std::vector<Actor*> AiManager::RandomStomp(Actor* pred, std::size_t numberOfPrey) {
		// Get targets in front
		auto& sizemanager = SizeManager::GetSingleton();
		if (IsGtsBusy(pred)) {
			return {};
		}
		if (!pred) {
			return {};
		}
		auto charController = pred->GetCharController();
		if (!charController) {
			return {};
		}
		NiPoint3 predPos = pred->GetPosition();
		auto preys = find_actors();
		// Sort prey by distance
		sort(preys.begin(), preys.end(),
		     [predPos](const Actor* preyA, const Actor* preyB) -> bool
		{
			float distanceToA = (preyA->GetPosition() - predPos).Length();
			float distanceToB = (preyB->GetPosition() - predPos).Length();
			return distanceToA < distanceToB;
		});

		// Filter out invalid targets
		preys.erase(std::remove_if(preys.begin(), preys.end(),[pred, this](auto prey)
		{
			return !this->CanStomp(pred, prey);
		}), preys.end());

		// Filter out actors not in front
		auto actorAngle = pred->data.angle.z;
		RE::NiPoint3 forwardVector{ 0.f, 1.f, 0.f };
		RE::NiPoint3 actorForward = RotateAngleAxis(forwardVector, -actorAngle, { 0.f, 0.f, 1.f });

		NiPoint3 predDir = actorForward;
		predDir = predDir / predDir.Length();
		preys.erase(std::remove_if(preys.begin(), preys.end(),[predPos, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - predPos;
			if (preyDir.Length() <= 1e-4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= 0; // 180 degress
		}), preys.end());
		// Filter out actors not in a truncated cone
		// \      x   /
		//  \  x     /
		//   \______/  <- Truncated cone
		//   | pred |  <- Based on width of pred
		//   |______|
		float predWidth = 70 * get_visual_scale(pred);
		float shiftAmount = fabs((predWidth / 2.0) / tan(STOMP_ANGLE/2.0));
		NiPoint3 coneStart = predPos - predDir * shiftAmount;
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predWidth, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= predWidth*0.4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= cos(STOMP_ANGLE*PI/180.0);
		}), preys.end());
		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}
		return preys;
	}

	bool AiManager::CanStomp(Actor* pred, Actor* prey) {
		if (pred == prey) {
			return false;
		}
		if (IsGtsBusy(pred)) {
			return false;
		}
		if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore || !CanPerformAnimationOn(pred, prey)) {
			return false;
		}
		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey) * GetScaleAdjustment(prey);

		float bonus = 1.0;
		if (IsCrawling(pred)) {
			bonus = 2.0; // +100% stomp distance
		}
		if (prey->IsDead() && pred_scale/prey_scale < 8.0) {
			return false;
		}

		float sizedifference = pred_scale/prey_scale;

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if (pred->formID == 0x14 && prey_distance <= (MINIMUM_STOMP_DISTANCE * pred_scale * bonus) && pred_scale/prey_scale < MINIMUM_STOMP_SCALE_RATIO) {
			return false;
		}
		if (prey_distance <= (MINIMUM_STOMP_DISTANCE * pred_scale * bonus)
		    && pred_scale/prey_scale > MINIMUM_STOMP_SCALE_RATIO
		    && prey_distance > 25.0) { // We don't want the Stomp to be too close
			return true;
		} else {
			return false;
		}
	}

	void AiManager::Reset() {
		this->data_ai.clear();
	}

	void AiManager::ResetActor(Actor* actor) {
		this->data_ai.erase(actor->formID);
	}

	AiData& AiManager::GetAiData(Actor* giant) {
		// Create it now if not there yet
		this->data_ai.try_emplace(giant->formID, giant);

		return this->data_ai.at(giant->formID);
	}
}
