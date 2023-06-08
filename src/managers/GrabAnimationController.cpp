#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/GrabAnimationController.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "managers/explosion.hpp"
#include "managers/footstep.hpp"
#include "data/persistent.hpp"
#include "managers/tremor.hpp"
#include "managers/Rumble.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "spring.hpp"
#include "node.hpp"


namespace {

	const float MINIMUM_GRAB_DISTANCE = 85.0;
	const float MINIMUM_GRAB_SCALE_RATIO = 6.0;
	const float GRAB_ANGLE = 70;
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
}

namespace Gts {
    GrabAnimationController& GrabAnimationController::GetSingleton() noexcept {
		static GrabAnimationController instance;
		return instance;
	}

	std::string GrabAnimationController::DebugName() {
		return "GrabAnimationController";
	}

    std::vector<Actor*> GrabAnimationController::GetGrabTargetsInFront(Actor* pred, std::size_t numberOfPrey) {
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
			return !this->CanGrab(pred, prey);
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
		float shiftAmount = fabs((predWidth / 2.0) / tan(GRAB_ANGLE/2.0));

		NiPoint3 coneStart = predPos - predDir * shiftAmount;
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predWidth, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= predWidth*0.4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= cos(GRAB_ANGLE*PI/180.0);
		}), preys.end());

		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}

		return preys;
	}

	bool GrabAnimationController::CanGrab(Actor* pred, Actor* prey) {
		if (pred == prey) {
			return false;
		}
		if (prey->IsDead()) {
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

		float sizedifference = pred_scale/prey_scale;

		float MINIMUM_GRAB_SCALE = MINIMUM_GRAB_SCALE_RATIO;
		float MINIMUM_DISTANCE = MINIMUM_GRAB_DISTANCE;

        if (HasSMT(pred)) {
            MINIMUM_GRAB_SCALE = 0.8; 
			MINIMUM_DISTANCE *= 2.5;
        }

		float balancemode = SizeManager::GetSingleton().BalancedMode();

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if (pred->formID == 0x14 && prey_distance <= MINIMUM_DISTANCE * pred_scale && pred_scale/prey_scale < MINIMUM_GRAB_SCALE) {
			Notify("{} is too big to be grabbed.", prey->GetDisplayFullName());
			return false;
		}
		if (prey_distance <= (MINIMUM_DISTANCE * pred_scale) && pred_scale/prey_scale > MINIMUM_GRAB_SCALE) {
			if ((prey->formID != 0x14 && prey->IsEssential() && Runtime::GetBool("ProtectEssentials"))) {
				return false;
			} else {
				return true;
			}
		} else {
			return false;
		}
	}

	void GrabAnimationController::StartGrab(Actor* pred, Actor* prey) {
		auto& grabbing = GrabAnimationController::GetSingleton();
		if (!grabbing.CanGrab(pred, prey)) {
			return;
		}
		auto spine = find_node(prey, "NPC Spine2 [Spn2]");
		if (spine) {
			SpawnParticle(prey, 3.0, "GTS/Effects/Rune.nif", spine->world.rotate, spine->world.translate, get_visual_scale(prey) * 2.5, 7, spine);
		}
        if (HasSMT(pred)) {
			float expected = 8.0f;
			float predscale = get_target_scale(pred);
			float preyscale = get_target_scale(prey);
            float sizedifference = predscale/preyscale;
			float difference = std::clamp(predscale/expected, preyscale/expected, 0.96f);
			float shrink = preyscale - difference;
			if (sizedifference < expected) {
            	mod_target_scale(prey, -shrink);
				log::info("Shrink: {}, sizediference: {}", shrink, sizedifference);
			}
        }
		Grab::GetSingleton().GrabActor(pred, prey);
		AnimationManager::StartAnim("GrabSomeone", pred);
	}
}