#include "managers/vore.hpp"
#include "scale/scale.hpp"
#include "util.hpp"

using namespace RE;
using namespace Gts;

namespace {
	const float MINIMUM_VORE_DISTANCE = 128.0;
	const float MINIMUM_VORE_SCALE_RATIO = 4.0;
}

namespace Gts {
	Vore& Vore::GetSingleton() noexcept {
		static Vore instance;
		return instance;
	}

	void Vore::Update() {
	}

	Actor* Vore::GetPlayerVoreTarget() {
		// Get vore target for player
		auto player = PlayerCharacter::GetSingleton();
		if (!player) {
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
			if ((d < nearest_distance) && this->CanVore(player, actor)) {
				nearest_distance = d;
				closestActor = actor;
			}
		}

		return closestActor;
	}

	Actor* Vore::GetVoreTarget(Actor* pred) {
		// Get vore target for actor
		if (!pred) {
			return nullptr;
		}
		auto charController = pred->GetCharController();
		if (!charController) {
			return nullptr;
		}

		NiPoint3 worldForward = charController->forwardVec * -1;
		NiPoint3 actorPos = pred->GetPosition();

		glm::vec3 start = actorPos + worldForward * 0.0;
		glm::vec3 end = rayStart + worldForward * 1.0;

		Actor* closestActor = nullptr;
		float nearest_distance = 1e8;
		for (auto actor: find_actors()) {
			NiPoint3 actorPos = actor->GetPosition();
			// https://mathworld.wolfram.com/Point-LineDistance3-Dimensional.html
			float d1 = (end - start).cross(start - actorPos).Length() / (end - start).Length();
			float d2 = (actorPos - start).cross(actorPos - end).Length() / (end - start).Length();
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

	bool Vore::CanVore(Actor* pred, Actor* prey) {
		if (pred == prey) {
			return false;
		}
		auto& runtime = Runtime::GetSingleton();

		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);
		float prey_distance = (pred.GetPosition() - prey.GetPosition()).Length();
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
		pred->NotifyAnimationGraph("IdleActivatePickupLow");
		pred->GetMagicCaster(RE::MagicSystem::CastingSource::kInstant)->CastSpellImmediate(runtime.StartVore, false, prey, 1.00f, false, 0.0f, pred);
		log::info("{} was eaten by {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
	}
}
