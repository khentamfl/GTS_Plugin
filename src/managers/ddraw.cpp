#include "managers/ddraw.hpp"
#include "UI/DebugAPI.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;


namespace {
	const float MS_TIME = 10;
	const glm::vec4 CAPSULE_COLOR = { 0.0f, 0.0f, 1.0f, 1.0f };
	const float CAPSULE_LINETHICKNESS = 0.1;

	void DrawShape(const hkpShape* shape, const glm::mat4& transform) {
		if (shape->type == hkpShapeType::kCapsule) {
			const hkpCapsuleShape* capsule = static_cast<const hkpCapsuleShape*>(shape);
			if (capsule) {
				glm::vec3 start = HkToGlm(capsule->vertexA);
				glm::vec3 end = HkToGlm(capsule->vertexB);
				float radius = capsule->radius * *g_worldScaleInverse;

				DebugAPI::DrawCapsule(start, end, radius, transform, MS_TIME, CAPSULE_COLOR, CAPSULE_LINETHICKNESS);
			}
		} else if (shape->type == hkpShapeType::kList) {
			auto container = static_cast<const hkpListShape*>(shape);
			auto key = container->GetFirstKey();
			while (key != HK_INVALID_SHAPE_KEY) {
				auto buffer = hkpShapeBuffer();
				auto child_shape = container->GetChildShape(key, buffer);
				if (child_shape) {
					DrawShape(child_shape, transform);
				}
				key = container->GetNextKey(key);
			}
		} else if (shape->type == hkpShapeType::kBVTree) {
			auto actual_shape = static_cast<const hkpBvTreeShape*>(shape);
			const hkpShapeContainer* container = actual_shape->GetContainer();
			auto key = container->GetFirstKey();
			while (key != HK_INVALID_SHAPE_KEY) {
				auto buffer = hkpShapeBuffer();
				auto child_shape = container->GetChildShape(key, buffer);
				if (child_shape) {
					DrawShape(child_shape, transform);
				}
				key = container->GetNextKey(key);
			}
		} else {
			log::debug("- Shape (of type {}) is not handlled", static_cast<int>(shape->type));
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

		auto model = actor->GetCurrent3D();
		if (model) {
			glm::mat4 transform = Ni2Glm(model->world);
			for (auto bhkShape: charController->shapes) {
				hkpShape* shape = static_cast<hkpShape*>(bhkShape->referencedObject.get());
				if (shape) {
					log::info("  - Shape of CharController");
					DrawShape(shape, transform);
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
		DrawNiNodes(root);

		DrawCharController(actor);
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

			DrawActor(actor);
		}
	}
}
