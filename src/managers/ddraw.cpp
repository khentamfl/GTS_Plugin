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

	const glm::vec4 TRIANGLE_COLOR = { 0.00f, 0.00f, 1.00f, 1.0f };
	const float TRIANGLE_LINETHICKNESS = 0.1;

	const glm::vec4 BOX_COLOR = { 0.00f, 1.00f, 0.00f, 1.0f };
	const float BOX_LINETHICKNESS = 0.1;

	const glm::vec4 CONVEXVERTS_COLOR = { 0.00f, 1.00f, 1.00f, 1.0f };
	const float CONVEXVERTS_LINETHICKNESS = 1.0;

	const glm::vec4 UNKNOWN_COLOR = { 1.00f, 0.00f, 0.00f, 1.0f };
	const float UNKNOWN_LINETHICKNESS = 1.0;

	const glm::vec4 AABB_COLOR = { 0.00f, 0.00f, 0.00f, 1.0f };
	const float AABB_LINETHICKNESS = 3.0;

	const glm::vec4 RAY_COLOR = { 1.00f, 1.00f, 1.00f, 1.0f };
	const float RAY_LINETHICKNESS = 3.0;

	inline static glm::vec3 GLM_HK_TO_SK = glm::vec3((*Gts::g_worldScaleInverse));

	void DrawAabb(const hkpShape* shape, const glm::mat4& transform, const glm::vec4& color) {
		RE::hkTransform shapeTransform;
		// use identity matrix for the BB of the unrotated object
		shapeTransform.rotation.col0 = { 1.0f, 0.0f, 0.0f, 0.0f };
		shapeTransform.rotation.col1 = { 0.0f, 1.0f, 0.0f, 0.0f };
		shapeTransform.rotation.col2 = { 0.0f, 0.0f, 1.0f, 0.0f };

		shapeTransform.translation.quad = _mm_set_ps(0.0f, 0.0f, 0.0f, 1.0f);
		RE::hkAabb boundingBoxLocal;
		shape->GetAabbImpl(shapeTransform, 0.0f, boundingBoxLocal);
		glm::vec3 extends = HkToGlm(boundingBoxLocal.max) - HkToGlm(boundingBoxLocal.min);
		glm::vec3 halfExtends = extends * glm::vec3(0.5,0.5,0.5);
		glm::vec3 origin = HkToGlm(boundingBoxLocal.min) + halfExtends;
		DebugAPI::DrawBox(origin, halfExtends, transform, MS_TIME, color, AABB_LINETHICKNESS);
	}

	void DrawShape(const hkpShape* shape, const glm::mat4& transform) {
		DrawAabb(shape, transform,AABB_COLOR);
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
			const hkpTriangleShape* triangle = static_cast<const hkpTriangleShape*>(shape);
			if (triangle) {
				glm::vec3 pointA = HkToGlm(triangle->vertexA);
				glm::vec3 pointB = HkToGlm(triangle->vertexB);
				glm::vec3 pointC = HkToGlm(triangle->vertexC);

				DebugAPI::DrawTriangle(pointA, pointB, pointB, transform, MS_TIME, TRIANGLE_COLOR, TRIANGLE_LINETHICKNESS);
			}
		} else if (shape->type == hkpShapeType::kConvexVertices) {
			// log::info("Convext Verts");
			// Way too much of a pain to RE
			// Just show a sphere and a BB
			const hkpConvexVerticesShape* convexShape = static_cast<const hkpConvexVerticesShape*>(shape);
			if (convexShape) {
				float radius = convexShape->radius * (*Gts::g_worldScaleInverse);
				glm::vec3 localPos = glm::vec3(0.0, 0.0, 0.0);
				glm::vec3 worldPos = ApplyTransform(localPos, transform);
				DrawAabb(convexShape, transform, UNKNOWN_COLOR);
				DebugAPI::DrawSphere(worldPos, radius, MS_TIME, UNKNOWN_COLOR, UNKNOWN_LINETHICKNESS);
				log::info("Prepare for kConvexVertices CTD");
				std::size_t numVertices = convexShape->numVertices;
				log::info("  - numVertices: {}", numVertices);
				glm::vec3 previous = ApplyTransform(
					glm::vec3(
						convexShape->rotatedVertices[0].vertices[0].quad.m128_f32[0],
						convexShape->rotatedVertices[0].vertices[1].quad.m128_f32[0],
						convexShape->rotatedVertices[0].vertices[2].quad.m128_f32[0]
						) * GLM_HK_TO_SK,
					transform);
				log::info("  - Firs Vert: {},{},{}", previous[0],previous[1],previous[2]);

				for (std::size_t i = 0; i < numVertices; i++) {
					std::size_t j = i / 4;
					std::size_t k = i % 4;
					glm::vec3 vert = ApplyTransform(
						glm::vec3(
							convexShape->rotatedVertices[j].vertices[0].quad.m128_f32[k],
							convexShape->rotatedVertices[j].vertices[1].quad.m128_f32[k],
							convexShape->rotatedVertices[j].vertices[2].quad.m128_f32[k]
							) * GLM_HK_TO_SK,
						transform);
					log::info("  - Vert: {},{},{}", vert[0], vert[1], vert[2]);
					DebugAPI::DrawLineForMS(previous, vert,MS_TIME, CONVEXVERTS_COLOR, CONVEXVERTS_LINETHICKNESS);
					previous = vert;
				}
				log::info("What really? Where is the CTD I DEMAND A CTD");
			}
		} else if (shape->type == hkpShapeType::kConvexTransform) {
			// log::info("Convext transform");
			const hkpConvexTransformShape* transformShape = static_cast<const hkpConvexTransformShape*>(shape);
			if (transformShape) {
				auto container = transformShape->GetContainer();
				auto childTransform = transform * HkToGlm(transformShape->transform);
				auto key = container->GetFirstKey();
				while (key != HK_INVALID_SHAPE_KEY) {
					auto buffer = hkpShapeBuffer();
					auto childShape = container->GetChildShape(key, buffer);
					if (childShape) {
						// log::info("  - Child");
						DrawShape(childShape, childTransform);
					}
					key = container->GetNextKey(key);
				}
			}
		} else if (shape->type == hkpShapeType::kBox) {
			// log::info("Box");
			const hkpBoxShape* box = static_cast<const hkpBoxShape*>(shape);
			if (box) {
				glm::vec3 origin = glm::vec3(0.,0.,0.);
				glm::vec3 halfExtents = HkToGlm(box->halfExtents);

				DebugAPI::DrawBox(origin, halfExtents, transform, MS_TIME, TRIANGLE_COLOR, TRIANGLE_LINETHICKNESS);
			}
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
				if (shapeHkTransform) {
					glm::mat4 shapeTransform = HkToGlm(*shapeHkTransform);
					DrawShape(result.shape, shapeTransform);
					auto collidable = result.rootCollidable;
					if (collidable) {
						auto type = collidable->broadPhaseHandle.type;
						log::info("Collidable: {}", type);
						switch (type) {
							case 0: {
								// Invalid
								break;
							}
							case 1: {
								// hkpEntity
								hkpEntity* entity = collidable->GetOwner<hkpEntity>();
								log::info("It's an entity");
								auto objectRefr = entity->GetUserData();
								if (objectRefr) {
									log::info("  - Owner: {}", objectRefr->GetDisplayFullName());
									log::info("  - Type: {}", static_cast<int>(result.shape->type));
								}
								break;
							}
							case 2: {
								// hkpPhantom
								hkpPhantom* phantom = collidable->GetOwner<hkpPhantom>();
								log::info("It's a phantom");
								auto objectRefr = phantom->GetUserData();
								if (objectRefr) {
									log::info("  - Owner: {}", objectRefr->GetDisplayFullName());
									log::info("  - Type: {}", static_cast<int>(result.shape->type));
								}
								break;
							}
							case 3: {
								// hkpBroadPhaseBorder
								break;
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
