#include "managers/animation/Controllers/HugController.hpp"
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

	float GetCrushThreshold(Actor* actor) {
		float hp = 0.20;
		if (Runtime::HasPerkTeam(actor, "HugCrush_MightyCuddles")) {
			hp += 0.10; // 0.30
		} if (Runtime::HasPerkTeam(actor, "HugCrush_HugsOfDeath")) {
			hp += 0.20; // 0.50
		}
		return hp;
	}

	[[nodiscard]] inline RE::NiPoint3 RotateAngleAxis(const RE::NiPoint3& vec, const float angle, const RE::NiPoint3& axis)
	{
		float S = sin(angle);
		float C = cos(angle);

		const float XX = axis.x * axis.x;
		const float YY = axis.y * axis.y;
		const float ZZ = axis.z * axis.z;

		const float XY = axis.x * axis.y;
		const float YZ = axis.y * axis.z;
		const float ZX = axis.z * axis.x;

		const float XS = axis.x * S;
		const float YS = axis.y * S;
		const float ZS = axis.z * S;

		const float OMC = 1.f - C;

		return RE::NiPoint3(
			(OMC * XX + C) * vec.x + (OMC * XY - ZS) * vec.y + (OMC * ZX + YS) * vec.z,
			(OMC * XY + ZS) * vec.x + (OMC * YY + C) * vec.y + (OMC * YZ - XS) * vec.z,
			(OMC * ZX - YS) * vec.x + (OMC * YZ + XS) * vec.y + (OMC * ZZ + C) * vec.z
			);
	}

	void DoSandwich(Actor* pred) {
		if (!Persistent::GetSingleton().Sandwich_Ai) {
			log::info("Sandwich AI is false");
			return;
		}
		auto& Sandwiching = ThighSandwichController::GetSingleton();
		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerkTeam(pred, "MassVorePerk")) {
			numberOfPrey = 1 + (get_visual_scale(pred)/3);
		}
		std::vector<Actor*> preys = Sandwiching.GetSandwichTargetsInFront(pred, numberOfPrey);
		for (auto prey: preys) {
			Sandwiching.StartSandwiching(pred, prey);
			auto node = find_node(pred, "GiantessRune", false);
			if (node) {
				node->local.scale = 0.01;
				update_node(node);
			}
		}
	}

	void DoHugs(Actor* pred) {
		if (IsGtsBusy(pred)) {
			return;
		}
		auto& Sandwiching = ThighSandwichController::GetSingleton();
		std::size_t numberOfPrey = 1;
		std::vector<Actor*> preys = Sandwiching.GetSandwichTargetsInFront(pred, numberOfPrey);
		for (auto prey: preys) {
			float sizedifference = get_visual_scale(pred)/get_visual_scale(prey);
			if (sizedifference > 0.9 && sizedifference < 3.0) {
				HugAnimationController::StartHug(pred, prey);
				HugShrink::AttachActorTask(pred, prey);
				StartHugsTask(pred, prey);
			}
		}
	}

	void StartHugsTask(Actor* giant, Actor* tiny) {
		std::string name = std::format("Huggies_Forced_{}", giant->formID);
		ActorHandle gianthandle = giant->CreateRefHandle();
		ActorHandle tinyhandle = tiny->CreateRefHandle();
		static timer ActionTimer = Timer(1.5);
		TaskManager::Run(name, [=](auto& progressData) {
			if (!gianthandle) {
				return false;
			}
			if (!tinyhandle) {
				return false;
			}
			auto giantref = gianthandle.get().get();
			auto tinyref = tinyhandle.get().get();
			if (ActionTimer.ShouldRunFrame()) {
				int rng = rand() % 10;
				if (rng < 6) {
					float health = GetHealthPercentage(tinyref);	
					float HpThreshold = GetCrushThreshold(giantref);
					if (health <= HpThreshold) {
						AnimationManager::StartAnim("Huggies_HugCrush", player);
						AnimationManager::StartAnim("Huggies_HugCrush_Victim", huggedActor);
					} else {
						AnimationManager::StartAnim("Huggies_Shrink", giantref);
						AnimationManager::StartAnim("Huggies_Shrink_Victim", tinyref);
					}
				}
			}
			log::info("Hugs Task is running");
			if (!HugShrink::GetHuggiesActor(gianthandle)) {
				return false;
			}
			return true;
		});	
	}

	void StrongStomp(Actor* pred, int rng) {
		if (rng <= 5) {
			AnimationManager::StartAnim("StrongStompRight", pred);
		} else {
			AnimationManager::StartAnim("StrongStompLeft", pred);
		}
	}
	void LightStomp(Actor* pred, int rng) {
		if (rng <= 5) {
			AnimationManager::StartAnim("StompRight", pred);
		} else {
			AnimationManager::StartAnim("StompLeft", pred);
		}
	}
	void Kicks(Actor* pred, int rng) {
		if (rng <= 2) {
			AnimationManager::StartAnim("HeavyKickRight", pred);
		} else if (rng <= 3) {
			AnimationManager::StartAnim("HeavyKickLeft", pred);
		} else if (rng <= 6) {
			AnimationManager::StartAnim("LightKickLeft", pred);
		} else {
			AnimationManager::StartAnim("LightKickRight", pred);
		}
	}

	void DoStomp(Actor* pred) {
		if (!Persistent::GetSingleton().Stomp_Ai) {
			return;
		}
		int random = rand() % 10;
		int actionrng = rand() % 10;
		std::size_t amount = 6;
		std::vector<Actor*> preys = AiManager::GetSingleton().RandomStomp(pred, amount);
		for (auto prey: preys) {
			if (AiManager::GetSingleton().CanStomp(pred, prey)) {
				if (random <= 3) {
					StrongStomp(pred, actionrng);
					return;
				} else if (random <= 7) {
					LightStomp(pred, actionrng);
					return;
				} else if (random <= 9) {
					Kicks(pred, actionrng);
					return;
				} 
			}
		}
	}

	void AnimationAttempt(Actor* actor) {
		float scale = std::clamp(get_visual_scale(actor), 1.0f, 6.0f);
		int rng = rand() % 40;
		if (rng > 2 && rng < 6 * scale) {
			DoStomp(actor);
		} else if (rng < 2) {
			DoSandwich(actor);
		} else if (rng < 1) {
			DoHugs(actor);
		}
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
					if (IsTeammate(actor) && actor->formID != 0x14) {
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
		// Get vore target for actor
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
		if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore) {
			return false;
		}
		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);
		if (IsDragon(prey)) {
			prey_scale *= 3.0;
		}
		if (prey->IsDead() && pred_scale/prey_scale < 8.0) {
			return false;
		}

		float sizedifference = pred_scale/prey_scale;

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if (pred->formID == 0x14 && prey_distance <= (MINIMUM_STOMP_DISTANCE * pred_scale) && pred_scale/prey_scale < MINIMUM_STOMP_SCALE_RATIO) {
			return false;
		}
		if (prey_distance <= (MINIMUM_STOMP_DISTANCE * pred_scale)
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
