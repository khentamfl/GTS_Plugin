#include "node.h"

using namespace SKSE;
using namespace Gts;

namespace Gts {
	void walk_nodes(Actor* actor) {
		if (!actor->Is3DLoaded()) {
			return;
		}
		auto model = actor->Get3D();
		auto name = model->name;

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
					log::trace("Node {}", currentnode->name);
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

	NiAVObject* find_node(Actor* actor, string& node_name) {
		if (!actor->Is3DLoaded()) {
			return nullptr;
		}
		auto model = actor->Get3D();
		if (!model) {
			return nullptr;
		}
		auto node_lookup = model->GetObjectByName(node_name);
		if (node_lookup) {
			return node_lookup;
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
					if  (currentnode->name.c_str() == node_name) {
						return currentnode;
					}
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

		return nullptr;
	}

	void scale_hkpnodes(Actor* actor, float prev_scale, float new_scale) {
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
										log::info("Shape found: {} for {}", typeid(*shape).name(), currentnode->name.c_str());
										if (shape->type == hkpShapeType::kCapsule) {
											const hkpCapsuleShape* orig_capsule = static_cast<const hkpCapsuleShape*>(shape);
											// Here be dragons
											hkpCapsuleShape* capsule = const_cast<hkpCapsuleShape*>(orig_capsule);
											log::info("  - Capsule found: {}", typeid(*orig_capsule).name());
											float scale_factor = new_scale * 100.0 / prev_scale;
											hkVector4 vec_scale = hkVector4(scale_factor);
											capsule->vertexA = capsule->vertexA * vec_scale;
											capsule->vertexB = capsule->vertexB * vec_scale;
											capsule->radius *= scale_factor;
										}
									}
								}
							}
						}
					}
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

		return;
	}

	void clone_bound(Actor* actor) {
		// This is the bound on the NiExtraNodeData
		// This data is shared between all skeletons and this hopes to correct this
		auto model = actor->Get3D();
		if (model) {
			auto extra_bbx = model->GetExtraData("BBX");
			if (extra_bbx) {
				BSBound* bbx = static_cast<BSBound*>(extra_bbx);
				model->RemoveExtraData("BBX");
				auto new_extra_bbx = NiExtraData::Create<BSBound>();
				new_extra_bbx->name = bbx->name;
				new_extra_bbx->center = bbx->center;
				new_extra_bbx->extents = bbx->extents;
				//model->AddExtraData("BBX",  new_extra_bbx);
				model->InsertExtraData(new_extra_bbx);
			}
		}
	}

	BSBound* get_bound(Actor* actor) {
		// This is the bound on the NiExtraNodeData
		auto model = actor->Get3D();
		if (model) {
			auto extra_bbx = model->GetExtraData("BBX");
			if (extra_bbx) {
				BSBound* bbx = static_cast<BSBound*>(extra_bbx);
				return bbx;
			}
		}
		return nullptr;
	}

	NiAVObject* get_bumper(Actor* actor) {
		string node_name = "CharacterBumper";
		return find_node(actor, node_name);
	}
}
