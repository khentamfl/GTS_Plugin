#include "managers/collider/collider_actor_data.hpp"
#include "scale/scale.hpp"
#include "hooks/RE.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace {
	float GetHeightofCharController(bhkCharacterController* charController) {
		if (!charController) {
			return 0.0;
		}

		RE::hkTransform shapeTransform;
		// use identity matrix for the BB of the unrotated object
		shapeTransform.rotation.col0 = { 1.0f, 0.0f, 0.0f, 0.0f };
		shapeTransform.rotation.col1 = { 0.0f, 1.0f, 0.0f, 0.0f };
		shapeTransform.rotation.col2 = { 0.0f, 0.0f, 1.0f, 0.0f };
		shapeTransform.translation.quad = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);

		float height = 0.0;
		for (auto bShapeRef: charController->shapes) {
			bhkShape* bShape = bShapeRef.get();
			if (bShape) {
				hkReferencedObject* refShape = bShape->referencedObject.get();
				if (refShape) {
					hkpShape* shape = static_cast<hkpShape*>(refShape);
					if (shape) {
						hkAabb outAabb;
						shape->GetAabbImpl(shapeTransform, 0.0, outAabb);
						float shape_height = (outAabb.max - outAabb.min).quad.m128_f32[2];
						height = std::max(shape_height, height);
					}
				}
			}
		}

		return height;
	}

	hkaRagdollInstance* GetRagdoll(Actor* actor) {
		BSAnimationGraphManagerPtr animGraphManager;
		if (actor->GetAnimationGraphManager(animGraphManager)) {
			for (auto& graph : animGraphManager->graphs) {
				if (graph) {
					auto& character = graph->characterInstance;
					auto ragdollDriver = character.ragdollDriver.get();
					if (ragdollDriver) {
						auto ragdoll = ragdollDriver->ragdoll;
						if (ragdoll) {
							return ragdoll;
						}
					}
				}
			}
		}
		return nullptr;
	}
}

namespace Gts {
	ColliderActorData::ColliderActorData(Actor* actor) {
	}

	ColliderActorData::~ColliderActorData() {
	}

	void ColliderActorData::FlagUpdate() {
		this->last_update_frame.store(0);
	}

	void ColliderActorData::Reset() {
		this->last_update_frame.store(0);
	}

	void ColliderActorData::ApplyScale(const float& new_scale, const hkVector4& vec_scale) {
		this->ragdollData.ApplyScale(new_scale, vec_scale);
		this->charContData.ApplyScale(new_scale, vec_scale);
	}

	void ColliderActorData::ApplyPose(Actor* actor, const float& new_scale) {
		auto model = actor->GetCurrent3D();
		if (model) {
			hkVector4 origin = hkVector4(model->world.translate * (*g_worldScale));
			this->ragdollData.ApplyPose(origin, new_scale);
		}
	}

	void ColliderActorData::Update(Actor* actor, std::uint64_t last_reset_frame) {
		auto charController = actor->GetCharController();
		auto ragdoll = GetRagdoll(actor);

		bool needs_reset = this->last_update_frame.exchange(last_reset_frame) < last_reset_frame;
		bool charControllerChanged = this->charContData.charCont != charController;
		bool ragdollChanged = this->ragdollData.ragdoll != ragdoll;
		if (needs_reset || charControllerChanged || ragdollChanged ) {
			this->UpdateColliders(actor);
		}

		if (actor->formID == 0x14) {
			log::info("== Frame ==");
			log::info("  Center: {}", charController->center);
			log::info("  pushDelta: {}", Vector2Str(charController->pushDelta));
			log::info("  pitchAngle: {}", Vector2Str(charController->pitchAngle));
			log::info("  rollAngle: {}", Vector2Str(charController->rollAngle));
			log::info("  pitchMult: {}", Vector2Str(charController->pitchMult));
			log::info("  actorHeight: {}", Vector2Str(charController->actorHeight));
			log::info("  speedPct: {}", Vector2Str(charController->speedPct));
			log::info("");
		}

		const float EPSILON = 1e-3;

		float visual_scale = get_visual_scale(actor);
		float natural_scale = get_natural_scale(actor);
		float scale_factor = visual_scale/natural_scale;

		if ((fabs(this->last_scale - scale_factor) <= EPSILON) &&  !needs_reset) {
			return;
		}

		hkVector4 vecScale = hkVector4(scale_factor, scale_factor, scale_factor, scale_factor);

		// Prune any colliders that are not used anymore
		this->PruneColliders(actor);

		this->ApplyScale(scale_factor, vecScale);

		this->last_scale = scale_factor;
	}

	void ColliderActorData::UpdateColliders(Actor* actor) {
		// Search Ragdoll
		auto ragdoll = GetRagdoll(actor);
		this->AddRagdoll(ragdoll);

		// Search CharControllers
		auto charController = actor->GetCharController();
		this->AddCharController(charController);
	}

	void ColliderActorData::PruneColliders(Actor* actor) {
		this->ragdollData.PruneColliders(actor);
		this->charContData.PruneColliders(actor);
	}

	void ColliderActorData::AddRagdoll(hkaRagdollInstance* ragdoll) {
		this->ragdollData.UpdateColliders(ragdoll);
	}

	void ColliderActorData::AddCharController(bhkCharacterController* charController) {
		this->charContData.UpdateColliders(charController);
	}

}
