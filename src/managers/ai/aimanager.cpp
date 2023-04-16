#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/ThighSandwichController.hpp"
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
	const float STOMP_ANGLE = 60;
	const float PI = 3.14159;

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
		if (!Runtime::HasPerk(PlayerCharacter::GetSingleton(), "KillerThighs")) {
			return false;
		}
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

    void DoStomp(Actor* pred) {
		if (!Runtime::HasPerk(PlayerCharacter::GetSingleton(), "DestructionBasics")) {
			log::info("No matching perk");
			return;
		}
		if (!Persistent::GetSingleton().Stomp_Ai) {
			log::info("Stomp AI is false");
			return;
		}
        int random = rand() % 4;
        int actionrng = rand() % 4;
        std::size_t amount = 6;
        std::vector<Actor*> preys = AiManager::GetSingleton().RandomStomp(pred, amount);
        for (auto prey: preys) {
            log::info("Doing Stomp as {}, random:{}, action rng: {}", pred->GetDisplayFullName(), random, actionrng);
            if (AiManager::GetSingleton().CanStomp(pred, prey)) {
                if (random <= 2) {
                    if (actionrng <= 2) {
                        AnimationManager::StartAnim("StompRight", pred);
                    } else {
                        AnimationManager::StartAnim("StompLeft", pred);
                    }
                } else if (random > 2) {
                    if (actionrng <= 2) {
                        AnimationManager::StartAnim("StrongStompRight", pred);
                    } else {
                        AnimationManager::StartAnim("StrongStompLeft", pred);
                    }
                }
            }
        }
    }

	void AnimationAttempt(Actor* actor) {
        float scale = std::clamp(get_visual_scale(actor), 1.0f, 6.0f);
        int rng = rand() % 40;
        log::info("RNG: {}, scale: {}", rng, scale);
        if (rng > 2 && rng < 6 * scale) {
            log::info("RNG < {}, doing stomp", 6 * scale);
            DoStomp(actor);
        } else if (rng < 2) {
            DoSandwich(actor);   
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
        Profilers::Start("Ai: Update");
		static Timer ActionTimer = Timer(0.80);
		if (ActionTimer.ShouldRun()) {
			auto& persist = Persistent::GetSingleton();
       	 	for (auto actor: find_actors()) {
				std::vector<Actor*> AbleToAct = {};
				for (auto actor: find_actors()) {
					if (actor->formID != 0x14 && (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) && (actor->IsInCombat() || !persist.vore_combatonly)) {
						AbleToAct.push_back(actor);
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
        	Profilers::Stop("Ai: Update");
    	}
	}


    std::vector<Actor*> AiManager::RandomStomp(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for actor
		auto& sizemanager = SizeManager::GetSingleton();
		if (IsGtsBusy(pred)) {
            log::info("{} is Busy", pred->GetDisplayFullName());
			return {};
		}
		if (!pred) {
            log::info("Pred is none");
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
		} if (IsGtsBusy(pred)) {
			log::info("Pred is busy");
            return false;
        } if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore) {
			log::info("Prey is protected");
			return false;
		}
		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);
		if (IsDragon(prey)) {
			prey_scale *= 3.0;
		} if (prey->IsDead() && pred_scale/prey_scale < 8.0) {
			log::info("prey is dead and size difference is not met");
			return false;
		}

		float sizedifference = pred_scale/prey_scale;

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if (pred->formID == 0x14 && prey_distance <= (MINIMUM_STOMP_DISTANCE * pred_scale) && pred_scale/prey_scale < MINIMUM_STOMP_SCALE_RATIO) {
			log::info("Can't stomp");
			return false;
		}
		if (prey_distance <= (MINIMUM_STOMP_DISTANCE * pred_scale) 
            && pred_scale/prey_scale > MINIMUM_STOMP_SCALE_RATIO 
            && prey_distance > 25.0) { // We don't want the Stomp to be too close
			log::info("Stomp true");
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