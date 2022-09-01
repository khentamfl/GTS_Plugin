#include "managers/collider.hpp"
#include "managers/GtsManager.hpp"
#include "scale/scale.hpp"

#include "util.hpp"


using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {
	const std::uint32_t CAPSULE_MARKER = 494148758080886;
	hkpCapsuleShape* MakeCapsule() {
		hkpCapsuleShape* x = new hkpCapsuleShape();
		// First value is the vtable pointer
		REL::Relocation<std::uintptr_t> vptr(reinterpret_cast<std::uintptr_t>(x));

		// This is the skyrim vtable location
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};

		// Make it use skyrims vtable not our hacky one
		safe_write(vptr.address(), vtable.address());

		x->pad28 = CAPSULE_MARKER;

		return x;
	}


	void scale_recursive(std::string_view prefix, hkpShape* shape, float scale) {
		if (!shape) {
			return;
		}
		if (shape->type == hkpShapeType::kCapsule) {
			log::debug("{}- Capsule (Scaling by {})", prefix, scale);
			auto actual_shape = static_cast<hkpCapsuleShape*>(shape);
			actual_shape->radius *= scale;
			actual_shape->vertexA = actual_shape->vertexA * hkVector4(scale);
			actual_shape->vertexB = actual_shape->vertexB * hkVector4(scale);
		} else if (shape->type == hkpShapeType::kList) {
			log::debug("{}- List (checking children)", prefix);
			auto container = static_cast<hkpListShape*>(shape);
			auto key = container->GetFirstKey();
			while (key != HK_INVALID_SHAPE_KEY) {
				auto buffer = hkpShapeBuffer();
				auto child_shape = container->GetChildShape(key, buffer);
				if (child_shape) {
					hkpShape* dragon_child = const_cast<hkpShape*>(child_shape);
					scale_recursive(std::string(prefix) + "  ", dragon_child, scale);
				}
				key = container->GetNextKey(key);
			}
		} else if (shape->type == hkpShapeType::kBVTree) {
			log::debug("{}- Tree (checking children)", prefix);
			auto actual_shape = static_cast<hkpBvTreeShape*>(shape);
			const hkpShapeContainer* container = actual_shape->GetContainer();
			auto key = container->GetFirstKey();
			while (key != HK_INVALID_SHAPE_KEY) {
				auto buffer = hkpShapeBuffer();
				auto child_shape = container->GetChildShape(key, buffer);
				if (child_shape) {
					hkpShape* dragon_child = const_cast<hkpShape*>(child_shape);
					scale_recursive(std::string(prefix) + "  ", dragon_child, scale);
				}
				key = container->GetNextKey(key);
			}
		} else {
			log::debug("{}- Shape (of type {})", prefix, static_cast<int>(shape->type));
		}
	}
}

namespace {
	void RescaleCapsule(hkpCapsuleShape* capsule, CapsuleData* data, float& scale_factor, hkVector4& vec_scale) {
		const float EPSILON = 1e-3;

		float expected_radius = data->radius * scale_factor;
		if(fabs(capsule->radius - expected_radius) > EPSILON) {
			capsule->vertexA = data->start * vec_scale;
			capsule->vertexB = data->end * vec_scale;
			capsule->radius = expected_radius;
		}
	}

