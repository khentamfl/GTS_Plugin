#include "managers/collider.hpp"
#include "scale/scale.hpp"
#include "util.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;
using namespace REL;

namespace {
	const std::uint32_t CAPSULE_MARKER = 494148758080886;
	hkpCapsuleShape* MakeCapsule() {
		hkpCapsuleShape* x = new hkpCapsuleShape();
		// First value is the vtable pointer
		REL::Relocation<std::uintptr_t> vptr(reinterpret_cast<std::uintptr_t>(x));

		// This is the skyrim vtable location
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};

		// Make it use skyrims vtable not our hacky one
		safe_write(vptr.address(), vtable.address());

		x->vertexA = hkVector4(0.0);
		x->vertexB = hkVector4(0.0);
		x->radius = 0.0;
		x->userData = nullptr;
		x->type = hkpShapeType::kCapsule;
		x->pad28 = CAPSULE_MARKER;
		x->pad2C = 0;
		x->pad1C = 0;
		x->memSizeAndFlags = 0xffff; // 0xffff Indicates normal size for type; If 0x0 disable ref counting

		return x;
	}

	void scale_relative_byref(hkVector4& input, const hkVector4& origin, const float& scale) {
		input = (input - origin)*scale + origin;
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
			for (auto rigidBody: this->rb_data) {
				if (rigidBody) {
					// Translation
					scale_relative_byref(rigidBody->motion.motionState.transform.translation, origin, new_scale);

					// COM 0
					scale_relative_byref(rigidBody->motion.motionState.sweptTransform.centerOfMass0, origin, new_scale);

					// COM 1
					scale_relative_byref(rigidBody->motion.motionState.sweptTransform.centerOfMass1, origin, new_scale);
				}
			}
		}
	}

	void ColliderActorData::Update(Actor* actor, std::uint64_t last_reset_frame) {
		bool force_reset = this->last_update_frame.exchange(last_reset_frame) < last_reset_frame;
		if (force_reset ||
		    (
			    (this->capsule_data.size() == 0) &&
			    (this->convex_data.size() == 0) &&
			    (this->list_data.size() == 0) &&
			    (this->rb_data.size() == 0)
		    )) {
			this->UpdateColliders(actor);
		}

		const float EPSILON = 1e-3;
		if (!actor) {
			return;
		}
		float visual_scale = get_visual_scale(actor);
		float natural_scale = get_natural_scale(actor);
		float scale_factor = visual_scale/natural_scale;

		if ((fabs(this->last_scale - scale_factor) <= EPSILON) &&  !force_reset) {
			return;
		}

		this->last_scale = scale_factor;
		hkVector4 vecScale = hkVector4(scale_factor, scale_factor, scale_factor, scale_factor);
		// this->ApplyScale(scale_factor, vecScale);
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
							// this->AddRagdoll(ragdoll);
						}
					}
				}
			}
		}

		// Search CharControllers
		auto charController = actor->GetCharController();
		// this->AddCharController(charController);
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
								log::info("Made new capsule: {}", reinterpret_cast<std::uintptr_t>(newCap));
								newCap->radius = orig_capsule->radius;
								newCap->vertexA = orig_capsule->vertexA;
								newCap->vertexB = orig_capsule->vertexB;
								newCap->userData = orig_capsule->userData;
								newCap->type = orig_capsule->type;
								newCap->memSizeAndFlags = orig_capsule->memSizeAndFlags;

								// Make rb use the new one
								rb->SetShape(newCap);
								mut_cap = newCap;
							}
							log::info("Adding capsule {}", reinterpret_cast<std::uintptr_t>(mut_cap));
							log::info("{},{},{}", mut_cap->vertexA.quad.m128_f32[0], mut_cap->vertexA.quad.m128_f32[1], mut_cap->vertexA.quad.m128_f32[2]);
							log::info("maybe this: {}", mut_cap->referenceCount);
							this->AddCapsule(mut_cap);
							log::info("Capsule added");
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
					// 	log::info("Body");
					// 	auto const_shape = body->GetShape();
					// 	if (const_shape) {
					// 		hkpShape* shape = const_cast<hkpShape*>(const_shape);
					// 		this->AddShape(shape);
					// 	}
					// }
					// for (auto phantom: hkpObject->phantoms) {
					// 	log::info("Phantom");
					// 	auto const_shape = phantom->GetShape();
					// 	if (const_shape) {
					// 		hkpShape* shape = const_cast<hkpShape*>(const_shape);
					// 		this->AddShape(shape);
					// 	}
					// }

					// This is the actual shape
					if (hkpObject->shapePhantom) {
						log::info("shapePhantom");
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
						log::info("m_character");
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
		switch (shape->type) {
			case hkpShapeType::kCapsule: {
				auto actual_shape = static_cast<hkpCapsuleShape*>(shape);
				if (actual_shape) {
					this->AddCapsule(actual_shape);
				}
				break;
			}
			case hkpShapeType::kConvexVertices: {
				auto actual_shape = static_cast<hkpConvexVerticesShape*>(shape);
				if (actual_shape) {
					this->AddConvexVerts(actual_shape);
				}
				break;
			}
			case hkpShapeType::kList: {
				const hkpListShape* container = static_cast<const hkpListShape*>(shape);
				if (container) {
					log::info("Got ListShape: {}", reinterpret_cast<std::uintptr_t>(container));
					auto num_shapes = container->GetNumChildShapes();
					log::info("  - num_shapes: {}", num_shapes);
					hkpShapeKey key = container->GetFirstKey();
					log::info("  - First Key: {}", key);
					while (key != HK_INVALID_SHAPE_KEY) {
						auto buffer = hkpShapeBuffer();
						auto child_shape = container->GetChildShape(key, buffer);
						if (child_shape) {
							hkpShape* child_shape_mut = const_cast<hkpShape*>(child_shape);
							if (child_shape_mut) {
								log::info("Adding child");
								this->AddShape(child_shape_mut);
							}
						}
						key = container->GetNextKey(key);
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
		this->rb_data.insert(key);
	}
}
