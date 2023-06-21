 #include "colliders/charcontroller.hpp"
#include "colliders/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace {
  COL_LAYER GetCollidesWith(const std::uint32_t& collisionFilterInfo) {
    return static_cast<COL_LAYER>(collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);
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
      auto collidable = ent->GetCollidable();
      return GetCollidesWith(collidable);
    } else {
      return COL_LAYER::kUnidentified;
    }
  }

  COL_LAYER GetBelongsTo(const std::uint32_t& collisionFilterInfo) {
    return static_cast<COL_LAYER>((collidable->broadPhaseHandle.collisionFilterInfo >> 16) & 0x7F);
  }
  COL_LAYER GetBelongsTo(const hkpCollidable* collidable) {
    if (collidable) {
      return GetCollidesWith(collidable->broadPhaseHandle.collisionFilterInfo);
    } else {
      return COL_LAYER::kUnidentified;
    }
  }
  COL_LAYER GetBelongsTo(const hkpWorldObject* entity) {
    if (entity) {
      auto collidable = ent->GetCollidable();
      return GetCollidesWith(collidable);
    } else {
      return COL_LAYER::kUnidentified;
    }
  }

  void SetCollidesWith(std::uint32_t& collisionFilterInfo, const COL_LAYER& newLayer) {
    auto newCollision = collidable->broadPhaseHandle.collisionFilterInfo & 0xFFFFFF80; // Clear old one
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
      auto collidable = ent->GetCollidableRW();
      return SetCollidesWith(collidable, newLayer);
    }
  }

  void SetBelongsTo(std::uint32_t& collisionFilterInfo, const COL_LAYER& newLayer) {
    auto newCollision = collidable->broadPhaseHandle.collisionFilterInfo & 0xFF80FFFF; // Clear old one
    newCollision = newCollision | (static_cast<std::uint32_t>(newLayer) << 16);
    collisionFilterInfo = newCollision;
  }
  void SetBelongsTo(hkpCollidable* collidable, const COL_LAYER& newLayer) {
    if (collidable) {
      return SetCollidesWith(collidable->broadPhaseHandle.collisionFilterInfo, newLayer);
    }
  }
  void SetBelongsTo(hkpWorldObject* entity, const COL_LAYER& newLayer) {
    if (entity) {
      auto collidable = ent->GetCollidableRW();
      return SetCollidesWith(collidable, newLayer);
    }
  }


}

namespace Gts {
	CharContData::CharContData(bhkCharacterController* charCont) {
		this->ReInit(charCont);
	}

	void CharContData::ReInit(bhkCharacterController* charCont) {
		this->charCont = charCont;
		this->rbs.clear();
		this->phantoms.clear();

		if (!charCont) {
			return;
		}


		bhkCharProxyController* charProxyController = skyrim_cast<bhkCharProxyController*>(charCont);
		bhkCharRigidBodyController* charRigidBodyController = skyrim_cast<bhkCharRigidBodyController*>(charCont);
		if (charProxyController) {
			// Player controller is a proxy one
			auto& proxy = charProxyController->proxy;
			hkReferencedObject* refObject = proxy.referencedObject.get();
			if (refObject) {
				hkpCharacterProxy* hkpObject = skyrim_cast<hkpCharacterProxy*>(refObject);
				if (hkpObject) {
					// This one appears to be active during combat.
					// Maybe used for sword swing collision detection
					for (auto phantom: hkpObject->phantoms) {
						this->AddPhantom(phantom);
					}

					// This is the actual shape
					if (hkpObject->shapePhantom) {
						this->AddPhantom(hkpObject->shapePhantom);
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
						this->AddRB(hkpObject->m_character);
					}
				}
			}
		}
	}

	void CharContData::AddRB(hkpRigidBody* rb) {
		if (!rb) {
			return;
		}
    this->rbs.try_emplace(rb, hkRefPtr(rb));
	}
  void CharContData::AddPhantom(hkpPhantom* phantom) {
		if (!phantom) {
			return;
		}
    this->phantoms.try_emplace(phantom, hkRefPtr(phantom));
	}

  void CharContData::DisableCollisions() {
    std::vector<hkpWorldObject*> entities = {};
    for (auto& [key, rb]: this->rbs) {
      entities.push_back(rb.get());

      // Disable gravity
      log::info("Disable gravity (was {})", rb->motion.gravityFactor);
      rb->motion.gravityFactor = 0.0;
      rb->motion.SetMassInv(0.0);
    }
    for (auto& [key, ph]: this->phantoms) {
      entities.push_back(ph.get());
    }

    for (auto ent: entities) {
      auto collidable = ent->GetCollidableRW();
      if (collidable) {
        log::info("- Disable collision");
        log::info("Current info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
        log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);
        log::info("     belongs: {:0X}", (collidable->broadPhaseHandle.collisionFilterInfo >> 16) & 0x7F);

        // Change collides with
        if (GetCollidesWith(ent) == COL_LAYER::kCharController) {
          SetCollidesWith(ent, COL_LAYER::kNonCollidable);
        }

        // Change belongs to
        if (GetBelongsTo(ent) == COL_LAYER::kCamera) {
          SetBelongsTo(ent, COL_LAYER::kNonCollidable);
        }

        log::info("    New info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
        log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);
        log::info("     belongs: {:0X}", (collidable->broadPhaseHandle.collisionFilterInfo >> 16) & 0x7F);
      }
    }
  }

  void CharContData::EnableCollisions() {
    log::info("Trying to enable collision");
    std::vector<hkpWorldObject*> entities = {};
    for (auto& [key, rb]: this->rbs) {
      entities.push_back(rb.get());

      // Enable gravity
      rb->motion.gravityFactor = 1.0;
      rb->motion.SetMassInv(1.0);
    }
    for (auto& [key, ph]: this->phantoms) {
      entities.push_back(ph.get());
    }

    for (auto ent: entities) {
      auto collidable = ent->GetCollidableRW();
      if (collidable) {
        log::info("- Enabling collision");
        log::info("Current info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
        log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);
        log::info("     belongs: {:0X}", (collidable->broadPhaseHandle.collisionFilterInfo >> 16) & 0x7F);

        // Change collides with
        if (GetCollidesWith(ent) == COL_LAYER::kNonCollidable) {
          SetCollidesWith(ent, COL_LAYER::kCharController);
        }

        // Change belongs to
        if (GetBelongsTo(ent) == COL_LAYER::kNonCollidable) {
          SetBelongsTo(ent, COL_LAYER::kCamera);
        }

        log::info("    New info: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo);
        log::info("        with: {:0X}", collidable->broadPhaseHandle.collisionFilterInfo & 0x7F);
        log::info("     belongs: {:0X}", (collidable->broadPhaseHandle.collisionFilterInfo >> 16) & 0x7F);
      }
    }
  }
}
