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

	hkVector4 scale_relative(const hkVector4& input, const hkVector4& origin, const float& scale) {
		return (input - origin)*scale + origin;
	}

	void scale_relative_byref(hkVector4& input, const hkVector4& origin, const float& scale) {
		input = (input - origin)*scale + origin;
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

	void AddRigidBody(ColliderActorData* actor_data, hkpRigidBody* hkp_rigidbody) { // NOLINT
		if (hkp_rigidbody) {
			auto shape = hkp_rigidbody->GetShape();
			if (shape) {
				if (shape->type == hkpShapeType::kCapsule) {
					const hkpCapsuleShape* orig_capsule = static_cast<const hkpCapsuleShape*>(shape);
					actor_data->ReplaceCapsule(hkp_rigidbody, orig_capsule);
				}
			}
		}
	}

	void SearchColliders(Actor* actor, ColliderActorData* actor_data) {
		BSAnimationGraphManagerPtr animGraphManager;
		if (actor->GetAnimationGraphManager(animGraphManager)) {
			for (auto& graph : animGraphManager->graphs) {
				if (graph) {
					auto& character = graph->characterInstance;
					auto ragdollDriver = character.ragdollDriver.get();
					if (ragdollDriver) {
						auto ragdoll = ragdollDriver->ragdoll;
						if (ragdoll) {
							log::info("Got ragdoll");
							for (auto& rb: ragdoll->rigidBodies) {
								AddRigidBody(actor_data, rb);
							}
						}
					}
				}
			}
		}
	}

	void ScaleColliders(Actor* actor, ColliderActorData* actor_data, bool force_update) {
		const float EPSILON = 1e-3;
		if (!actor) {
			return;
		}
		float visual_scale = get_visual_scale(actor);
		float natural_scale = get_natural_scale(actor);
		float scale_factor = visual_scale/natural_scale;


		bool search_nodes = !actor_data->HasCapsuleData() || force_update;
		if (search_nodes) {
			SearchColliders(actor, actor_data);
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

		static bool doonce = false;
		if (!doonce) {
			doonce = true;
			log::info("Doing test scale on {}", actor->GetDisplayFullName());
			auto charController = actor->GetCharController();
			if (charController) {
				for (auto bhkShape: charController->shapes) {
					hkpShape* shape = static_cast<hkpShape*>(bhkShape->referencedObject.get());
					if (shape) {
						log::info("  - Scaling");
						scale_recursive("    ", shape, 10.0);
					}
				}
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
			if (!actor) {
				continue;
			}
			if (!actor->Is3DLoaded()) {
				continue;
			}
			ColliderActorData* actor_data = GetActorData(actor);
			if (actor_data) {
				float scale = get_visual_scale(actor)/get_natural_scale(actor);
				if (fabs(scale - 1.0) <= 1e-4) {
					continue;
				}
				auto model = actor->GetCurrent3D();
				if (model) {
					hkVector4 player_origin = hkVector4(model->world.translate * (*g_worldScale));
					for (auto &[key, capsule_data]: actor_data->GetCapsulesData()) {
						float scale = get_visual_scale(actor)/get_natural_scale(actor);
						auto& capsule = capsule_data.capsule;
						auto& rigidBody = capsule_data.rigidBody;
						if (capsule) {
							if (rigidBody) {
								// Translation
								scale_relative_byref(rigidBody->motion.motionState.transform.translation, player_origin, scale);

								// COM 0
								scale_relative_byref(rigidBody->motion.motionState.sweptTransform.centerOfMass0, player_origin, scale);

								// COM 1
								scale_relative_byref(rigidBody->motion.motionState.sweptTransform.centerOfMass1, player_origin, scale);
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

	CapsuleData::CapsuleData(const hkpCapsuleShape* orig_capsule, hkpRigidBody* rigidBody) {
		this->capsule = MakeCapsule();
		this->rigidBody = rigidBody;
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

	void ColliderActorData::ReplaceCapsule(hkpRigidBody* rigid_body, const hkpCapsuleShape* orig_capsule) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!orig_capsule) {
			return;
		}
		auto key = orig_capsule;
		try {
			auto& result = this->capsule_data.at(key);
		} catch (const std::out_of_range& oor) {
			CapsuleData new_data(orig_capsule, rigid_body);
			auto new_capsule = new_data.capsule;
			key = new_capsule;
			rigid_body->SetShape(new_capsule);
			this->capsule_data.try_emplace(key, std::move(new_data));
		}
	}
}
