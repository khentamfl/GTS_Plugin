#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/VoreHandler.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"
#include <cmath>

using namespace RE;
using namespace Gts;

namespace {
	const float MINIMUM_VORE_DISTANCE = 82.0;
	const float MINIMUM_VORE_SCALE_RATIO = 4.8;
	const float VORE_ANGLE = 76;
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

	void ToggleFreeCamera() {
		auto playerCamera = PlayerCamera::GetSingleton();
		playerCamera->ToggleFreeCameraMode(false);
	}

  void VoreInputEvent(const InputEventData& data) {
    static Timer voreTimer = Timer(5.0);
    auto pred = PlayerCharacter::GetSingleton();

    if (voreTimer.ShouldRunFrame()) {
      auto& VoreManager = Vore::GetSingleton();
      std::size_t numberOfPrey = 1;
      if (Runtime::HasPerk(pred, "MassVorePerk")) {
        numberOfPrey = 3;
      }
      std::vector<Actor*> preys = VoreManager.GetVoreTargetsInFront(pred, numberOfPrey);
      for (auto prey: preys) {
        VoreManager.StartVore(pred, prey);
      }
    }
  }
}

namespace Gts {
	Vore& Vore::GetSingleton() noexcept {
		static Vore instance;
		return instance;
	}

	std::string Vore::DebugName() {
		return "Vore";
	}

  void Vore::DataReady() {
    InputManager::RegisterInputEvent("Vore", VoreInputEvent);
  }

	void Vore::Update() {
		auto player = PlayerCharacter::GetSingleton();

		if (!Runtime::HasPerk(player, "VorePerk")) {
			return;
		}
		static Timer timer = Timer(3.00); // Random Vore once per 3 sec
		if (timer.ShouldRunFrame()) { //Try to not overload
			for (auto actor: find_actors()) {
				if ((Runtime::InFaction(actor,"FollowerFaction") || actor->IsPlayerTeammate()) && actor->IsInCombat()) {
					RandomVoreAttempt(actor);
					//log::info("Found Vore Caster");
				}
			}
		}
	}

