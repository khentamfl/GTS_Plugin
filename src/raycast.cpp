#include "raycast.h"

using namespace Gts;
using namespace RE;

namespace Gts {

	void RayCollector::add_filter(NiAVObject* obj) noexcept {
		object_filter.push_back(obj);
	}

	void RayCollector::AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) {
		log::info("Add Ray Hit");
		const hkpShape* shape = a_body.GetShape(); // Shape that was collided with
		const hkpShape* first_shape = shape; // Used for checking against filter
		const hkpShape* last_shape = shape; // Used for checking against filter
		hkpShapeKey key = a_body.shapeKey;
		if (shape) {
			switch (shape->type) {
				case hkpShapeType::kList:
				{
					log::info("Listshape");
					const hkpListShape* container = static_cast<const hkpListShape*>(shape);
					// Get collision shape

					if (key == HK_INVALID_SHAPE_KEY) {
						// If invalid just use first
						key = container->GetFirstKey();
					}
					auto buffer = hkpShapeBuffer();
					shape = container->GetChildShape(key, buffer);
					// Get first shape
					first_shape = container->GetChildShape(container->GetFirstKey(), buffer);
					// Get last shape
					last_shape = shape;
					while (key != HK_INVALID_SHAPE_KEY) {
						last_shape = container->GetChildShape(key, buffer);
						key = container->GetNextKey(key);
					}
				}
				break;
				default:
					log::info("Another shape");
			}
		}

		if (shape) {
			if (object_filter.size() > 0)
			{
				for (auto object: this->object_filter) {
					NiObject* collision_object = static_cast<NiObject*>(object);
					NiObject* nishape = static_cast<NiObject*>(shape->userData);
					NiObject* first_nishape = static_cast<NiObject*>(first_shape->userData);
					NiObject* last_nishape = static_cast<NiObject*>(last_shape->userData);
					if ((collision_object  == nishape) || (collision_object  == first_nishape) || (collision_object  == last_nishape)) {
						log::info("Filtered");
						return;
					}
				}
			}
			log::info("Adding result");
			HitResult hit_result;
			hit_result.shape = shape;
			hit_result.fraction = a_hitInfo.hitFraction;
			results.push_back(hit_result);
		}
	}

	NiPoint3 CastRay(Actor* actor, NiPoint3 origin, NiPoint3 direction, float length, bool& success) {
		success = false;
		if (!actor) {
			log::info("No Actor");
			return NiPoint3();
		}
		auto cell = actor->GetParentCell();
		auto collision_world = cell->GetbhkWorld();
		if (!collision_world) {
			log::info("No world");
			return NiPoint3();
		}
		bhkPickData pick_data;

		pick_data.rayInput.from = origin;

		NiPoint3 normed = direction / direction.Length();
		pick_data.rayInput.to = origin + normed * length;

		pick_data.ray = pick_data.rayInput.to - pick_data.rayInput.from; // Length in each axis to travel

		RayCollector collector = RayCollector();
		collector.add_filter(actor->Get3D(false));
		collector.add_filter(actor->Get3D(true));
		pick_data.rayHitCollectorA0 = &collector;
		pick_data.rayHitCollectorA8 = &collector;
		pick_data.rayHitCollectorB0 = &collector;
		pick_data.rayHitCollectorB8 = &collector;

		collision_world->PickObject(pick_data);
		float min_fraction = 1.0;
		success = !collector.results.empty();
		if (collector.results.size() > 0) {
			success = true;
			for (auto ray_result: collector.results) {
				if (ray_result.fraction < min_fraction) {
					min_fraction = ray_result.fraction;
				}
			}
			return origin + normed * length * min_fraction;
		} else {
			log::info("No result");
			return NiPoint3();
		}
	}
}


void hkpClosestRayHitCollector::AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) {
	log::info("Dummy AddRayHit");
	// Dummy
}

hkpClosestRayHitCollector::~hkpClosestRayHitCollector() {
	log::info("Dummy Destructor");
	// Dummy
}
