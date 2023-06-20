#include "colliders/charcontroller.hpp"
#include "colliders/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

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
        if (static_cast<COL_LAYER>(collidable->broadPhaseHandle.collisionFilterInfo & 0x7F) == COL_LAYER::kCharController) {
          auto oldCollision = collidable->broadPhaseHandle.collisionFilterInfo;
          auto newCollision = collidable->broadPhaseHandle.collisionFilterInfo & 0xFFFFFFF8; // Clear old one
          newCollision = newCollision | static_cast<std::uint32_t>(COL_LAYER::kTerrain); // Set terrain
          if (oldCollision != newCollision) {
            log::info("Updaing collision from {:0X} to {:0X}", oldCollision, newCollision);
            collidable->broadPhaseHandle.collisionFilterInfo = newCollision;
          }
        }
      }
    }
  }

  void CharContData::EnableCollisions() {
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
        if (static_cast<COL_LAYER>(collidable->broadPhaseHandle.collisionFilterInfo & 0x7F) == COL_LAYER::kTerrain) {
          auto oldCollision = collidable->broadPhaseHandle.collisionFilterInfo;
          auto newCollision = collidable->broadPhaseHandle.collisionFilterInfo & 0xFFFFFFF8; // Clear old one
          newCollision = newCollision | static_cast<std::uint32_t>(COL_LAYER::kCharController); // Set terrain
          if (oldCollision != newCollision) {
            log::info("Updaing collision from {:0X} to {:0X}", oldCollision, newCollision);
            collidable->broadPhaseHandle.collisionFilterInfo = newCollision;
          }
        }
      }
    }
  }
}
