#include "raycast.h"
#include "util.h"

using namespace Gts;
using namespace RE;

namespace Gts {

	void RayCollector::add_filter(NiAVObject* obj) noexcept {
		object_filter.push_back(obj);
	}
	bool RayCollector::is_filtered(NiAVObject* obj) {
		while (obj) {
			for (auto object: this->object_filter) {
				if (obj == object) {
					return true;
				}
			}
			obj = obj->parent;
		}
		return false;
	}

	void RayCollector::AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) {
		log::info("Add Ray Hit");
		const hkpShape* shape = a_body.GetShape(); // Shape that was collided with

		if (shape) {
			auto ni_shape = shape->userData;
			if (ni_shape) {
				auto collision_node = ni_shape->AsBhkNiCollisionObject();
				if (collision_node) {
					auto av_node = collision_node->sceneObject;
					if (av_node) {
						if (is_filtered(av_node)) {
							log::info("Filtered");
							return;
						}
					} else {
						log::info("No scene node");
					}
				} else {
					log::info("Not a collision object????");
				}
			} else {
				log::info("No bhknode");
			}

			log::info("Adding result");
			HitResult hit_result;
			hit_result.shape = shape;
			hit_result.fraction = a_hitInfo.hitFraction;
			results.push_back(hit_result);
		}
	}

	NiPoint3 CastRay(Actor* actor, NiPoint3 in_origin, NiPoint3 direction, float length, bool& success) {

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
		log::info("Making ray picker");
		bhkPickData pick_data;

		NiPoint3 origin = unit_to_meter(in_origin);
		log::info("Ray Start: {},{},{}", origin.x, origin.y, origin.z);
		pick_data.rayInput.from = origin;

		log::info("Ray direction: {},{},{}", direction.x, direction.y, direction.z);
		NiPoint3 normed = direction / direction.Length();
		log::info("Normalised Ray direction: {},{},{}", normed.x, normed.y, normed.z);
		NiPoint3 end = unit_to_meter(origin + normed * length);
		log::info("Ray End: {},{},{}", end.x, end.y, end.z);
		// pick_data.rayInput.to = end;

		NiPoint3 delta = end - origin;
		log::info("Ray Delta: {},{},{}", delta.x, delta.y, delta.z);
		pick_data.ray = delta; // Length in each axis to travel

		RayCollector collector = RayCollector();
		collector.add_filter(actor->Get3D(false));
		collector.add_filter(actor->Get3D(true));
		// pick_data.rayHitCollectorA0 = &collector;
		pick_data.rayHitCollectorA8 = &collector;
		// pick_data.rayHitCollectorB0 = &collector;
		// pick_data.rayHitCollectorB8 = &collector;
		log::info("Picking ray");

		collision_world->PickObject(pick_data);
		log::info("Ray picked");
		float min_fraction = 1.0;
		success = !collector.results.empty();
		if (collector.results.size() > 0) {
			success = true;
			for (auto ray_result: collector.results) {
				if (ray_result.fraction < min_fraction) {
					min_fraction = ray_result.fraction;
				}
			}
			return meter_to_unit(origin + normed * length * min_fraction);
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
