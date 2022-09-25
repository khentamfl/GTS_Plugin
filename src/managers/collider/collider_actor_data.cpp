#include "managers/collider.hpp"
#include "scale/scale.hpp"
#include "util.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

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
		bool charControllerChanged = this->lastCharController != actor->GetCharController();
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

		auto charController = actor->GetCharController();
		if (charControllerChanged && charController != nullptr) {
			log::info("Actor: {}: Reset center to: {} from: {}", actor->GetDisplayFullName(), charController->center, this->charControllerCenter);
			this->charControllerCenter = charController->center;
		}

		const float EPSILON = 1e-3;

		float visual_scale = get_visual_scale(actor);
		float natural_scale = get_natural_scale(actor);
		float scale_factor = visual_scale/natural_scale;

		if ((fabs(this->last_scale - scale_factor) <= EPSILON) &&  !force_reset) {
			return;
		}

		this->last_scale = scale_factor;
		hkVector4 vecScale = hkVector4(scale_factor, scale_factor, scale_factor, scale_factor);

		// Prune any colliders that are not used anymore
		this->PruneColliders(actor);

		this->ApplyScale(scale_factor, vecScale);

		if (charController) {
			log::info("Actor: {}", actor->GetDisplayFullName());
			log::info("Scale: {}", scale_factor);
			log::info("Up: {}", Vector2Str(charController->up));
			log::info("Unscaled center: {}", this->charControllerCenter);

			hkVector4 pos;
			charController->GetPositionImpl(pos, false);
			log::info("Current Pos: {}", Vector2Str(pos));

			hkVector4 posWwCenter;
			charController->GetPositionImpl(posWwCenter, true);
			log::info("Current Pos + Center: {}", Vector2Str(posWwCenter));

			hkVector4 delta = posWwCenter - pos;
			log::info("Current Delta: {}", Vector2Str(delta));


			charController->center = this->charControllerCenter * scale_factor;

			hkVector4 newPos = pos + charController->up * this->charControllerCenter * scale_factor;
			log::info("Calc Pos: {}", Vector2Str(newPos));

			charController->SetPositionImpl(newPos, true, false);

			charController->GetPositionImpl(pos, false);
			log::info("New Pos: {}", Vector2Str(pos));

			charController->GetPositionImpl(posWwCenter, true);
			log::info("New Pos + Center: {}", Vector2Str(posWwCenter));

			delta = posWwCenter - pos;
			log::info("New Delta: {}", Vector2Str(delta));
		}
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
					for (auto body: hkpObject->bodies) {
						auto const_shape = body->GetShape();
						if (const_shape) {
							hkpShape* shape = const_cast<hkpShape*>(const_shape);
							this->AddShape(shape);
						}
					}

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
