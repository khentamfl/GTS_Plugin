#include "managers/ddraw.hpp"
#include "UI/DebugAPI.hpp"
#include "node.hpp"
#include "util.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;


namespace {
	void DrawRigidBody(hkpRigidBody* rigid_body) {
		auto shape = rigid_body->GetShape();
		if (shape) {
			if (shape->type == hkpShapeType::kCapsule) {
				const hkpCapsuleShape* capsule = static_cast<const hkpCapsuleShape*>(shape);
				if (capsule) {
					glm::vec3 start = HkToGlm(capsule->vertexA) * *g_worldScaleInverse;
					glm::vec3 end = HkToGlm(capsule->vertexB) * *g_worldScaleInverse;

					float radius = capsule->radius * *g_worldScaleInverse;

					auto& transform = rigid_body->motion.motionState.transform;
					glm::vec3 translation = HkToGlm(transform.translation)  * *g_worldScaleInverse;
					glm::mat3 rotation = HkToGlm(transform.rotation);
					float x_angle = 0.0;
					float y_angle = 0.0;
					float z_angle = 0.0;
					glm::extractEulerAngleXYZ(glm::mat4(rotation), x_angle, y_angle, z_angle);
					glm::vec3 euler_angles = glm::vec3(x_angle, y_angle, z_angle);
					log::info("Drawing Capsule, {},{},{}={},{},{}", start.x,start.y,start.z,end.x,end.y,end.z);
					DebugAPI::DrawCapsule(translation + start, translation + end, radius, euler_angles);
				}
			}
		}
	}

	void DrawNiAvObject(NiAVObject* currentnode) {
		auto collision_object = currentnode->GetCollisionObject();
		if (collision_object) {
			auto bhk_rigid_body = collision_object->GetRigidBody();
			if (bhk_rigid_body) {
				hkReferencedObject* hkp_rigidbody_ref = bhk_rigid_body->referencedObject.get();
				if (hkp_rigidbody_ref) {
					hkpRigidBody* hkp_rigidbody = skyrim_cast<hkpRigidBody*>(hkp_rigidbody_ref);
					if (hkp_rigidbody) {
						DrawRigidBody(hkp_rigidbody);
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
		auto player = PlayerCharacter::GetSingleton();
		if (!player) {
			return;
		}
		if (!player->Is3DLoaded()) {
			return;
		}

		auto model = player->GetCurrent3D();
		if (model) {
			auto spine_node = find_node(player, "NPC Spine [Spn0]", Person::Current);
			if (spine_node) {
				DebugAPI::DrawSphere(Ni2Glm(spine_node->world.translate), meter_to_unit(2.0));
			}
		}
	}
}
