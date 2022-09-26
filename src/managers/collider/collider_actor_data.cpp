#include "managers/collider.hpp"
#include "scale/scale.hpp"
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
}

namespace Gts {
	ColliderActorData::ColliderActorData(Actor* actor) {
		this->Reset();
		this->form_id = actor->formID;
	}

	ColliderActorData::~ColliderActorData() {
		this->Reset();
	}

	void ColliderActorData::FlagUpdate() {
		this->last_update_frame.store(0);
	}

	void ColliderActorData::Reset() {
		this->capsule_data.clear();
		this->convex_data.clear();
		this->list_data.clear();
		this->rb_data.clear();

		this->last_scale = -1.0;
		this->last_update_frame.store(0);
		this->form_id = 0;
		this->lastCharController = nullptr;
	}

	void ColliderActorData::ApplyScale(const float& new_scale, const hkVector4& vec_scale) {
		for (auto &[key, data]: this->capsule_data) {
			data.ApplyScale(new_scale, vec_scale);
		}
		for (auto &[key, data]: this->convex_data) {
			data.ApplyScale(new_scale, vec_scale);
		}
		for (auto &[key, data]: this->list_data) {
			data.ApplyScale(new_scale, vec_scale);
		}
	}

	void ColliderActorData::ApplyPose(Actor* actor, const float& new_scale) {
		auto model = actor->GetCurrent3D();
		if (model) {
			hkVector4 origin = hkVector4(model->world.translate * (*g_worldScale));
			for (auto &[key, data]: this->rb_data) {
				data.ApplyPose(origin, new_scale);
			}
		}
	}

	void ColliderActorData::Update(Actor* actor, std::uint64_t last_reset_frame) {
		bool force_reset = this->last_update_frame.exchange(last_reset_frame) < last_reset_frame;
		auto charController = actor->GetCharController();
		if (!charController) {
			return;
		}
		bool charControllerChanged = this->lastCharController != charController;
		if (force_reset ||
		    (
			    (this->capsule_data.size() == 0) &&
			    (this->convex_data.size() == 0) &&
			    (this->list_data.size() == 0) &&
			    (this->rb_data.size() == 0)
		    ) ||
		    charControllerChanged) {
			this->UpdateColliders(actor);
		}


		if (charControllerChanged) {
			log::info("Actor: {}: Reset center to: {} from: {}", actor->GetDisplayFullName(), charController->center, this->charControllerCenter);
			this->charControllerCenter = charController->center;
			this->actorHeight = charController->actorHeight;
		}

		const float EPSILON = 1e-3;

		float visual_scale = get_visual_scale(actor);
		float natural_scale = get_natural_scale(actor);
		float scale_factor = visual_scale/natural_scale;

		if ((fabs(this->last_scale - scale_factor) <= EPSILON) &&  !force_reset) {
			return;
		}

		hkVector4 vecScale = hkVector4(scale_factor, scale_factor, scale_factor, scale_factor);

		// Prune any colliders that are not used anymore
		this->PruneColliders(actor);

		this->ApplyScale(scale_factor, vecScale);

		float preScaleCenter = charController->center;
		// Change the center offset of the collider
		charController->actorHeight = this->actorHeight * scale_factor;
		charController->center = this->charControllerCenter * scale_factor * 2.0;
		float postScaleCenter = charController->center;

		// Adjust position caused by center shift
		hkVector4 pos;
		charController->GetPositionImpl(pos, false);
		hkVector4 postScalePos = pos + hkVector4(0.0, 0.0, 1.0, 0.0) * (postScaleCenter - preScaleCenter);
		charController->SetPositionImpl(postScalePos, false, false);


		this->last_scale = scale_factor;
	}

	void ColliderActorData::UpdateColliders(Actor* actor) {
		// Search Ragdoll
		BSAnimationGraphManagerPtr animGraphManager;
		if (actor->GetAnimationGraphManager(animGraphManager)) {
			for (auto& graph : animGraphManager->graphs) {
				if (graph) {
					auto& character = graph->characterInstance;
					auto ragdollDriver = character.ragdollDriver.get();
					if (ragdollDriver) {
						auto ragdoll = ragdollDriver->ragdoll;
						if (ragdoll) {
							this->AddRagdoll(ragdoll);
						}
					}
				}
			}
		}

		// Search CharControllers
		auto charController = actor->GetCharController();
		this->lastCharController = charController;
		this->AddCharController(charController);
	}

