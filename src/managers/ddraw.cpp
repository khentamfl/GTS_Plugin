#include "managers/ddraw.hpp"
#include "UI/DebugAPI.hpp"
#include "hooks/RE.hpp"
#include "node.hpp"
#include "util.hpp"
#include "raycast.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;


namespace {
	const float MS_TIME = 10;

	const glm::vec4 CAPSULE_COLOR = { 0.50f, 0.50f, 0.20f, 1.0f };
	const float CAPSULE_LINETHICKNESS = 0.1;

	const glm::vec4 TRIANGLE_COLOR = { 0.20f, 0.50f, 0.50f, 1.0f };
	const float TRIANGLE_LINETHICKNESS = 0.1;

	const glm::vec4 BOX_COLOR = { 0.80f, 0.20f, 0.50f, 1.0f };
	const float BOX_LINETHICKNESS = 0.1;

	const glm::vec4 UNKNOWN_COLOR = { 0.10f, 0.10f, 0.10f, 1.0f };
	const float UNKNOWN_LINETHICKNESS = 0.1;

	const glm::vec4 RAY_COLOR = { 0.90f, 0.10f, 0.10f, 1.0f };
	const float RAY_LINETHICKNESS = 3.0;

	void DrawShape(const hkpShape* shape, const glm::mat4& transform) {
		if (shape->type == hkpShapeType::kCapsule) {
			// log::info("Capsule");
			const hkpCapsuleShape* capsule = static_cast<const hkpCapsuleShape*>(shape);
			if (capsule) {
				glm::vec3 start = HkToGlm(capsule->vertexA);
				glm::vec3 end = HkToGlm(capsule->vertexB);
				float radius = capsule->radius * *g_worldScaleInverse;

				DebugAPI::DrawCapsule(start, end, radius, transform, MS_TIME, CAPSULE_COLOR, CAPSULE_LINETHICKNESS);
			}
		} else if (shape->type == hkpShapeType::kTriangle) {
			// log::info("Triangle");
			// const hkpTriangleShape* triangle = static_cast<const hkpTriangleShape*>(shape);
			// if (triangle) {
			// 	glm::vec3 pointA = HkToGlm(triangle->vertexA);
			// 	glm::vec3 pointB = HkToGlm(triangle->vertexB);
			// 	glm::vec3 pointC = HkToGlm(triangle->vertexC);
			//
			// 	DebugAPI::DrawTriangle(pointA, pointB, pointB, transform, MS_TIME, TRIANGLE_COLOR, TRIANGLE_LINETHICKNESS);
			// }
		} else if (shape->type == hkpShapeType::kBox) {
			// log::info("Triangle");
			const hkpBoxShape* box = static_cast<const hkpBoxShape*>(shape);
			if (box) {
				glm::vec3 origin = glm::vec3(0.,0.,0.);
				glm::vec3 halfExtents = HkToGlm(box->halfExtents);

				DebugAPI::DrawBox(origin, halfExtents, transform, MS_TIME, TRIANGLE_COLOR, TRIANGLE_LINETHICKNESS);
			}
		} else if (shape->type == hkpShapeType::kConvexVertices) {
			// Too much effort to RE and draw that
			// const hkpConvexShape* unknown = static_cast<const hkpConvexShape*>(shape);
			// if (unknown) {
			// 	glm::vec3 localPos = glm::vec3(0.,0.,0.);
			// 	glm::vec3 worldPos = (transform * glm::vec4(localPos, 1.0));
			// 	float radius = radius * (*Gts::g_worldScaleInverse);
			// 	DebugAPI::DrawSphere(worldPos, radius, MS_TIME, UNKNOWN_COLOR, UNKNOWN_LINETHICKNESS);
			// }
		} else if (shape->type == hkpShapeType::kList) {
			// log::info("List");
			auto container = static_cast<const hkpListShape*>(shape);
			auto key = container->GetFirstKey();
			while (key != HK_INVALID_SHAPE_KEY) {
				auto buffer = hkpShapeBuffer();
				auto child_shape = container->GetChildShape(key, buffer);
				if (child_shape) {
					// log::info("  - Child");
					DrawShape(child_shape, transform);
				}
				key = container->GetNextKey(key);
			}
		} else if (shape->type == hkpShapeType::kBVTree) {
			// log::info("Tree");
			auto actual_shape = static_cast<const hkpBvTreeShape*>(shape);
			const hkpShapeContainer* container = actual_shape->GetContainer();
			auto key = container->GetFirstKey();
			while (key != HK_INVALID_SHAPE_KEY) {
				auto buffer = hkpShapeBuffer();
				auto child_shape = container->GetChildShape(key, buffer);
				if (child_shape) {
					// log::info("  - Child");
					DrawShape(child_shape, transform);
				}
				key = container->GetNextKey(key);
			}
		} else if (shape->type == hkpShapeType::kMOPP) {
			// log::info("MOP Tree");
			// MOPP is a Tree implementation we jsut cast too tree and deal with
			// it at that level
			auto actual_shape = static_cast<const hkpBvTreeShape*>(shape);
			const hkpShapeContainer* container = actual_shape->GetContainer();
			auto key = container->GetFirstKey();
			while (key != HK_INVALID_SHAPE_KEY) {
				auto buffer = hkpShapeBuffer();
				auto child_shape = container->GetChildShape(key, buffer);
				if (child_shape) {
					// log::info("  - Child");
					DrawShape(child_shape, transform);
				}
				key = container->GetNextKey(key);
			}
		} else {
			log::debug("- Shape (of type {}) is not handlled", static_cast<int>(shape->type));
		}
	}