	void AddNode(ColliderActorData* actor_data, NiAVObject* currentnode) { // NOLINT
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
								// if (orig_capsule->pad28 == CAPSULE_MARKER) {
								// 	log::error("Capsule is a lost one of ours");
								// }
								actor_data->ReplaceCapsule(hkp_rigidbody, orig_capsule, currentnode);
							}
						}
					}
				}
			}
		}
	}

	void SearchColliders(NiAVObject* root, ColliderActorData* actor_data) {
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

	void ScaleColliders(Actor* actor, ColliderActorData* actor_data, bool force_update) {
		const float EPSILON = 1e-3;
		float visual_scale = get_visual_scale(actor);
		float natural_scale = get_natural_scale(actor);
		float scale_factor = visual_scale/natural_scale;


		bool search_nodes = !actor_data->HasCapsuleData() || force_update;
		if (search_nodes) {
			log::info("{}: Searching for capsules", actor->GetDisplayFullName());
			for (auto person: {true, false} ) {
				auto model = actor->Get3D(person);
				if (model) {
					SearchColliders(model, actor_data);
				}
			}
		}

		if ((fabs(actor_data->last_scale - scale_factor) <= EPSILON) &&  !force_update) {
			return;
		}

		actor_data->last_scale = scale_factor;

		hkVector4 vec_scale = hkVector4(scale_factor, scale_factor, scale_factor, 1.0);
		for (auto &[key, capsule_data]: actor_data->GetCapsulesData()) {
			auto& capsule = capsule_data.capsule;
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
		auto playerCharacter = PlayerCharacter::GetSingleton();
		auto cell = playerCharacter->GetParentCell();
		if (!cell) {
			return;
		}

		if (cell != this->previous_cell) {
			this->FlagReset();
			this->previous_cell = cell;
		}

		std::uint64_t last_reset_frame = this->last_reset_frame.load();


		auto actors = find_actors();
		for (auto actor: actors) {
			if (actor->Is3DLoaded()) {
				ColliderActorData* actor_data = GetActorData(actor);
				if (actor_data) {
					bool force_reset = actor_data->last_update_frame.exchange(last_reset_frame) < last_reset_frame;
					ScaleColliders(actor, actor_data, force_reset);
				}
			}
		}

		auto itr = this->actor_data.begin();
		while (itr != this->actor_data.end())
		{
			bool found = (std::find(actors.begin(), actors.end(), itr->first) != actors.end());
			if (!found) {
				itr = this->actor_data.erase(itr);
			} else {
				itr++;
			}
		}

		// static bool doonce = true;
		// if (doonce) {
		// 	doonce = false;
		// 	log::info("==Experiment");
		// 	log::info("==END Experiment");
		// }
	}

	void ColliderManager::UpdateHavok() {
		auto actors = find_actors();
		auto& manager = GtsManager::GetSingleton();
		for (auto actor: actors) {
			if (actor->Is3DLoaded()) {
				ColliderActorData* actor_data = GetActorData(actor);
				if (actor_data) {
					for (auto &[key, capsule_data]: actor_data->GetCapsulesData()) {
						float scale = get_visual_scale(actor)/get_natural_scale(actor);
						auto& capsule = capsule_data.capsule;
						auto& rigidBody = capsule_data.rigidBody;
						auto& node = capsule_data.node;
						if (capsule) {
							if (rigidBody) {
								if (node) {
									{
										hkVector4 position = rigidBody->motion.motionState.transform.translation;
										log::info("Pre translation: {},{},{}",position.quad.m128_f32[0], position.quad.m128_f32[1], position.quad.m128_f32[2]);
									}

									NiPoint3 world_pos = node->world.translate;
									hkVector4 new_translation = hkVector4(world_pos * (*g_worldScale));
									rigidBody->motion.motionState.transform.translation = new_translation;

									{
										hkVector4 position = rigidBody->motion.motionState.transform.translation;
										log::info("Post translation: {},{},{}",position.quad.m128_f32[0], position.quad.m128_f32[1], position.quad.m128_f32[2]);
									}
									{
										hkVector4 position = rigidBody->motion.motionState.sweptTransform.centerOfMass0;
										log::info("centerOfMass0: {},{},{}",position.quad.m128_f32[0], position.quad.m128_f32[1], position.quad.m128_f32[2]);
									}
									{
										hkVector4 position = rigidBody->motion.motionState.sweptTransform.centerOfMass1;
										log::info("centerOfMass1: {},{},{}",position.quad.m128_f32[0], position.quad.m128_f32[1], position.quad.m128_f32[2]);
									}
									{
										hkVector4 position = rigidBody->motion.motionState.sweptTransform.centerOfMassLocal;
										log::info("centerOfMassLocal: {},{},{}",position.quad.m128_f32[0], position.quad.m128_f32[1], position.quad.m128_f32[2]);
									}

									{
										if (rigidBody->motion.mavedMotion) {
											hkVector4 position = rigidBody->motion.mavedMotion->motionState.transform.translation;
											log::info("mavedMotion.translation: {},{},{}",position.quad.m128_f32[0], position.quad.m128_f32[1], position.quad.m128_f32[2]);
										}
									}
								}
							}
						}
					}
				}
			}
		}

	}

	ColliderActorData::ColliderActorData(Actor* actor) {
		this->Reset();
		this->form_id = actor->formID;
	}

	ColliderActorData* ColliderManager::GetActorData(Actor* actor) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!actor) {
			return nullptr;
		}
		auto key = actor;
		ColliderActorData* result = nullptr;
		try {
			result = &this->actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			this->actor_data.try_emplace(key, actor);
			try {
				result = &this->actor_data.at(key);
			} catch (const std::out_of_range& oor) {
				result = nullptr;
			}
		}
		return result;
	}

	ColliderActorData::~ColliderActorData() {
		this->Reset();
	}

	CapsuleData::CapsuleData(const hkpCapsuleShape* orig_capsule, hkpRigidBody* rigidBody, NiAVObject* node) {
		this->capsule = MakeCapsule();
		this->rigidBody = rigidBody;
		this->node = node;
		this->capsule->radius = orig_capsule->radius;
		this->capsule->vertexA = orig_capsule->vertexA;
		this->capsule->vertexB = orig_capsule->vertexB;
		this->capsule->userData = orig_capsule->userData;
		this->capsule->type = orig_capsule->type;

		this->start = orig_capsule->vertexA;
		this->end = orig_capsule->vertexB;
		this->radius = orig_capsule->radius;
	}

	CapsuleData::~CapsuleData () {
		this->capsule->RemoveReference();
	}

	void ColliderActorData::ReplaceCapsule(hkpRigidBody* rigid_body, const hkpCapsuleShape* orig_capsule, NiAVObject* node) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!orig_capsule) {
			return;
		}
		auto key = orig_capsule;
		try {
			auto& result = this->capsule_data.at(key);
		} catch (const std::out_of_range& oor) {
			CapsuleData new_data(orig_capsule, rigid_body, node);
			auto new_capsule = new_data.capsule;
			key = new_capsule;
			rigid_body->SetShape(new_capsule);
			this->capsule_data.try_emplace(key, std::move(new_data));
		}
	}
}