	void ColliderActorData::PruneColliders(Actor* actor) {
		for (auto i = this->capsule_data.begin(); i != this->capsule_data.end();) {
			auto& data = (*i);
			auto key = data.first;
			if (key->GetReferenceCount() == 1) {
				i = this->capsule_data.erase(i);
			} else {
				++i;
			}
		}
		for (auto i = this->convex_data.begin(); i != this->convex_data.end();) {
			auto& data = (*i);
			auto key = data.first;
			if (key->GetReferenceCount() == 1) {
				i = this->convex_data.erase(i);
			} else {
				++i;
			}
		}
		for (auto i = this->list_data.begin(); i != this->list_data.end();) {
			auto& data = (*i);
			auto key = data.first;
			if (key->GetReferenceCount() == 1) {
				i = this->list_data.erase(i);
			} else {
				++i;
			}
		}
		for (auto i = this->rb_data.begin(); i != this->rb_data.end();) {
			auto& data = (*i);
			auto key = data.first;
			if (key->GetReferenceCount() == 1) {
				i = this->rb_data.erase(i);
			} else {
				++i;
			}
		}
	}

	void ColliderActorData::AddRagdoll(hkaRagdollInstance* ragdoll) {
		for (auto& rb: ragdoll->rigidBodies) {
			if (rb) {
				auto shape = rb->GetShape();
				if (shape) {
					if (shape->type == hkpShapeType::kCapsule) {
						const hkpCapsuleShape* orig_capsule = static_cast<const hkpCapsuleShape*>(shape);
						hkpCapsuleShape* mut_cap = const_cast<hkpCapsuleShape*>(orig_capsule);
						// Is the capsule in our data?
						if (mut_cap) {
							if (this->capsule_data.find(mut_cap) == this->capsule_data.end()) {
								// Nope
								// Clone it (because ragdolls share shape between all actors of same race)
								hkpCapsuleShape* newCap = MakeCapsule();
								newCap->radius = orig_capsule->radius;
								newCap->vertexA = orig_capsule->vertexA;
								newCap->vertexB = orig_capsule->vertexB;
								newCap->userData = orig_capsule->userData;
								newCap->type = orig_capsule->type;
								newCap->memSizeAndFlags = orig_capsule->memSizeAndFlags;

								// Make rb use the new one
								rb->SetShape(newCap);
								this->AddCapsule(newCap);
								// newCap->RemoveReference();
							} else {
								this->AddCapsule(mut_cap);
							}

						}
						if (rb) {
							this->AddRidgedBody(rb);
						}
					}
				}
			}
		}
	}

