#include "managers/animation/Controllers/HugController.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ThighSandwich.hpp"
#include "managers/animation/HugShrink.hpp"
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

	const float MINIMUM_HUG_DISTANCE = 95.0;
	const float MINIMUM_HUG_SCALE_RATIO = 0.9;
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
	HugAnimationController& HugAnimationController::GetSingleton() noexcept {
		static HugAnimationController instance;
		return instance;
	}

	std::string HugAnimationController::DebugName() {
		return "HugAnimationController";
	}

	std::vector<Actor*> HugAnimationController::GetHugTargetsInFront(Actor* pred, std::size_t numberOfPrey) {
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
			return !this->CanHug(pred, prey);
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

	bool HugAnimationController::CanHug(Actor* pred, Actor* prey) {
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
		if (HasSMT(pred)) {
			pred_scale += 0.25;
		}

		float sizedifference = pred_scale/prey_scale;

		float MINIMUM_HUG_SCALE = MINIMUM_HUG_SCALE_RATIO;
		float MINIMUM_DISTANCE = MINIMUM_HUG_DISTANCE;

		float balancemode = SizeManager::GetSingleton().BalancedMode();

		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if (pred->formID == 0x14 && prey_distance <= MINIMUM_DISTANCE * pred_scale && pred_scale/prey_scale < MINIMUM_HUG_SCALE) {
			Notify("{} is too big to be hugged.", prey->GetDisplayFullName());
			return false;
		}
		if (prey_distance <= (MINIMUM_DISTANCE * pred_scale) && pred_scale/prey_scale >= MINIMUM_HUG_SCALE) {
			if ((prey->formID != 0x14 && prey->IsEssential() && Runtime::GetBool("ProtectEssentials"))) {
				return false;
			} 
			if (!IsHuman(prey)) { // Allow hugs with humanoids only
				if (pred->formID == 0x14) {
					std::string_view message = std::format("You have no desire to hug {}", prey->GetDisplayFullName());
					TiredSound(pred, message); // Just no. We don't have Creature Anims.
					return false;
				} 
				return false;
			} 
			return true;
		} else {
			return false;
		}
	}

	void HugAnimationController::StartHug(Actor* pred, Actor* prey) {
		auto& hugging = HugAnimationController::GetSingleton();
		if (!hugging.CanHug(pred, prey)) {
			return;
		}
		static Timer HugTimer = Timer(12.0);
		if (!HugTimer.ShouldRunFrame()) {
			TiredSound(pred, "Hugs are on the cooldown");
			return;
		}
		HugShrink::GetSingleton().HugActor(pred, prey);
		log::info("Pred {} is Attacking: {}", pred->IsAttacking());
		AnimationManager::StartAnim("Huggies_Try", pred);
		AnimationManager::StartAnim("Huggies_Try_Victim", prey);
	}
}