#include "managers/collider/ragdoll_data.hpp"
#include "util.hpp"
#include "hooks/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	RagdollData::RagdollData() {
		RagdollData(nullptr);
	}

	RagdollData::RagdollData(hkaRagdollInstance* ragdoll) {
		this->Init(ragdoll);
		if (ragdoll) {
			this->UpdateColliders(ragdoll);
		}
	}

	RagdollData::~RagdollData () {
	}

	void RagdollData::Init(hkaRagdollInstance* ragdoll) {
		if (this->ragdoll != ragdoll) {
			this->ragdoll = ragdoll;
			this->capsule_data.clear();
			this->rb_data.clear();
		}
	}

	void RagdollData::UpdateColliders(hkaRagdollInstance* ragdoll) {
		if (this->ragdoll != ragdoll) {
			this->Init(ragdoll);
		}
		if (!ragdoll) {
			return;
		}

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

	void RagdollData::ApplyScale(const float& new_scale, const hkVector4& vec_scale) {
		for (auto &[key, data]: this->capsule_data) {
			data.ApplyScale(new_scale, vec_scale);
		}
	}

	void RagdollData::ApplyPose(const hkVector4& origin, const float& new_scale) {
		for (auto &[key, data]: this->rb_data) {
			data.ApplyPose(origin, new_scale);
		}
	}

	void RagdollData::PruneColliders(Actor* actor) {
		for (auto i = this->capsule_data.begin(); i != this->capsule_data.end();) {
			auto& data = (*i);
			auto key = data.first;
			if (key->GetReferenceCount() == 1) {
				i = this->capsule_data.erase(i);
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

	void RagdollData::AddCapsule(hkpCapsuleShape* orig_capsule) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!orig_capsule) {
			return;
		}
		auto key = orig_capsule;
		this->capsule_data.try_emplace(key, orig_capsule);
	}

	void RagdollData::AddRidgedBody(hkpRigidBody* rigid_body) {
		std::unique_lock lock(this->_lock);
		if (!rigid_body) {
			return;
		}
		auto key = rigid_body;
		this->rb_data.try_emplace(key, rigid_body);
	}
}
