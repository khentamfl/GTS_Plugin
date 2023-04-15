#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/ThighSandwichController.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "spring.hpp"
#include "node.hpp"

 namespace {

    const float MINIMUM_SANDWICH_DISTANCE = 70.0;
	const float MINIMUM_SANDWICH_SCALE_RATIO = 6.0;
	const float SANDWICH_ANGLE = 60;
	const float PI = 3.14159;

    void PrintSuffocate(Actor* pred, Actor* prey) {
        int random = rand() % 5;
		if (random <= 1) {
			Cprint("{} was slowly smothered between {} thighs", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 2) {
			Cprint("{} was suffocated by the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 3) {
			Cprint("Thighs of {} suffocated {} to death", pred->GetDisplayFullName(), prey->GetDisplayFullName());
		} else if (random == 4) {
			Cprint("{} got smothered between the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 5) {
			Cprint("{} lost life to the thighs of {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		}
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
 }


 namespace Gts {
    SandwichingData::SandwichingData(Actor* giant) : giant(giant? giant->CreateRefHandle() : ActorHandle()) {
	}

	void SandwichingData::AddTiny(Actor* tiny) {
		this->tinies.try_emplace(tiny->formID, tiny->CreateRefHandle());
	}

    std::vector<Actor*> SandwichingData::GetActors() {
		std::vector<Actor*> result;
		for (auto& [key, actorref]: this->tinies) {
			auto actor = actorref.get().get();
			result.push_back(actor);
		}
		return result;
	}

    ThighSandwichController& ThighSandwichController::GetSingleton() noexcept {
		static ThighSandwichController instance;
		return instance;
	}

	std::string ThighSandwichController::DebugName() {
		return "ThighSandwichController";
	}

	void SandwichingData::UpdateRune(Actor* giant) {
		string node_name = "GiantessRune";
		if (this->RuneShrink == true) {
			auto node = find_node(giant, node_name, false);
			if (node) {
				this->ShrinkRune.halflife = 0.7/AnimationManager::GetAnimSpeed(giant);
				this->ShrinkRune.target = 1.0;
				this->ScaleRune.value = 0.0;
				this->ScaleRune.target = 0.0;
				node->local.scale = 1.0 - this->ShrinkRune.value;
				//log::info("Shrink Rune Value: {}", this->ShrinkRune.value);
				update_node(node);
			}
		}
		else if (this->RuneScale == true) {
			auto node = find_node(giant, node_name, false);
			if (node) {
				this->ScaleRune.halflife = 0.6/AnimationManager::GetAnimSpeed(giant);
				this->ScaleRune.target = 1.0;
				this->ShrinkRune.value = 0.0;
				node->local.scale = this->ScaleRune.value;
				//log::info("Scale Rune Value: {}", this->ScaleRune.value);
				update_node(node);
			}
		}
	}

    void SandwichingData::Update() {
        auto giant = this->giant.get().get();
        if (!giant) {
            return;
        }
    	float giantScale = get_visual_scale(giant);


		SandwichingData::UpdateRune(giant);

        for (auto& [key, tinyref]: this->tinies) {
			auto tiny = tinyref.get().get();
			if (!tiny) {
				return;
			}
			auto bone = find_node(giant, "AnimObjectA");
			if (!bone) {
				return;
			}
			float tinyScale = get_visual_scale(tiny);
			NiPoint3 tinyLocation = tiny->GetPosition();
			NiPoint3 targetLocation = bone->world.translate;
        	NiPoint3 deltaLocation = targetLocation - tinyLocation;
        	float deltaLength = deltaLocation.Length();

			if (giantScale/tinyScale < 6.0) {
				PushActorAway(giant, tiny, 0.5);
				this->tinies.erase(tiny->formID); // Disallow button abuses to keep tiny when on low scale
			}

			tiny->SetPosition(targetLocation, true);
			tiny->SetPosition(targetLocation, false);
            if (this->Suffocate) {
                float sizedifference = get_visual_scale(giant)/get_visual_scale(tiny);
			    float damage = 0.005 * sizedifference;
                float hp = GetAV(tiny, ActorValue::kHealth);
			    DamageAV(tiny, ActorValue::kHealth, damage);
                if (damage > hp && !tiny->IsDead()) {
                    this->Remove(tiny);
                    PrintSuffocate(giant, tiny);
                }
            }
			Actor* tiny_is_actor = skyrim_cast<Actor*>(tiny);
			if (tiny_is_actor) {
				//ManageRagdoll(tiny_is_actor, deltaLength, deltaLocation, targetLocation);
				auto charcont = tiny_is_actor->GetCharController();
				if (charcont) {
					charcont->SetLinearVelocityImpl((0.0, 0.0, 0.0, 0.0)); // Needed so Actors won't fall down.
				}
            }
        }
    }

    void ThighSandwichController::Update() {
        for (auto& [key, SandwichData]: this->data) {
			SandwichData.Update();
		}
    }

	std::vector<Actor*> ThighSandwichController::GetSandwichTargetsInFront(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for actor
		auto& sizemanager = SizeManager::GetSingleton();
		if (sizemanager.GetActionBool(pred, 1.0)) {
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
			return !this->CanSandwich(pred, prey);
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
		float shiftAmount = fabs((predWidth / 2.0) / tan(SANDWICH_ANGLE/2.0));

		NiPoint3 coneStart = predPos - predDir * shiftAmount;
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predWidth, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= predWidth*0.4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= cos(SANDWICH_ANGLE*PI/180.0);
		}), preys.end());

		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}

		return preys;
	}

	bool ThighSandwichController::CanSandwich(Actor* pred, Actor* prey) {
		if (pred == prey) {
			return false;
		}

		if (!Runtime::HasPerkTeam(pred, "KillerThighs")) {
			return false;
		}

		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);
		if (IsDragon(prey)) {
			prey_scale *= 2.0;
		}

		float sizedifference = pred_scale/prey_scale;

		float MINIMUM_VORE_SCALE = MINIMUM_SANDWICH_SCALE_RATIO;

		float balancemode = SizeManager::GetSingleton().BalancedMode();

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if (pred->formID == 0x14 && prey_distance <= (MINIMUM_SANDWICH_DISTANCE * pred_scale) && pred_scale/prey_scale < MINIMUM_VORE_SCALE) {
			Notify("{} is too big to be smothered between thighs.", prey->GetDisplayFullName());
			return false;
		}
		if (prey_distance <= (MINIMUM_SANDWICH_DISTANCE * pred_scale) && pred_scale/prey_scale > MINIMUM_VORE_SCALE) {
				if ((prey->IsEssential() && Runtime::GetBool("ProtectEssentials")) || Runtime::HasSpell(prey, "StartVore")) {
					return false;
				} else {
					return true;
				}
			}
		else {
			return false;
		}
	}

	void ThighSandwichController::StartSandwiching(Actor* pred, Actor* prey) {
        auto& sandwiching = ThighSandwichController::GetSingleton();
		if (!sandwiching.CanSandwich(pred, prey)) {
			return;
		}
		auto& data = sandwiching.GetSandwichingData(pred);
		data.AddTiny(prey);
		AnimationManager::StartAnim("ThighEnter", pred);
	}

	void ThighSandwichController::Reset() {
		this->data.clear();
	}
	void SandwichingData::ReleaseAll() {
		this->tinies.clear();
	}

	void ThighSandwichController::ResetActor(Actor* actor) {
		this->data.erase(actor->formID);
	}

    void SandwichingData::Remove(Actor* tiny) {
        this->tinies.erase(tiny->formID);
    }

    void SandwichingData::EnableSuffocate(bool enable) {
		this->Suffocate = enable;
	}
	void SandwichingData::ManageScaleRune(bool enable) {
		this->RuneScale = enable;
	}
	void SandwichingData::ManageShrinkRune(bool enable) {
		this->RuneShrink = enable;
	}
	void SandwichingData::OverideShrinkRune(float value) {
		this->ScaleRune.value = value;
	}

    SandwichingData& ThighSandwichController::GetSandwichingData(Actor* giant) {
		// Create it now if not there yet
		this->data.try_emplace(giant->formID, giant);

		return this->data.at(giant->formID);
	}
 }
