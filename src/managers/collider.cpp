#include "managers/collider.hpp"
#include "scale/scale.hpp"

#include "util.hpp"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {
	void RescaleCapsule(hkpCapsuleShape* capsule, CapsuleData* data, float& scale_factor, hkVector4& vec_scale) {
		const float EPSILON = 1e-3;

		float expected_radius = data->radius * scale_factor;
		if(fabs(capsule->radius - expected_radius) > EPSILON) {
			capsule->vertexA = data->start * vec_scale;
			capsule->vertexB = data->end * vec_scale;
			log::info("Old Radius: {}", capsule->radius);
			capsule->radius = expected_radius;
			log::info("New Radius: {}", capsule->radius);

			// Dragons
			hkpCapsuleShape* ctd_me_capsule = new hkpCapsuleShape();
		}
	}

	void AddNode(ActorData* actor_data, NiAVObject* currentnode) { // NOLINT
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
								actor_data->GetCapsuleData(capsule); // Will insert the data
							}
						}
					}
				}
			}
		}
	}

	void SearchColliders(NiAVObject* root, ActorData* actor_data) {
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
					// Do smth//
					AddNode(actor_data, currentnode);
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

	void ScaleColliders(Actor* actor, ActorData* actor_data) {
		const float EPSILON = 1e-3;
		if (!actor->Is3DLoaded()) {
			return;
		}
		float visual_scale = get_visual_scale(actor);
		float natural_scale = get_natural_scale(actor);
		float scale_factor = visual_scale/natural_scale;

		if (fabs(actor_data->last_scale - scale_factor) <= EPSILON) {
			return;
		}
		log::info("Updating: {}", actor->GetDisplayFullName());
		actor_data->last_scale = scale_factor;

		bool search_nodes = !actor_data->HasCapsuleData();
		if (search_nodes) {
			for (auto person: {true, false} ) {
				auto model = actor->Get3D(person);
				if (model) {
					SearchColliders(model, actor_data);
				}
			}
		}

		hkVector4 vec_scale = hkVector4(scale_factor, scale_factor, scale_factor, 1.0);
		for (auto &[capsule, capsule_data]: actor_data->GetCapsulesData()) {
			if (capsule) {
				RescaleCapsule(capsule, &capsule_data, scale_factor, vec_scale);
			}
		}
	}
}

namespace Gts {

	ColliderManager& ColliderManager::GetSingleton() noexcept {
		static ColliderManager instance;
		return instance;
	}

	void ColliderManager::Update() {
		for (auto actor: find_actors()) {
			ActorData* actor_data = GetActorData(actor);
			if (actor_data) {
				ScaleColliders(actor, actor_data);
			}
		}
	}

	ActorData* ColliderManager::GetActorData(Actor* actor) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!actor) {
			return nullptr;
		}
		auto key = actor;
		ActorData* result = nullptr;
		try {
			result = &this->actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			this->actor_data.try_emplace(key);
			try {
				result = &this->actor_data.at(key);
			} catch (const std::out_of_range& oor) {
				result = nullptr;
			}
		}
		return result;
	}

	CapsuleData* ActorData::GetCapsuleData(hkpCapsuleShape* capsule) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!capsule) {
			return nullptr;
		}
		auto key = capsule;
		CapsuleData* result = nullptr;
		try {
			result = &this->capsule_data.at(key);
		} catch (const std::out_of_range& oor) {
			CapsuleData new_data;
			new_data.start = capsule->vertexA;
			new_data.end = capsule->vertexB;
			new_data.radius = capsule->radius;
			this->capsule_data.try_emplace(key, new_data);
			try {
				result = &this->capsule_data.at(key);
			} catch (const std::out_of_range& oor) {
				result = nullptr;
			}
		}
		return result;
	}
}