	void DrawWorldObject(hkpWorldObject* worldObject) {
		auto shape = worldObject->GetShape();
		auto motionState = worldObject->GetMotionState();
		if (motionState) {
			glm::mat4 transform = HkToGlm(motionState->transform);
			if (shape) {
				DrawShape(shape, transform);
			}
		}
	}
	void DrawRigidBody(hkpRigidBody* rigidBody) {
		auto shape = rigidBody->GetShape();
		glm::mat4 transform = HkToGlm(rigidBody->motion.motionState.transform);
		if (shape) {
			DrawShape(shape, transform);
		}
	}

	void DrawNiAvObject(NiAVObject* currentnode) {
		auto collisionObject = currentnode->GetCollisionObject();
		if (collisionObject) {
			auto bhkRigidBody = collisionObject->GetRigidBody();
			if (bhkRigidBody) {
				hkReferencedObject* hkpRigidBodyRef = bhkRigidBody->referencedObject.get();
				if (hkpRigidBodyRef) {
					hkpRigidBody* rigidBody = skyrim_cast<hkpRigidBody*>(hkpRigidBodyRef);
					if (rigidBody) {
						DrawRigidBody(rigidBody);
					}
				}
			}
		}
	}

	void DrawNiNodes(NiAVObject* root) {
		std::deque<NiAVObject*> queue;
		queue.push_back(root);


		while (!queue.empty()) {
			auto currentnode = queue.front();
			queue.pop_front();
			try {
				if (currentnode) {
					auto ninode = currentnode->AsNode();
					if (ninode) {
						for (auto child: ninode->GetChildren()) {
							// Bredth first search
							queue.push_back(child.get());
							// Depth first search
							//queue.push_front(child.get());
						}
					}
					DrawNiAvObject(ninode);
				}
			}
			catch (const std::overflow_error& e) {
				log::warn("Overflow: {}", e.what());
			} // this executes if f() throws std::overflow_error (same type rule)
			catch (const std::runtime_error& e) {
				log::warn("Underflow: {}", e.what());
			} // this executes if f() throws std::underflow_error (base class rule)
			catch (const std::exception& e) {
				log::warn("Exception: {}", e.what());
			} // this executes if f() throws std::logic_error (base class rule)
			catch (...) {
				log::warn("Exception Other");
			}
		}
	}

	void DrawCharController(Actor* actor) {
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}
		auto charController = actor->GetCharController();
		if (!charController) {
			return;
		}

		log::info("Drawing char controller");
		hkpRigidBody* supportBody = charController->supportBody.get();
		if (supportBody) {
			log::info("  - Support Body");
			DrawRigidBody(supportBody);
		}

		hkpRigidBody* bumpedBody = charController->bumpedBody.get();
		if (bumpedBody) {
			log::info("  - Bumped Body");
			DrawRigidBody(bumpedBody);
		}

		hkpRigidBody* bumpedCharCollisionObject = charController->bumpedCharCollisionObject.get();
		if (bumpedCharCollisionObject) {
			log::info("  - Bumped Char Collision Object");
			DrawRigidBody(bumpedCharCollisionObject);
		}

