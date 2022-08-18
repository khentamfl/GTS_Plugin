#include "managers/collider.hpp"
#include "scale/scale.hpp"

#include "util.hpp"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace RE {
	// DRAGON SLAYING

	const hkClass* hkReferencedObject::GetClassType() const // 01 - { return 0; }
	{
	}
	void hkReferencedObject::CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const // 02
	{

	}

	hkpShape::~hkpShape()  // 00
	{
	}

	// add
	float hkpShape::GetMaximumProjection(const hkVector4& a_direction) const // 03
	{

	}
	const hkpShapeContainer* hkpShape::GetContainer() const // 04 - { return 0; }
	{
	}
	bool hkpShape::IsConvex() const // 05 - { return false; }
	{
	}
	std::int32_t hkpShape::CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const// 06 - { return -1; }
	{

	}
	hkVector4Comparison hkpShape::CastRayBundleImpl(const hkpShapeRayBundleCastInput& a_input, hkpShapeRayBundleCastOutput& a_output, const hkVector4Comparison& a_mask) const// 0A
	{
	}

	hkpSphereRepShape::~hkpSphereRepShape()  // 00
	{

	}

	hkpConvexShape::~hkpConvexShape() {

	}

	float hkpConvexShape::GetMaximumProjection(const hkVector4& a_direction) const { // 03

	}
	bool hkpConvexShape::IsConvex() const { // 05 - { return true; }

	}
	void hkpConvexShape::CastRayWithCollectorImpl(const hkpShapeRayCastInput& a_input, const hkpCdBody& a_cdBody, hkpRayHitCollector& a_collector) const // 09
	{

	}
	std::uint32_t hkpConvexShape::Unk_10(void) // 10 - { return 2; }
	{
	}
	void hkpConvexShape::Unk_11(void) // 11
	{
	}

	// VTABLE_hkpCapsuleShape
	hkpCapsuleShape::~hkpCapsuleShape()  // 00
	{

	}

	// override (hkpConvexShape)
	void hkpCapsuleShape::CalcContentStatistics(hkStatisticsCollector* a_collector, const hkClass* a_class) const // 02
	{

	}
	std::int32_t hkpCapsuleShape::CalcSizeForSpu(const CalcSizeForSpuInput& a_input, std::int32_t a_spuBufferSizeLeft) const // 06 - { return 56; }
	{

	}
	void hkpCapsuleShape::GetAabbImpl(const hkTransform& a_localToWorld, float a_tolerance, hkAabb& a_out) const // 07
	{

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

	}
	const hkpSphere* hkpCapsuleShape::GetCollisionSpheresImpl(hkSphere* a_sphereBuffer)// 0C
	{

	}
	void hkpCapsuleShape::GetCentreImpl(hkVector4& a_centreOut) // 0D
	{

	}
	void hkpCapsuleShape::Unk_0E(void) // 0E
	{

	}
	void hkpCapsuleShape::Unk_0F(void) // 0F
	{

	}
	void hkpCapsuleShape::Unk_11(void) // 11
	{

	}
}

namespace {
	void RescaleCapsule(hkpCapsuleShape* capsule, CapsuleData* data, float& scale_factor, hkVector4& vec_scale) {
		const float EPSILON = 1e-3;

		float expected_radius = data->radius * scale_factor;
		if(fabs(capsule->radius - expected_radius) > EPSILON) {
			capsule->vertexA = data->start * vec_scale;
			capsule->vertexB = data->end * vec_scale;
			log::info("Old Radius: {}", capsule->radius);
			capsule->radius = expected_radius;
			log::info("New Radius: {}", capsule->radius);

			// Dragons
			hkpCapsuleShape* ctd_me_capsule = new hkpCapsuleShape();
		}
	}

	void AddNode(ActorData* actor_data, NiAVObject* currentnode) { // NOLINT
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
								// Here be dragons
								hkpCapsuleShape* capsule = const_cast<hkpCapsuleShape*>(orig_capsule);
								actor_data->GetCapsuleData(capsule); // Will insert the data
							}
						}
					}
				}
			}
		}
	}

	void SearchColliders(NiAVObject* root, ActorData* actor_data) {
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

	void ScaleColliders(Actor* actor, ActorData* actor_data) {
		const float EPSILON = 1e-3;
		if (!actor->Is3DLoaded()) {
			return;
		}
		float visual_scale = get_visual_scale(actor);
		float natural_scale = get_natural_scale(actor);
		float scale_factor = visual_scale/natural_scale;

		if (fabs(actor_data->last_scale - scale_factor) <= EPSILON) {
			return;
		}
		log::info("Updating: {}", actor->GetDisplayFullName());
		actor_data->last_scale = scale_factor;

		bool search_nodes = !actor_data->HasCapsuleData();
		if (search_nodes) {
			for (auto person: {true, false} ) {
				auto model = actor->Get3D(person);
				if (model) {
					SearchColliders(model, actor_data);
				}
			}
		}

		hkVector4 vec_scale = hkVector4(scale_factor, scale_factor, scale_factor, 1.0);
		for (auto &[capsule, capsule_data]: actor_data->GetCapsulesData()) {
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
		for (auto actor: find_actors()) {
			ActorData* actor_data = GetActorData(actor);
			if (actor_data) {
				ScaleColliders(actor, actor_data);
			}
		}
	}

	ActorData* ColliderManager::GetActorData(Actor* actor) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!actor) {
			return nullptr;
		}
		auto key = actor;
		ActorData* result = nullptr;
		try {
			result = &this->actor_data.at(key);
		} catch (const std::out_of_range& oor) {
			this->actor_data.try_emplace(key);
			try {
				result = &this->actor_data.at(key);
			} catch (const std::out_of_range& oor) {
				result = nullptr;
			}
		}
		return result;
	}

	CapsuleData* ActorData::GetCapsuleData(hkpCapsuleShape* capsule) { // NOLINT
		std::unique_lock lock(this->_lock);
		if (!capsule) {
			return nullptr;
		}
		auto key = capsule;
		CapsuleData* result = nullptr;
		try {
			result = &this->capsule_data.at(key);
		} catch (const std::out_of_range& oor) {
			CapsuleData new_data;
			new_data.start = capsule->vertexA;
			new_data.end = capsule->vertexB;
			new_data.radius = capsule->radius;
			this->capsule_data.try_emplace(key, new_data);
			try {
				result = &this->capsule_data.at(key);
			} catch (const std::out_of_range& oor) {
				result = nullptr;
			}
		}
		return result;
	}
}
