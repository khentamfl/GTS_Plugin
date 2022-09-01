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

	void DrawRigidBody(hkpRigidBody* rigidBody) {
		auto shape = rigidBody->GetShape();
		if (shape) {
			if (shape->type == hkpShapeType::kCapsule) {
				const hkpCapsuleShape* capsule = static_cast<const hkpCapsuleShape*>(shape);
				if (capsule) {
					glm::vec3 start = HkToGlm(capsule->vertexA);
					glm::vec3 end = HkToGlm(capsule->vertexB);
					float radius = capsule->radius * *g_worldScaleInverse;

					glm::mat4 transform = HkToGlm(rigidBody->motion.motionState.transform);
					DebugAPI::DrawCapsule(start, end, radius, transform, MS_TIME, CAPSULE_COLOR, CAPSULE_LINETHICKNESS);
				}
			}
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

	void DrawActor(Actor* actor) {
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}

		auto root = actor->GetCurrent3D();
		DrawNiNodes(root);
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