		{
			hkTransform outTransform;
			charController->GetTransformImpl(outTransform);
			glm::mat4 transform = HkToGlm(outTransform);
			for (auto bhkShape: charController->shapes) {
				hkpShape* shape = static_cast<hkpShape*>(bhkShape->referencedObject.get());
				if (shape) {
					log::info("  - Shape of CharController");
					DrawShape(shape, transform);
				}
			}
		}

		bhkCharProxyController* charProxyController = skyrim_cast<bhkCharProxyController*>(charController);
		if (charProxyController) {
			log::info("HAS bhkCharProxyController");
			auto& proxy = charProxyController->proxy;
			hkReferencedObject* refObject = proxy.referencedObject.get();
			if (refObject) {
				log::info("Has refObject");
				hkpCharacterProxy* hkpObject = skyrim_cast<hkpCharacterProxy*>(refObject);
				if (hkpObject) {
					for (hkpRigidBody* body: hkpObject->bodies) {
						log::info("Draw body");
						DrawRigidBody(body);
					}
					for (auto phantom: hkpObject->phantoms) {
						log::info("Draw Body");
						DrawWorldObject(phantom);
					}
					auto shapePhantom = hkpObject->shapePhantom;
					if (shapePhantom) {
						log::info("Draw shape phantom");
						DrawWorldObject(shapePhantom);
					}
				}
			}
		}
	}

	void DrawRagdoll(Actor* actor) {
		BSAnimationGraphManagerPtr animGraphManager;
		if (actor->GetAnimationGraphManager(animGraphManager)) {
			for (auto& graph : animGraphManager->graphs) {
				if (graph) {
					auto& character = graph->characterInstance;
					auto ragdollDriver = character.ragdollDriver.get();
					if (ragdollDriver) {
						auto ragdoll = ragdollDriver->ragdoll;
						if (ragdoll) {
							log::info("Got ragdoll");
							for (auto& rb: ragdoll->rigidBodies) {
								DrawRigidBody(rb);
							}
						}
					}
				}
			}
		}
	}

	void DrawRay(Actor* actor) {
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}
		auto charController = actor->GetCharController();
		if (!charController) {
			return;
		}

		hkTransform outTransform;
		charController->GetTransformImpl(outTransform);
		glm::mat4 transform = HkToGlm(outTransform);

		glm::vec3 worldForward = glm::normalize(HkVecToGlmVec(charController->forwardVec)) * glm::vec3(-1);

		glm::vec3 actorPos = HkToGlm(outTransform.translation);
		glm::vec3 rayStart = actorPos + worldForward * meter_to_unit(0.5);
		glm::vec3 rayEnd = rayStart + worldForward * meter_to_unit(5);

		DebugAPI::DrawLineForMS(rayStart, rayEnd, MS_TIME, RAY_COLOR, RAY_LINETHICKNESS);

		TESObjectCELL* cell = actor->GetParentCell();
		bool success = false;
		auto results = CastRayResults(cell, Glm2Ni(rayStart), Glm2Ni(worldForward), meter_to_unit(5), success, {});
		if (success) {
			for (auto result: results) {
				const hkTransform* shapeHkTransform = static_cast<const hkTransform*>(result.motion);
				if (motion) {
					glm::mat4 shapeTransform = HkToGlm(shapeHkTransform);
					DrawShape(result.shape, shapeTransform);
					auto collidable = result.rootCollidable;
					if (collidable) {
						auto type = collidable->m_broadPhaseHandle.type;
						switch (type) {
							case 0: {
								// Invalid
							}
							case 1: {
								// hkpEntity
								hkpEntity* entity = collidable->GetOwner<hkpEntity>();
								log::info("It's an entity");
							}
							case 2: {
								// hkpPhantom
								hkpEntity* phantom = collidable->GetOwner<hkpPhantom>();
								log::info("It's a phantom");
							}
							case 3: {
								// hkpBroadPhaseBorder
							}
						}
					}
				}
			}
		}
	}

	void DrawActor(Actor* actor) {
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}

		auto root = actor->GetCurrent3D();
		// DrawNiNodes(root);

		DrawCharController(actor);
		DrawRagdoll(actor);
	}


}

namespace Gts {
	DDraw& DDraw::GetSingleton() noexcept {
		static DDraw instance;
		return instance;
	}

	void DDraw::Update() {
		if (!this->enabled.load()) {
			return;
		}

		for (auto actor: find_team_player()) {
			if (!actor->Is3DLoaded()) {
				continue;
			}

			// DrawActor(actor);
			DrawRay(actor);
		}
	}
}
