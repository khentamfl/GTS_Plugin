#include "node.h"
#include <nlohmann/json.hpp>

using namespace SKSE;
using namespace Gts;
using json = nlohmann::json;

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

	NiAVObject* find_node(Actor* actor, std::string_view node_name, bool first_person) {
		if (!actor->Is3DLoaded()) {
			return nullptr;
		}
		auto model = actor->Get3D(first_person);
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
										log::trace("Shape found: {} for {}", typeid(*shape).name(), currentnode->name.c_str());
										if (shape->type == hkpShapeType::kCapsule) {
											const hkpCapsuleShape* orig_capsule = static_cast<const hkpCapsuleShape*>(shape);
											hkTransform identity;
											identity.rotation.col0 = hkVector4(1.0,0.0,0.0,0.0);
											identity.rotation.col1 = hkVector4(0.0,1.0,0.0,0.0);
											identity.rotation.col2 = hkVector4(0.0,0.0,1.0,0.0);
											identity.translation   = hkVector4(0.0,0.0,0.0,1.0);
											hkAabb out;
											orig_capsule->GetAabbImpl(identity, 1e-3, out);
											float min[4];
											float max[4];
											_mm_store_ps(&min[0], out.min.quad);
											_mm_store_ps(&max[0], out.max.quad);
											log::trace(" - Current bounds: {},{},{}<{},{},{}", min[0], min[1],min[2], max[0],max[1],max[2]);
											// Here be dragons
											hkpCapsuleShape* capsule = const_cast<hkpCapsuleShape*>(orig_capsule);
											log::trace("  - Capsule found: {}", typeid(*orig_capsule).name());
											float scale_factor = new_scale / prev_scale;
											hkVector4 vec_scale = hkVector4(scale_factor);
											capsule->vertexA = capsule->vertexA * vec_scale;
											capsule->vertexB = capsule->vertexB * vec_scale;
											capsule->radius *= scale_factor;

											capsule->GetAabbImpl(identity, 1e-3, out);
											_mm_store_ps(&min[0], out.min.quad);
											_mm_store_ps(&max[0], out.max.quad);
											log::trace(" - New bounds: {},{},{}<{},{},{}", min[0], min[1],min[2], max[0],max[1],max[2]);
											log::trace(" - pad28: {}", orig_capsule->pad28);
											log::trace(" - pad2C: {}", orig_capsule->pad2C);
											log::trace(" - float(pad28): {}", static_cast<float>(orig_capsule->pad28));
											log::trace(" - float(pad2C): {}", static_cast<float>(orig_capsule->pad2C));

											hkp_rigidbody->SetShape(capsule);
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
		auto model = actor->Get3D(false);
		if (model) {
			auto extra_bbx = model->GetExtraData("BBX");
			if (extra_bbx) {
				BSBound* bbx = static_cast<BSBound*>(extra_bbx);
				return bbx;
			}
		}
		auto model_first = actor->Get3D(true);
		if (model_first) {
			auto extra_bbx = model_first->GetExtraData("BBX");
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

	float get_hh_offset(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		float result = 0.0;
		auto shoes = actor->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kFeet);
		if (shoes) {
			auto actor_base = actor->GetActorBase();
			if (actor_base) {
				auto race = actor_base->GetRace();
				auto arma = shoes->GetArmorAddonByMask(race, BGSBipedObjectForm::BipedObjectSlot::kFeet);
				if (arma) {
					actor->VisitArmorAddon(shoes, arma, [&](bool isFP, NiAVObject& armorNode) {
						// Legacy method
						auto hh_legacy_data = armorNode.GetExtraData("HH_OFFSET");
						if (hh_legacy_data) {
							NiFloatExtraData* hh_float_data = static_cast<NiFloatExtraData*>(hh_legacy_data);
							if (hh_float_data) {
								result = hh_float_data->value;
								return;
							}
						}
						// Modern method
						auto sdta_data = armorNode.GetExtraData("SDTA");
						if (sdta_data) {
							NiStringExtraData* extra_offset_data = static_cast<NiStringExtraData*>(sdta_data);
							if (extra_offset_data) {
								std::string data = extra_offset_data->value;
								// Formatted as json of the form
								// [{"name":"NPC","pos":[0, 0,%f]}]"
								auto json = json::parse(data);
								if (json) {
									if (json["name"] == "NPC") {
										auto pos = json["pos"];
										if (pos.is_array() && pos.size() == 3) {
											result = pos[2].get<float>();
										}
									}
								}
							}
						}
					});
				}
			}
		}
		return result;
	}
}
