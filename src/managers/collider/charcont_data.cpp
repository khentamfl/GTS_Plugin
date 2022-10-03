#include "managers/collider/charcont_data.hpp"
#include "util.hpp"
#include "hooks/RE.hpp"

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	CharContData::CharContData() {
		CharContData(nullptr);
	}
	CharContData::CharContData(bhkCharacterController* charCont) {
		this->Init(charCont);
		if (charCont) {
			this->UpdateColliders(charCont);
		}
	}

	CharContData::~CharContData () {
	}

	void CharContData::Init(bhkCharacterController* charCont) {
		if (this->charCont != charCont) {
			this->charCont = charCont;
			this->capsule_data.clear();
			this->convex_data.clear();
			this->list_data.clear();
			this->center = 0.0;

			if (charCont) {
				this->center = charCont->center;
				this->rotCenter = charCont->rotCenter;
			}
		}
	}

	void CharContData::UpdateColliders(bhkCharacterController* charCont) {
		if (this->charCont != charCont) {
			this->Init(charCont);
		}
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
					// I think these ones are objects that we collide with
					// Needs testing
					// for (auto body: hkpObject->bodies) {
					// 	auto const_shape = body->GetShape();
					// 	if (const_shape) {
					// 		hkpShape* shape = const_cast<hkpShape*>(const_shape);
					// 		this->AddShape(shape);
					// 	}
					// }

					// This one appears to be active during combat.
					// Maybe used for sword swing collision detection
					for (auto phantom: hkpObject->phantoms) {
						auto const_shape = phantom->GetShape();
						if (const_shape) {
							hkpShape* shape = const_cast<hkpShape*>(const_shape);
							this->AddShape(shape);
						}
					}

					// This is the actual shape
					if (hkpObject->shapePhantom) {
						auto const_shape = hkpObject->shapePhantom->GetShape();
						if (const_shape) {
							hkpShape* shape = const_cast<hkpShape*>(const_shape);
							this->AddShape(shape);
						}
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
						auto const_shape = hkpObject->m_character->GetShape();
						if (const_shape) {
							hkpShape* shape = const_cast<hkpShape*>(const_shape);
							this->AddShape(shape);
						}
					}
				}
			}
		}

		if (charCont) {
			// There are two collider shapes on the characontroller
			// One of these shapes should already be handelled in the above
			// calcs however there is anohter one attached to the characontroller
			// that we are not sure what it does.
			// Regardless we add it to the collider system now
			for (auto bShapeRef: charCont->shapes) {
				bhkShape* bShape = bShapeRef.get();
				if (bShape) {
					hkReferencedObject* refShape = bShape->referencedObject.get();
					if (refShape) {
						hkpShape* shape = static_cast<hkpShape*>(refShape);
						if (shape) {
							this->AddShape(shape);
						}
					}
				}
			}
		}
	}

	void CharContData::ApplyScale(const float& new_scale, const hkVector4& vec_scale) {
		for (auto &[key, data]: this->capsule_data) {
			data.ApplyScale(new_scale, vec_scale);
		}
		for (auto &[key, data]: this->convex_data) {
			data.ApplyScale(new_scale, vec_scale);
		}
		for (auto &[key, data]: this->list_data) {
			data.ApplyScale(new_scale, vec_scale);
		}
		if (this->charCont) {
			this->charCont->center = this->center * new_scale;
			this->charCont->rotCenter = this->rotCenter * vec_scale;
		}
	}

	void CharContData::ApplyPose(const hkVector4& origin, const float& new_scale) {
	}

	void CharContData::PruneColliders(Actor* actor) {
		for (auto i = this->capsule_data.begin(); i != this->capsule_data.end();) {
			auto& data = (*i);
			auto key = data.first;
			if (key->GetReferenceCount() == 1) {
				i = this->capsule_data.erase(i);
			} else {
				++i;
			}
		}
		for (auto i = this->convex_data.begin(); i != this->convex_data.end();) {
			auto& data = (*i);
			auto key = data.first;
			if (key->GetReferenceCount() == 1) {
				i = this->convex_data.erase(i);
			} else {
				++i;
			}
		}
		for (auto i = this->list_data.begin(); i != this->list_data.end();) {
			auto& data = (*i);
			auto key = data.first;
			if (key->GetReferenceCount() == 1) {
				i = this->list_data.erase(i);
			} else {
				++i;
			}
		}
	}

	void CharContData::AddShape(hkpShape* shape) {
		// Go over the shape and add what we can
		if (!shape) {
			return;
		}
		// log::info("Adding shape: {}", static_cast<std::uint32_t>(shape->type));
		switch (shape->type) {
			case hkpShapeType::kCapsule: {
				// log::info("Adding Capsule");
				auto actual_shape = static_cast<hkpCapsuleShape*>(shape);
				if (actual_shape) {
					this->AddCapsule(actual_shape);
				}
				break;
			}
			case hkpShapeType::kConvexVertices: {
				// log::info("Adding ConvVerts");
				auto actual_shape = static_cast<hkpConvexVerticesShape*>(shape);
				if (actual_shape) {
					this->AddConvexVerts(actual_shape);
				}
				break;
			}
			case hkpShapeType::kList: {
				// log::info("Adding List");
				const hkpListShape* container = static_cast<const hkpListShape*>(shape);
				if (container) {
					for (auto childShapeInfo: container->childInfo) {
						auto child_shape = childShapeInfo.shape;
						if (child_shape) {
							hkpShape* child_shape_mut = const_cast<hkpShape*>(child_shape);
							if (child_shape_mut) {
								this->AddShape(child_shape_mut);
							}
						}
					}

					auto container_mut = const_cast<hkpListShape*>(container);
					if (container_mut) {
						this->AddList(container_mut);
					}
				}
				break;
			}
		}
	}

	void CharContData::AddCapsule(hkpCapsuleShape* orig_capsule) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!orig_capsule) {
			return;
		}
		auto key = orig_capsule;
		auto result = this->capsule_data.try_emplace(key, orig_capsule);
		if (result.second) {
			(*result.first).second.SetOrigin(0.0, 0.0, -this->center);
		}
	}

	void CharContData::AddConvexVerts(hkpConvexVerticesShape* convex) {
		std::unique_lock lock(this->_lock);
		if (!convex) {
			return;
		}
		auto key = convex;
		auto result = this->convex_data.try_emplace(key, convex);
		if (result.second) {
			(*result.first).second.SetOrigin(0.0, 0.0, -this->center);
		}
	}

	void CharContData::AddList(hkpListShape* list) {
		std::unique_lock lock(this->_lock);
		if (!list) {
			return;
		}
		auto key = list;
		auto result = this->list_data.try_emplace(key, list);
		if (result.second) {
			(*result.first).second.SetOrigin(0.0, 0.0, -this->center);
		}
	}
}
