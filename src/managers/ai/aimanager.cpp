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
	const float MINIMUM_STOMP_SCALE_RATIO = 2.5;
	const float STOMP_ANGLE = 72;
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
        if ((Runtime::HasPerk(pred, "KillerThighs"))) {
            auto& Sandwiching = ThighSandwichController::GetSingleton();
            std::size_t numberOfPrey = 1;
			if (Runtime::HasPerkTeam(pred, "MassVorePerk")) {
				numberOfPrey = 1 + (get_visual_scale(pred)/3);
			}
            std::vector<Actor*> preys = Sandwiching.GetSandwichTargetsInFront(ai, numberOfPrey);
			for (auto prey: preys) {
                if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore) {
                    return;
                }
				Sandwiching.StartSandwiching(ai, prey);
				auto node = find_node(ai, "GiantessRune", false);
				if (node) {
					node->local.scale = 0.01;
					update_node(node);
				}
			}
        }
    }

    void DoStomp(Actor* pred) {
        int random = rand() % 3;
        int actionrng = rand() % 3;
        std::size_t amount = 3;
        std::vector<Actor*> preys = AiManager::GetSingleton().RandomStomp(pred, amount);
        for (auto prey: preys) {
            log::info("Doing Stomp as {}, random:{}, action rng: {}", pred->GetDisplayFullName(), random, actionrng);
            if (AiManager::GetSingleton().CanStomp(pred, prey)) {
                if (random <= 2) {
                    if (actionrng <= 1) {
                        AnimationManager::StartAnim("StompRight", pred);
                    } else {
                        AnimationManager::StartAnim("StompLeft", pred);
                    }
                } else if (random > 2) {
                    if (actionrng <= 1) {
                        AnimationManager::StartAnim("StrongStompRight", pred);
                    } else {
                        AnimationManager::StartAnim("StrongStompLeft", pred);
                    }
                }
            }
        }
    }
 }


 namespace Gts {
    AiData::AiData(Actor* giant) : giant(giant? giant->CreateRefHandle() : ActorHandle()) {
	}

    bool AiData::GetTimer(int type) {
        if (type == 1) {
		    return this->ActionTimer.ShouldRun();
        } else if (type == 2) {
            return this->RepeatTimer.ShouldRun();
        }
        return false;
	}
    int AiData::GetRandom() {
        return this->random;
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
        for (auto actor: find_actors()) {
            auto& persist = Persistent::GetSingleton();
            if (actor->formID != 0x14 && (Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) && (actor->IsInCombat() || !persist.vore_combatonly)) {
                auto ai = GetAiData(actor);
                if (ai.GetTimer(1) == true) {
                    auto rng = ai.GetRandom();
                    if (rng < 3 && !IsGtsBusy(ai)) {
                        log::info("RNG < 3, doing stomp");
                        DoStomp(actor);
                    }
                    else if (rng < 5 && !IsGtsBusy(ai)) {
                        DoSandwich(actor);
                    }
                }
            }
        }
        Profilers::Stop("Ai: Update");
    }


    std::vector<Actor*> AiManager::RandomStomp(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for actor
		auto& sizemanager = SizeManager::GetSingleton();
		if (IsGtsBusy(pred)) {
            log::info("{} is Buy", pred->GetDisplayFullName());
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
            return false;
        } if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore) {
			return false;
		}
		if (!Runtime::HasPerkTeam(PlayerCharacter::GetSingleton(), "DestructionBasics")) {
			return false;
		}
		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);
		if (IsDragon(prey)) {
			prey_scale *= 3.0;
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