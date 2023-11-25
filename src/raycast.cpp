#include "raycast.hpp"

using namespace Gts;
using namespace RE;

namespace Gts {

  void RayCollector::add_group_filter(COL_LAYER group) noexcept {
    groups.push_back(group);
  }

	void RayCollector::add_filter(NiObject* obj) noexcept {
		object_filter.push_back(obj);
	}
	bool RayCollector::is_filtered(NiObject* obj) {
		for (auto object: this->object_filter) {
			if (obj == object) {
				return true;
			}
		}
		return false;
	}
	bool RayCollector::is_filtered_av(NiAVObject* obj) {
		while (obj) {
			if (!is_filtered(obj)) {
				obj = obj->parent;
			} else {
				return true;
			}
		}
		return false;
	}

	void RayCollector::AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) {
		const hkpShape* shape = a_body.GetShape(); // Shape that was collided with

		// Search for top level shape
		const hkpCdBody* top_body = a_body.parent;
		while (top_body) {
			if (top_body->shape) {
				shape = top_body->shape;
			}
			top_body = top_body->parent;
		}

		if (shape) {
			auto ni_shape = shape->userData;
			if (ni_shape) {
        auto filter_info = ni_shape->filterInfo;
        COL_LAYER collision_layer = static_cast<COL_LAYER>(filter_info & 0x7F);
        if (! groups.empty()) {
          bool found = false;
          for (auto group: groups) {
            if (group == collision_layer) {
              found = true;
              break;
            }
          }
          if (!found) {
            return;
          }
        }
				if (is_filtered(ni_shape)) {
					return;
				}

				auto collision_node = ni_shape->AsBhkNiCollisionObject();
				if (collision_node) {
					auto av_node = collision_node->sceneObject;
					if (av_node) {
						if (is_filtered_av(av_node)) {
							return;
						}
					}
				}

				auto ni_node = ni_shape->AsNode();
				if (ni_node) {
					if (is_filtered_av(ni_node)) {
						return;
					}
				}
			}

			HitResult hit_result;
			hit_result.shape = shape;
			hit_result.fraction = a_hitInfo.hitFraction;
			results.push_back(hit_result);
		}
	}

	NiPoint3 CastRay(TESObjectREFR* ref, const NiPoint3& in_origin, const NiPoint3& direction, const float& unit_length, const std::vector<COL_LAYER>& groups, bool& success) {
		float length = unit_to_meter(unit_length);
		success = false;
		if (!ref) {
			return NiPoint3();
		}
		auto cell = ref->GetParentCell();
		if (!cell) {
			return NiPoint3();
		}
		auto collision_world = cell->GetbhkWorld();
		if (!collision_world) {
			return NiPoint3();
		}
		bhkPickData pick_data;

		NiPoint3 origin = unit_to_meter(in_origin);
		pick_data.rayInput.from = origin;

		NiPoint3 normed = direction / direction.Length();
		NiPoint3 end = origin + normed * length;
		pick_data.rayInput.to = end;

		NiPoint3 delta = end - origin;
		pick_data.ray = delta; // Length in each axis to travel

		RayCollector collector = RayCollector();
		collector.add_filter(ref->Get3D1(false));
		collector.add_filter(ref->Get3D1(true));
    for (auto group: groups) {
      collector.add_group_filter(group);
    }
		// pick_data.rayHitCollectorA0 = &collector;
		pick_data.rayHitCollectorA8 = &collector;
		// pick_data.rayHitCollectorB0 = &collector;
		// pick_data.rayHitCollectorB8 = &collector;

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
			return meter_to_unit(origin + normed * length * min_fraction);
		} else {
			return NiPoint3();
		}
	}

  NiPoint3 CastRay(TESObjectREFR* ref, const NiPoint3& origin, const NiPoint3& direction, const float& length, bool& success) {
    std::vector<COL_LAYER> groups = {};
    return CastRay(ref, origin, direction, length, groups, success);
  }
}


void hkpClosestRayHitCollector::AddRayHit(const hkpCdBody& a_body, const hkpShapeRayCastCollectorOutput& a_hitInfo) {
	// Dummy
}

hkpClosestRayHitCollector::~hkpClosestRayHitCollector() {
	// Dummy
}

hkpRayHitCollector::~hkpRayHitCollector() {
	// Dummy
}


ExtraDataList::ExtraDataList() {
	// Dummy
}