	void ColliderActorData::AddCharController(bhkCharacterController* charController) {
		bhkCharProxyController* charProxyController = skyrim_cast<bhkCharProxyController*>(charController);
		bhkCharRigidBodyController* charRigidBodyController = skyrim_cast<bhkCharRigidBodyController*>(charController);
		if (charProxyController) {
			// Player controller is a proxy one
			auto& proxy = charProxyController->proxy;
			hkReferencedObject* refObject = proxy.referencedObject.get();
			if (refObject) {
				hkpCharacterProxy* hkpObject = skyrim_cast<hkpCharacterProxy*>(refObject);
				if (hkpObject) {
					// I think these ones are objects that we collide with
					// Needs testing
					// for (auto body: hkpObject->bodies) {
					// 	auto const_shape = body->GetShape();
					// 	if (const_shape) {
					// 		hkpShape* shape = const_cast<hkpShape*>(const_shape);
					// 		this->AddShape(shape);
					// 	}
					// }

					// This one appears to be active during combat.
					// Maybe used for sword swing collision detection
					for (auto phantom: hkpObject->phantoms) {
						auto const_shape = phantom->GetShape();
						if (const_shape) {
							hkpShape* shape = const_cast<hkpShape*>(const_shape);
							this->AddShape(shape);
						}
					}

					// This is the actual shape
					if (hkpObject->shapePhantom) {
						auto const_shape = hkpObject->shapePhantom->GetShape();
						if (const_shape) {
							hkpShape* shape = const_cast<hkpShape*>(const_shape);
							this->AddShape(shape);
						}
					}
				}
			}
		} else if (charRigidBodyController) {
			// NPCs seem to use rigid body ones
			auto& characterRigidBody = charRigidBodyController->characterRigidBody;
			hkReferencedObject* refObject = characterRigidBody.referencedObject.get();
			if (refObject) {
				hkpCharacterRigidBody* hkpObject = skyrim_cast<hkpCharacterRigidBody*>(refObject);
				if (hkpObject) {
					if (hkpObject->m_character) {
						auto const_shape = hkpObject->m_character->GetShape();
						if (const_shape) {
							hkpShape* shape = const_cast<hkpShape*>(const_shape);
							this->AddShape(shape);
						}
					}
				}
			}
		}

		if (charController) {
			// There are two collider shapes on the characontroller
			// One of these shapes should already be handelled in the above
			// calcs however there is anohter one attached to the characontroller
			// that we are not sure what it does.
			// Regardless we add it to the collider system now
			for (auto bShapeRef: charController->shapes) {
				bhkShape* bShape = bShapeRef.get();
				if (bShape) {
					hkReferencedObject* refShape = bShape->referencedObject.get();
					if (refShape) {
						hkpShape* shape = static_cast<hkpShape*>(refShape);
						if (shape) {
							this->AddShape(shape);
						}
					}
				}
			}
		}
	}

	void ColliderActorData::AddShape(hkpShape* shape) {
		// Go over the shape and add what we can
		if (!shape) {
			return;
		}
		// log::info("Adding shape: {}", static_cast<std::uint32_t>(shape->type));
		switch (shape->type) {
			case hkpShapeType::kCapsule: {
				// log::info("Adding Capsule");
				auto actual_shape = static_cast<hkpCapsuleShape*>(shape);
				if (actual_shape) {
					this->AddCapsule(actual_shape);
				}
				break;
			}
			case hkpShapeType::kConvexVertices: {
				// log::info("Adding ConvVerts");
				auto actual_shape = static_cast<hkpConvexVerticesShape*>(shape);
				if (actual_shape) {
					this->AddConvexVerts(actual_shape);
				}
				break;
			}
			case hkpShapeType::kList: {
				// log::info("Adding List");
				const hkpListShape* container = static_cast<const hkpListShape*>(shape);
				if (container) {
					for (auto childShapeInfo: container->childInfo) {
						auto child_shape = childShapeInfo.shape;
						if (child_shape) {
							hkpShape* child_shape_mut = const_cast<hkpShape*>(child_shape);
							if (child_shape_mut) {
								this->AddShape(child_shape_mut);
							}
						}
					}

					auto container_mut = const_cast<hkpListShape*>(container);
					if (container_mut) {
						this->AddList(container_mut);
					}
				}
				break;
			}
		}
	}

	void ColliderActorData::AddCapsule(hkpCapsuleShape* orig_capsule) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!orig_capsule) {
			return;
		}
		auto key = orig_capsule;
		this->capsule_data.try_emplace(key, orig_capsule);
	}

	void ColliderActorData::AddConvexVerts(hkpConvexVerticesShape* convex) {
		std::unique_lock lock(this->_lock);
		if (!convex) {
			return;
		}
		auto key = convex;
		this->convex_data.try_emplace(key, convex);
	}

	void ColliderActorData::AddList(hkpListShape* list) {
		std::unique_lock lock(this->_lock);
		if (!list) {
			return;
		}
		auto key = list;
		this->list_data.try_emplace(key, list);
	}

	void ColliderActorData::AddRidgedBody(hkpRigidBody* rigid_body) {
		std::unique_lock lock(this->_lock);
		if (!rigid_body) {
			return;
		}
		auto key = rigid_body;
		this->rb_data.try_emplace(key, rigid_body);
	}
}
