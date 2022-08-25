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
}

namespace RE {
	// DRAGON SLAYING

	hkpShape::~hkpShape()  // 00
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x00;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpShape*)> func(result);
		func(this);
	}

	// add
	float hkpShape::GetMaximumProjection(const hkVector4& a_direction) const // 03
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x03;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpShape::GetMaximumProjection)> func(result);
		return func(this, a_direction);
	}
	const hkpShapeContainer* hkpShape::GetContainer() const // 04 - { return 0; }
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x04;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpShape::GetContainer)> func(result);
		return func(this);
	}
	bool hkpShape::IsConvex() const // 05 - { return false; }
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x05;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpShape::IsConvex)> func(result);
		return func(this);
	}
	std::int32_t hkpShape::CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const// 06 - { return -1; }
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x06;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpShape::CalcSizeForSpu)> func(result);
		return func(this, a_input, a_spuBufferSizeLeft);
	}
	hkVector4Comparison hkpShape::CastRayBundleImpl(const hkpShapeRayBundleCastInput& a_input, hkpShapeRayBundleCastOutput& a_output, const hkVector4Comparison& a_mask) const// 0A
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x0A;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpShape::CastRayBundleImpl)> func(result);
		return func(this, a_input, a_output, a_mask);
	}

	hkpSphereRepShape::~hkpSphereRepShape()  // 00
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpShape[0]};
		const auto a_idx = 0x00;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpSphereRepShape*)> func(result);
		func(this);
	}

	hkpConvexShape::~hkpConvexShape() {
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		const auto a_idx = 0x00;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpConvexShape*)> func(result);
		func(this);
	}

	float hkpConvexShape::GetMaximumProjection(const hkVector4& a_direction) const { // 03
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		const auto a_idx = 0x03;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpConvexShape::GetMaximumProjection)> func(result);
		return func(this, a_direction);
	}
	bool hkpConvexShape::IsConvex() const { // 05 - { return true; }
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		const auto a_idx = 0x05;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpConvexShape::IsConvex)> func(result);
		return func(this);
	}
	void hkpConvexShape::CastRayWithCollectorImpl(const hkpShapeRayCastInput& a_input, const hkpCdBody& a_cdBody, hkpRayHitCollector& a_collector) const // 09
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		const auto a_idx = 0x09;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpConvexShape::CastRayWithCollectorImpl)> func(result);
		return func(this, a_input, a_cdBody, a_collector);
	}
	std::uint32_t hkpConvexShape::Unk_10(void) // 10 - { return 2; }
	{
		// REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		// const auto a_idx = 0x10;
		// const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		// const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		// REL::Relocation<decltype(&hkpConvexShape::Unk_10)> func(result);
		// return func(this);
		return 0;
	}
	void hkpConvexShape::Unk_11(void) // 11
	{
		// REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpConvexShape[0]};
		// const auto a_idx = 0x11;
		// const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		// const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		// REL::Relocation<decltype(&hkpConvexShape::Unk_11)> func(result);
		// func(this);
	}

	// VTABLE_hkpCapsuleShape
	hkpCapsuleShape::~hkpCapsuleShape()  // 00
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x00;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<void(hkpCapsuleShape*)> func(result);
		func(this);
	}

	// override (hkpConvexShape)
	void hkpCapsuleShape::CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const // 02
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x02;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::CalcContentStatistics)> func(result);
		return func(this, a_collector, a_class);
	}
	std::int32_t hkpCapsuleShape::CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const // 06 - { return 56; }
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x06;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::CalcSizeForSpu)> func(result);
		return func(this, a_input, a_spuBufferSizeLeft);
	}
	void hkpCapsuleShape::GetAabbImpl(const hkTransform& a_localToWorld, float a_tolerance, hkAabb& a_out) const // 07
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x07;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::GetAabbImpl)> func(result);
		func(this, a_localToWorld, a_tolerance, a_out);
	}
	bool hkpCapsuleShape::CastRayImpl(const hkpShapeRayCastInput& a_input, hkpShapeRayCastOutput& a_output) const // 08
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x08;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::CastRayImpl)> func(result);
		return func(this, a_input, a_output);
	}
	std::int32_t hkpCapsuleShape::GetNumCollisionSpheresImpl() // 0B - { return 8; }
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x0B;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::GetNumCollisionSpheresImpl)> func(result);
		return func(this);
	}
	const hkpSphere* hkpCapsuleShape::GetCollisionSpheresImpl(hkSphere* a_sphereBuffer)// 0C
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x0C;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::GetCollisionSpheresImpl)> func(result);
		return func(this, a_sphereBuffer);
	}
	void hkpCapsuleShape::GetCentreImpl(hkVector4& a_centreOut) // 0D
	{
		REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		const auto a_idx = 0x0D;
		const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		REL::Relocation<decltype(&hkpCapsuleShape::GetCentreImpl)> func(result);
		func(this, a_centreOut);
	}
	void hkpCapsuleShape::Unk_0E(void) // 0E
	{
		// REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		// const auto a_idx = 0x0E;
		// const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		// const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		// REL::Relocation<decltype(&hkpCapsuleShape::Unk_0E)> func(result);
		// func(this);
	}
	void hkpCapsuleShape::Unk_0F(void) // 0F
	{
		// REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		// const auto a_idx = 0x0F;
		// const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		// const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		// REL::Relocation<decltype(&hkpCapsuleShape::Unk_0F)> func(result);
		// func(this);
	}
	void hkpCapsuleShape::Unk_11(void) // 11
	{
		// REL::Relocation<std::uintptr_t> vtable{VTABLE_hkpCapsuleShape[0]};
		// const auto a_idx = 0x11;
		// const auto addr = vtable.address() + (sizeof(void *) * a_idx);
		// const auto result = *reinterpret_cast<std::uintptr_t *>(addr);
		// REL::Relocation<decltype(&hkpCapsuleShape::Unk_11)> func(result);
		// func(this);
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
								actor_data->ReplaceCapsule(hkp_rigidbody, orig_capsule);
							}
							//  else if (actor_data->form_id == 0x14) {
							// 	std::string name = currentnode->name.c_str();
							// 	log::info("Node {} has type {}", name, static_cast<int>(shape->type));
							// }
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

		static bool doonce = true;
		if (doonce) {
			doonce = false;
			log::info("==Experiment");
			float factor = 100.0;
			for (auto actor: find_team_player()) {
				if (actor) {
					log::info(" + {}", actor->GetDisplayFullName());
					auto controller = actor->GetCharController();
					if (controller) {
						log::info("    - Got CharController");
						bhkCharProxyController* proxy_controller = skyrim_cast<bhkCharProxyController*>(controller);
						if (proxy_controller) {
							log::info("    - Got ProxyController");
							auto hkp_char_proxy = proxy_controller->GetCharacterProxy();
							if (hkp_char_proxy) {
								log::info("    - Got hkp proxy");

								auto phantom = hkp_char_proxy->shapePhantom;
								if (phantom) {
									auto shape = phantom->GetShape();
									if (shape) {
										log::info("    - Controller Phantom is of type: {}", static_cast<int>(shape->type));
										if (shape->type == hkpShapeType::kList) {
											const hkpListShape* container = static_cast<const hkpListShape*>(shape);

											if (container) {
												auto key = container->GetFirstKey();
												while (key != HK_INVALID_SHAPE_KEY) {
													auto buffer = hkpShapeBuffer();
													auto child_shape = container->GetChildShape(key, buffer);
													if (child_shape) {
														log::info("      - ChildShape is of type: {}", static_cast<int>(child_shape->type));
														if (child_shape->type == hkpShapeType::kCapsule) {
															log::info("        - Changing the capsule");
															const hkpCapsuleShape* orig_capsule = static_cast<const hkpCapsuleShape*>(child_shape);
															hkpCapsuleShape* dragon = const_cast<hkpCapsuleShape*>(orig_capsule);
															dragon->radius *= factor;
															dragon->vertexA = dragon->vertexA * factor;
															dragon->vertexB = dragon->vertexB * factor;
														}
													}
													key = container->GetNextKey(key);
												}
												log::info("    - Changing the List group size");
												hkpListShape* dragon_container = const_cast<hkpListShape*>(container);
												dragon_container->aabbHalfExtents = dragon_container->aabbHalfExtents * hkVector4(factor);
											}
										}
									}
								}
							}
						}
					}
				}
			}
			log::info("==END Experiment");
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

	CapsuleData::CapsuleData(const hkpCapsuleShape* orig_capsule) {
		this->capsule = MakeCapsule();
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
			CapsuleData new_data(orig_capsule);
			auto new_capsule = new_data.capsule;
			key = new_capsule;
			rigid_body->SetShape(new_capsule);
			this->capsule_data.try_emplace(key, std::move(new_data));

		}
	}
}
