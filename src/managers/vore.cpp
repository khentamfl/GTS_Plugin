#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "util.hpp"
#include <cmath>

using namespace RE;
using namespace Gts;

namespace {
	const float MINIMUM_VORE_DISTANCE = 128.0;
	const float MINIMUM_VORE_SCALE_RATIO = 8.0;
	const float VORE_ANGLE = 100;
	const float PI = 3.14159;
}

namespace Gts {
	Vore& Vore::GetSingleton() noexcept {
		static Vore instance;
		return instance;
	}

	void Vore::Update() {
		if (!pred->HasPerk(runtime.VorePerk)) {
			return;
		}
		static Timer timer = Timer(6.00);
		if (timer.ShouldRunFrame()) { //Try to not overload
			for (auto actor: find_actors()) {
				if (actor->IsInFaction(runtime.FollowerFaction) || actor->IsPlayerTeammate() && actor->IsInCombat() && PC->HasPerk(runtime.VorePerk)) {
					RandomVoreAttempt(actor);
				}
			}
		}
	}
}

void Vore::RandomVoreAttempt(Actor* caster) {
	Actor* player = PlayerCharacter::GetSingleton();
	auto& runtime = Runtime::GetSingleton();
	auto VoreManager = Vore::GetSingleton();
	int Requirement = (25 * Gigantism) * SizeManager::GetSingleton().BalancedMode();
	std::size_t numberOfPrey = 1;
	if (player->HasPerk(runtime.MassVorePerk)) {
		numberOfPrey = 3;
	}

	for (auto actor: find_actors()) {
		if (actor->formID == 0x14 || !actor->Is3DLoaded() || actor->IsDead()) {
			return;
		}
		float Gigantism = 1.0 - SizeManager::GetSingleton().GetEnchantmentBonus(caster)/100;

		int random = rand() % Requirement;
		int decide_chance = 1;
		if (random <= decide_chance) {
			Actor* pred = caster;
			std::vector<Actor*> preys = VoreManager.GetVoreTargetsInFront(pred, numberOfPrey);
			for (auto prey: preys) {
				VoreManager.StartVore(pred, prey);
			}
		}
	}

	Actor* Vore::GeVoreTargetCrossHair(Actor* pred) {
		// Get vore target for player
		if (!pred) {
			return nullptr;
		}
		auto playerCamera = PlayerCamera::GetSingleton();
		if (!playerCamera) {
			return nullptr;
		}
		auto crosshairPick = RE::CrosshairPickData::GetSingleton();
		if (!crosshairPick) {
			return nullptr;
		}
		auto cameraNode = playerCamera->cameraRoot.get();
		if (!cameraNode) {
			return nullptr;
		}
		NiPoint3 start = cameraNode->world.translate;
		NiPoint3 end = crosshairPick->collisionPoint;

		Actor* closestActor = nullptr;
		float nearest_distance = 1e8;
		for (auto actor: find_actors()) {
			NiPoint3 actorPos = actor->GetPosition();
			// https://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
			float d1 = (end - start).Cross(start - actorPos).Length() / (end - start).Length();
			float d2 = (actorPos - start).Cross(actorPos - end).Length() / (end - start).Length();
			float d;
			if ((d1 > 1e-4) && (d2 > 1e-4)) {
				d = std::min(d1, d2);
			} else if (d1 > 1e-4) {
				d = d1;
			} else if (d2 > 1e-4) {
				d = d2;
			} else {
				continue;
			}
			if ((d < nearest_distance) && this->CanVore(pred, actor)) {
				nearest_distance = d;
				closestActor = actor;
			}
		}

		return closestActor;
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
		hkVector4 forwardVechK = charController->forwardVec;
		NiPoint3 forwardVecNi = NiPoint3(
			forwardVechK.quad.m128_f32[0],
			forwardVechK.quad.m128_f32[1],
			forwardVechK.quad.m128_f32[2]
			);
		NiPoint3 worldForward = forwardVecNi * -1;
		NiPoint3 predDir = worldForward - predPos;
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
		//   | pred |  <- Based on width of prey
		//   |______|
		float predWidth = 70 * get_visual_scale(pred);
		float shiftAmount = tan(VORE_ANGLE/2.0) * predWidth / 2.0;

		NiPoint3 coneStart = predPos - forwardVecNi * shiftAmount;
		predDir = predPos - coneStart;
		predDir = predDir / predDir.Length();
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predWidth, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= predWidth) {
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
		auto& runtime = Runtime::GetSingleton();

		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);
		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if ((prey_distance < MINIMUM_VORE_DISTANCE * pred_scale)
		    && (pred_scale/prey_scale > MINIMUM_VORE_SCALE_RATIO)
		    && (!prey->IsEssential())
		    && !pred->HasSpell(runtime.StartVore)) {
			return true;
		} else {
			return false;
		}
	}

	void Vore::StartVore(Actor* pred, Actor* prey) {
		auto runtime = Runtime::GetSingleton();
		if (!CanVore(pred, prey)) {
			return;
		}
		pred->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.StartVore, false, prey, 1.00f, false, 0.0f, pred);
		ConsoleLog::GetSingleton()->Print("%s Was Eaten by %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
	}
}
