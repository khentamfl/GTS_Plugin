#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "util.hpp"

using namespace RE;
using namespace Gts;

namespace {
	const float MINIMUM_VORE_DISTANCE = 256.0;
	const float MINIMUM_VORE_SCALE_RATIO = 8.0;
}

namespace Gts {
	Vore& Vore::GetSingleton() noexcept {
		static Vore instance;
		return instance;
	}

	void Vore::Update() {
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
			return fabs(cosTheta) <= 0.866025403784; // <= cos(30*pi/180) === +-30 degrees
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
		auto PC = PlayerCharacter::GetSingleton();

		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);
		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();
		if ((prey_distance < MINIMUM_VORE_DISTANCE * pred_scale)
		    && PC->HasPerk(runtime.VorePerk);
		    && (pred_scale/prey_scale > MINIMUM_VORE_SCALE_RATIO)
		    && (!prey->IsEssential())
		    && !pred->HasSpell(runtime.StartVore)) {
			return true;
		} else {
			pred->NotifyAnimationGraph("IdleActivatePickupLow"); // Only play anim if we can't eat the target
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
