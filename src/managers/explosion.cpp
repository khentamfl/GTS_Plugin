#include "managers/explosion.h"
#include "managers/impact.h"
#include "managers/GtsManager.h"
#include "scale/scale.h"
#include "data/runtime.h"
#include "data/transient.h"
#include "raycast.h"


using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	void make_explosion_at(Foot kind, Actor* actor, NiPoint3 position, float scale) {
		if (!actor) return;

		BGSExplosion* base_explosion = nullptr;
		switch (kind) {
			case Foot::Left:
			case Foot::Right:
			case Foot::Front:
			case Foot::Back:
				base_explosion = Runtime::GetSingleton().footstepExplosion;
			case Foot::JumpLand:
				base_explosion = Runtime::GetSingleton().footstepExplosion;
		}

		if (base_explosion) {
			NiPointer<TESObjectREFR> instance_ptr = actor->PlaceObjectAtMe(base_explosion, false);
			if (!instance_ptr) return;
			TESObjectREFR* instance = instance_ptr.get();
			if (!instance) return;
			Explosion* explosion = instance->AsExplosion();
			if (!explosion) return;
			explosion->SetPosition(position);
			explosion->radius *= scale;
			explosion->imodRadius *= scale;
			explosion->unkB8 = nullptr;
		}
	}
}

namespace Gts {
	ExplosionManager& ExplosionManager::GetSingleton() noexcept {
		static ExplosionManager instance;
		return instance;
	}

	void ExplosionManager::OnImpact(const Impact& impact) {
		if (!impact.actor) return;
		auto actor = impact.actor;

		float scale = impact.effective_scale;
		float minimal_size = 4.0;
		if (scale > minimal_size && !actor->IsSwimming()) {
			if (actor->IsSprinting()) {
				scale *= 1.2; // Sprinting makes you seem bigger
			} else if (actor->IsSneaking()) {
				scale *= 0.55; // Sneaking makes you seem quieter
			} else if (actor->IsWalking()) {
				scale *= 0.85; // Walking makes you seem quieter
			}
			Foot foot_kind = impact.kind;
			if (foot_kind == Foot::JumpLand) {
				scale *= 2.0; // Jumping makes you sound bigger
			}
			for (NiAVObject* node: impact.nodes) {
				// First try casting a ray
				//log::info("# Foot Node: {}", node->name.c_str());
				NiPoint3 foot_location = node->world.translate;
				//log::info("  - Position: {},{},{}", foot_location.x, foot_location.y, foot_location.z);
				NiPoint3 ray_start = foot_location + NiPoint3(0.0, 0.0, meter_to_unit(0.05)); // Shift up a little
				NiPoint3 ray_direction(0.0, 0.0, -1.0);
				bool success = false;
				float ray_length = meter_to_unit(std::max(1.05*impact.scale, 1.05));
				NiPoint3 explosion_pos = CastRay(actor, ray_start, ray_direction, ray_length, success);

				if (!success) {
					return;
					explosion_pos = foot_location;
					auto temp_data = Transient::GetSingleton().GetData(impact.actor);
					if (temp_data) {
						explosion_pos.z -= temp_data->total_hh_adjustment;
					}
				}
				make_explosion_at(impact.kind, actor, explosion_pos, scale);
			}
		}
	}

}