	void Vore::RandomVoreAttempt(Actor* caster) {
		Actor* player = PlayerCharacter::GetSingleton();
		auto& VoreManager = Vore::GetSingleton();
		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerk(player, "MassVorePerk")) {
			numberOfPrey = 3;
		}
		//log::info("Attempting Random Vore");
		for (auto actor: find_actors()) {
			if (actor->formID == 0x14 || !actor->Is3DLoaded() || actor->IsDead()) {
				return;
			}
			float Gigantism = 1.0 / (1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(caster)/100);
			int Requirement = (10 * Gigantism) * SizeManager::GetSingleton().BalancedMode();

			int random = rand() % Requirement;
			int decide_chance = 2;
			if (random <= decide_chance) {
				Actor* pred = caster;
				//log::info("random Vore for {} is true", caster->GetDisplayFullName());
				//log::info("{} is looking for prey", caster->GetDisplayFullName());
				std::vector<Actor*> preys = VoreManager.GetVoreTargetsInFront(pred, numberOfPrey);
				for (auto prey: preys) {
					if (prey->formID == 0x14) {
						return;
					}
					VoreManager.StartVore(pred, prey);
				}
			}
		}
	}

	Actor* Vore::GeVoreTargetCrossHair(Actor* pred) {
		auto preys = this->GeVoreTargetsCrossHair(pred, 1);
		if (preys.size() > 0) {
			return preys[0];
		} else {
			return nullptr;
		}
	}

	std::vector<Actor*> Vore::GeVoreTargetsCrossHair(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for player
		if (!pred) {
			return {};
		}
		auto playerCamera = PlayerCamera::GetSingleton();
		if (!playerCamera) {
			return {};
		}
		auto crosshairPick = RE::CrosshairPickData::GetSingleton();
		if (!crosshairPick) {
			return {};
		}
		auto cameraNode = playerCamera->cameraRoot.get();
		if (!cameraNode) {
			return {};
		}
		NiPoint3 start = cameraNode->world.translate;
		NiPoint3 end = crosshairPick->collisionPoint;

		auto preys = find_actors();
		auto predPos = pred->GetPosition();

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
			return !this->CanVore(pred, prey);
		}), preys.end());;

		// Filter out actors not in front
		NiPoint3 predDir = end - start;
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

		NiPoint3 coneStart = start;
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= 1e-4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= cos(VORE_ANGLE*PI/180.0);
		}), preys.end());

		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}

		return preys;
	}

	Actor* Vore::GetVoreTargetInFront(Actor* pred) {
		auto preys = this->GetVoreTargetsInFront(pred, 1);
		if (preys.size() > 0) {
			return preys[0];
		} else {
			return nullptr;
		}
	}

	std::vector<Actor*> Vore::GetVoreTargetsInFront(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for actor
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
			return !this->CanVore(pred, prey);
		}), preys.end());;

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
		float shiftAmount = fabs((predWidth / 2.0) / tan(VORE_ANGLE/2.0));

		NiPoint3 coneStart = predPos - predDir * shiftAmount;
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predWidth, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= predWidth*0.4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= cos(VORE_ANGLE*PI/180.0);
		}), preys.end());

		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}

		return preys;
	}

	Actor* Vore::GetVoreTargetAround(Actor* pred) {
		auto preys = this->GetVoreTargetsAround(pred, 1);
		if (preys.size() > 0) {
			return preys[0];
		} else {
			return nullptr;
		}
	}

	std::vector<Actor*> Vore::GetVoreTargetsAround(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for actor
		// around them
		if (!pred) {
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
			return !this->CanVore(pred, prey);
		}), preys.end());

		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}

		return preys;
	}


	bool Vore::CanVore(Actor* pred, Actor* prey) {
		if (pred == prey) {
			return false;
		}

		if (!Runtime::HasPerkTeam(pred, "VorePerk")) {
			return false;
		}

		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);
		if (IsDragon(prey)) {
			prey_scale *= 2.0;
		}

		float sizedifference = pred_scale/prey_scale;

		float MINIMUM_VORE_SCALE = MINIMUM_VORE_SCALE_RATIO;

		float balancemode = SizeManager::GetSingleton().BalancedMode();
		if (Runtime::HasPerk(pred,"MassVorePerk")) {
			sizedifference *= 1.15; // Less stamina drain
			MINIMUM_VORE_SCALE *= 0.85; // Decrease Size Requirement
		}

		if (balancemode == 2.0) { // This is checked only if Balance Mode is enabled. Enables HP requirement on Vore.
			float getmaxhp = GetMaxAV(prey, ActorValue::kHealth);
			float gethp = GetAV(prey, ActorValue::kHealth);
			float healthrequirement = getmaxhp/pred_scale;
			if (pred->formID == 0x14, gethp > healthrequirement) {
				DamageAV(prey, ActorValue::kHealth, 6 * sizedifference);
				DamageAV(pred, ActorValue::kStamina, 26/sizedifference);
				Notify("{} is too healthy to be eaten", prey->GetDisplayFullName());
				return false;
			}
		}



		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if (pred->formID == 0x14 && prey_distance <= (MINIMUM_VORE_DISTANCE * pred_scale) && pred_scale/prey_scale < MINIMUM_VORE_SCALE) {
			Notify("{} is too big to be eaten.", prey->GetDisplayFullName());
		}
		if ((prey_distance <= (MINIMUM_VORE_DISTANCE * pred_scale))
		    && (pred_scale/prey_scale > MINIMUM_VORE_SCALE)
		    && (!prey->IsEssential())
		    && (!Runtime::HasSpell(prey, "StartVore"))) {
			return true;
		} else {

			return false;
		}
	}

	void Vore::StartVore(Actor* pred, Actor* prey) {
		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);

		float sizedifference = pred_scale/prey_scale;

		if (Runtime::HasPerk(pred, "MassVorePerk")) {
			sizedifference *= 1.15; // Less stamina drain
		}

		float wastestamina = 140/sizedifference; // Drain stamina, should be 300 once tests are over
		float staminacheck = pred->AsActorValueOwner()->GetActorValue(ActorValue::kStamina);

		if (pred->formID != 0x14) {
			wastestamina = 40/sizedifference; // Less tamina drain for non Player
		}



		if (!CanVore(pred, prey)) {
			return;
		}
		if (prey->IsEssential() && Runtime::GetBool("ProtectEssentials")) {
			Notify("{} is important, i shouldn't eat {}.", prey->GetDisplayFullName(), prey->GetDisplayFullName());
		}
		if (staminacheck < wastestamina) {
			Notify("{} is too tired for vore.", pred->GetDisplayFullName());
			DamageAV(prey, ActorValue::kHealth, 3 * sizedifference);
			Runtime::PlaySound("VoreSound_Fail", pred, 1.8, 0.0);
			Runtime::CastSpell(pred, prey, "gtsStaggerSpell");
			return;
		}

		DamageAV(pred, ActorValue::kStamina, wastestamina);
		Runtime::PlaySound("VoreSound_Success", pred, 0.6, 0.0);

		if (pred->formID == 0x14) {
			AdjustSizeLimit(0.0260, pred);
		}
		//Runtime::CastSpell(pred, prey, "StartVore");
		VoreHandler::GetSingleton().GrabVoreActor(pred, prey);
		AnimationManager::GetSingleton().StartAnim("StartVore", pred);
	}
}
