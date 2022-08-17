#include "managers/collider.hpp"

#include "util.hpp"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {
	void RescaleCapsule(hkpCapsuleShape* capsule, CapsuleData* data, float visual_scale) {
		hkVector4 vec_scale = hkVector4(visual_scale, visual_scale, visual_scale, 1.0);
		capsule->vertexA = capsule->vertexA * vec_scale;
		capsule->vertexB = capsule->vertexB * vec_scale;
		capsule->radius *= visual_scale;
	}

	void ProcessNode(ActorData* actor_data, NiAVObject* currentnode, float visual_scale) { // NOLINT
		const float EPSILON = 1e-3;
		auto collision_object = currentnode->GetCollisionObject();
		if (collision_object) {
			auto bhk_rigid_body = collision_object->GetRigidBody();
			if (bhk_rigid_body) {
				hkReferencedObject* hkp_rigidbody_ref = bhk_rigid_body->referencedObject.get();
				if (hkp_rigidbody_ref) {
					hkpRigidBody* hkp_rigidbody = skyrim_cast<hkpRigidBody*>(hkp_rigidbody_ref);
					if (hkp_rigidbody) {
						auto shape = hkp_rigidbody->GetShape();
						if (shape) {
							if (shape->type == hkpShapeType::kCapsule) {
								const hkpCapsuleShape* orig_capsule = static_cast<const hkpCapsuleShape*>(shape);
								// Here be dragons
								hkpCapsuleShape* capsule = const_cast<hkpCapsuleShape*>(orig_capsule);
								CapsuleData* capsule_data = actor_data->GetCapsuleData(capsule);
								if (capsule_data) { // NOLINT
									if(fabs(capsule_data->prev_scale - visual_scale) > EPSILON) {
										RescaleCapsule(capsule, capsule_data, visual_scale);
									}
								}
							}
							hkp_rigidbody->SetShape(capsule);
						}
					}
				}
			}
		}
	}

	void ScaleColliders(Actor* actor, ActorData* actor_data) {
		if (!actor->Is3DLoaded()) {
			return;
		}
		auto model = actor->Get3D();
		if (!model) {
			return;
		}
		// Game lookup failed we try and find it manually
		std::deque<NiAVObject*> queue;
		queue.push_back(model);


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
					// Do smth
					ProcessNode(currentnode);
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
}

namespace Gts {

	ContactManager& ColliderManager::GetSingleton() noexcept {
		static ContactManager instance;
		return instance;
	}

	void ColliderManager::Update() {
		for (auto actor: find_actors()) {
			ActorData* actor_data = GetActorData(actor);
			ScaleColliders(actor, actor_data);
		}
	}

	ActorData* ColliderManager::GetActorData(Actor* actor) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!actor) {
			return nullptr;
		}
		auto key = actor;
		try {
			auto no_discard = this->actor_data.at(key);
		} catch (const std::out_of_range& oor) {

			ActorData result;
			this->actor_data.try_emplace(key, result);
		}
		return &this->actor_data[key];
	}

	CapsuleData* ActorData::GetCapsuleData(hkpCapsuleShape* capsule) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!capsule) {
			return nullptr;
		}
		auto key = capsule;
		try {
			auto no_discard = this->capsule_data.at(key);
		} catch (const std::out_of_range& oor) {
			CapsuleData result;
			result.prev_scale = 1.0;
			result.start = capsule->vertexA;
			result.end = capsule->vertexB;
			result.radius = capsule->radius;
			this->capsule_data.try_emplace(key, result);
		}
		return &this->capsule_data[key];
	}
}
