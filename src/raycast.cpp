#include "UI/DebugAPI.hpp"
#include "raycast.hpp"

using namespace Gts;
using namespace RE;

namespace {
  NiPoint3 CastRayImpl(TESObjectREFR* ref, const NiPoint3& in_origin, const NiPoint3& direction, const float& unit_length, RayCollector& collector, bool& success) {
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


	NiPoint3 CastRay_Experimental(TESObjectREFR* ref, const NiPoint3& in_origin, const NiPoint3& direction, const float& unit_length, RayCollector& collector, bool& success) {
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

		//pick_data.rayHitCollectorA8 = &collector;

		pick_data.rayInput.enableShapeCollectionFilter = false;
		pick_data.rayInput.filterInfo = bhkCollisionFilter::GetSingleton()->GetNewSystemGroup() << 16 | stl::to_underlying(COL_LAYER::kLOS);

		float min_fraction = 1.0;

		if (collision_world->PickObject(pick_data); pick_data.rayOutput.HasHit()) {
			auto Object = static_cast<COL_LAYER>(pick_data.rayOutput.rootCollidable->broadPhaseHandle.collisionFilterInfo & 0x7F);
			log::info(" Hit Layer True:  {}, result count: {}", Object, collector.results.size());
			float fraction = pick_data.rayOutput.hitFraction;
			if (fraction < min_fraction) {
				min_fraction = fraction;
			}
			NiPoint3 hitdata = meter_to_unit(origin + normed * length * min_fraction);
			DebugAPI::DrawSphere(glm::vec3(hitdata.x, hitdata.y, hitdata.z), 8.0, 800, {1.0, 1.0, 0.0, 1.0});
			return hitdata;
			log::info(" Hit Layer Coords:  {}, result true", Vector2Str(hitdata));
		} 
		return NiPoint3();
	}
}

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
		log::info("New hit:");
			const hkpShape* shape = a_body.GetShape(); // Shape that was collided with
		if (shape) {
		auto ni_shape = shape->userData;
				if (ni_shape) {
			auto filter_info = ni_shape->filterInfo;
			log::info("  - First Shape: {}", filter_info);
			COL_LAYER collision_layer = static_cast<COL_LAYER>(filter_info & 0x7F);
			log::info("  - Layer {}", collision_layer);
			log::info("  - Group {}", filter_info >> 16);
		}
    }
		// Search for top level shape
		const hkpCdBody* top_body = a_body.parent;
		while (top_body) {
			if (top_body->shape) {
				shape = top_body->shape;
        if (shape) {
          auto ni_shape = shape->userData;
    			if (ni_shape) {
            auto filter_info = ni_shape->filterInfo;
            log::info("  - Parent Shape: {}", filter_info);
            COL_LAYER collision_layer = static_cast<COL_LAYER>(filter_info & 0x7F);
            log::info("  - Layer {}", collision_layer);
            log::info("  - Group {}", filter_info >> 16);
          }
        }
			}
			top_body = top_body->parent;
		}

		if (shape) {
			auto ni_shape = shape->userData;
			if (ni_shape) {
        auto filter_info = ni_shape->filterInfo;

        log::info("  - Final Shape: {}", filter_info);
        if (this->skip_capsules && shape->type == hkpShapeType::kCapsule) {
          log::info("  - Skip capsual");
          // Skip capsules
          return;
        }

        COL_LAYER collision_layer = static_cast<COL_LAYER>(filter_info & 0x7F);
        log::info("  - Layer {}", collision_layer);
        log::info("  - Group {}", filter_info >> 16);

        if (! groups.empty()) {
          bool found = false;
          for (auto group: groups) {
            if (group == collision_layer) {
              found = true;
              break;
            }
          }
          if (!found) {
            log::info("  - Collision layer not in allowed group");
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

  NiPoint3 CastRay(TESObjectREFR* ref, const NiPoint3& origin, const NiPoint3& direction, const float& length, bool& success) {
    RayCollector collector = RayCollector();
	collector.add_filter(ref->Get3D1(false));
	collector.add_filter(ref->Get3D1(true));
    collector.skip_capsules = false;
    return CastRayImpl(ref, origin, direction, length, collector, success);
  }

  NiPoint3 CastRayStatics(TESObjectREFR* ref, const NiPoint3& origin, const NiPoint3& direction, const float& length, bool& success) {
    RayCollector collector = RayCollector();
	collector.add_filter(ref->Get3D1(false));
	collector.add_filter(ref->Get3D1(true));
	collector.skip_capsules = true;
	std::vector<COL_LAYER> groups = {COL_LAYER::kUnidentified, COL_LAYER::kProps, COL_LAYER::kStatic, COL_LAYER::kTerrain, COL_LAYER::kGround, COL_LAYER::kInvisibleWall, COL_LAYER::kTransparentWall};
	for (auto& group: groups) {
		collector.add_group_filter(group);
	}
    return CastRay_Experimental(ref, origin, direction, length, collector, success);
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
