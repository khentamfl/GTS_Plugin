#include "colliders/common.hpp"

using namespace RE;

namespace Gts {
  COL_LAYER GetCollidesWith(const std::uint32_t& collisionFilterInfo) {
    return static_cast<COL_LAYER>(collisionFilterInfo & 0x7F);
  }
  COL_LAYER GetCollidesWith(const hkpCollidable* collidable) {
    if (collidable) {
      return GetCollidesWith(collidable->broadPhaseHandle.collisionFilterInfo);
    } else {
      return COL_LAYER::kUnidentified;
    }
  }
  COL_LAYER GetCollidesWith(const hkpWorldObject* entity) {
    if (entity) {
      auto collidable = entity->GetCollidable();
      return GetCollidesWith(collidable);
    } else {
      return COL_LAYER::kUnidentified;
    }
  }

  void SetCollidesWith(std::uint32_t& collisionFilterInfo, const COL_LAYER& newLayer) {
    auto newCollision = collisionFilterInfo & 0xFFFFFF80; // Clear old one
    newCollision = newCollision | static_cast<std::uint32_t>(newLayer);
    collisionFilterInfo = newCollision;
  }
  void SetCollidesWith(hkpCollidable* collidable, const COL_LAYER& newLayer) {
    if (collidable) {
      return SetCollidesWith(collidable->broadPhaseHandle.collisionFilterInfo, newLayer);
    }
  }
  void SetCollidesWith(hkpWorldObject* entity, const COL_LAYER& newLayer) {
    if (entity) {
      auto collidable = entity->GetCollidableRW();
      return SetCollidesWith(collidable, newLayer);
    }
  }


	void ColliderData::AddRB(hkpRigidBody* rb) {
		if (!rb) {
			return;
		}
    this->rbs.try_emplace(rb, hkRefPtr(rb));
	}
  void ColliderData::AddPhantom(hkpPhantom* phantom) {
		if (!phantom) {
			return;
		}
    this->phantoms.try_emplace(phantom, hkRefPtr(phantom));
	}

  std::vector<ColliderData*> ColliderData::GetChildren() {
    return {};
  }

  std::vector<hkpRigidBody*> ColliderData::GetRigidBodies() {
    std::vector<hkpRigidBody*> entities = {};
    for (auto& [key, rb]: this->rbs) {
      entities.push_back(rb.get());
    }
    for (auto& child: GetChildren()) {
      for (auto& ent: child->GetRigidBodies()) {
        entities.push_back(ent);
      }
    }
    return entities;
  }
  std::vector<hkpPhantom*> ColliderData::GetPhantoms() {
    std::vector<hkpPhantom*> entities = {};
    for (auto& [key, ph]: this->phantoms) {
      entities.push_back(ph.get());
    }
    for (auto& child: GetChildren()) {
      for (auto& ent: child->GetPhantoms()) {
        entities.push_back(ent);
      }
    }
    return entities;
  }

  std::vector<hkpWorldObject*> ColliderData::GetWorldObjects() {
    std::vector<hkpWorldObject*> entities = {};
    for (auto& rb: GetRigidBodies()) {
      entities.push_back(rb);
    }
    for (auto& ph: GetPhantoms()) {
      entities.push_back(ph);
    }
    return entities;
  }

  void ColliderData::DisableCollisions() {
    std::vector<hkpWorldObject*> entities = GetWorldObjects();
    for (auto& rb: GetRigidBodies()) {
      // Disable gravity
      // log::info("Disable gravity (was {})", rb->motion.gravityFactor);
      rb->motion.gravityFactor = 0.0;
      rb->motion.SetMassInv(0.0);
    }

    for (auto& ent: GetWorldObjects()) {
      auto collidable = ent->GetCollidable();
      if (collidable) {
        // log::info("- Disable collision");
        // log::info("Current info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
        // log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);

        // Change collides with
        if (GetCollidesWith(ent) == COL_LAYER::kCharController) {
          SetCollidesWith(ent, COL_LAYER::kNonCollidable);
        }

        // log::info("    New info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
        // log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);
      }
    }
  }

  void ColliderData::EnableCollisions() {
    for (auto& rb: GetRigidBodies()) {
      // Enable gravity
      rb->motion.gravityFactor = 1.0;
      rb->motion.SetMassInv(1.0);
    }

    for (auto& ent: GetWorldObjects()) {
      auto collidable = ent->GetCollidable();
      if (collidable) {
        // log::info("- Enabling collision");
        // log::info("Current info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
        // log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);

        // Change collides with
        if (GetCollidesWith(ent) == COL_LAYER::kNonCollidable) {
          SetCollidesWith(ent, COL_LAYER::kCharController);
        }

        // log::info("    New info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
        // log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);
      }
    }
  }
}
