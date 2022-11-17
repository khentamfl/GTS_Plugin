#include "managers/explosion.hpp"
#include "managers/impact.hpp"
#include "managers/GtsManager.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "data/transient.hpp"
#include "raycast.hpp"


using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	void make_explosion_at(Foot kind, Actor* actor, NiPoint3 position, float scale) {
		if (!actor) {
			return;
		}

		BGSExplosion* base_explosion = nullptr;
		switch (kind) {
			case Foot::Left:
			case Foot::Right:
			case Foot::Front:
			case Foot::Back:
				base_explosion = Runtime::GetExplosion("footstepExplosion");
			case Foot::JumpLand:
				base_explosion = Runtime::GetExplosion("footstepExplosion");
		}

		if (base_explosion) {
			NiPointer<TESObjectREFR> instance_ptr = actor->PlaceObjectAtMe(base_explosion, false);
			if (!instance_ptr) {
				return;
			}
			TESObjectREFR* instance = instance_ptr.get();
			if (!instance) {
				return;
			}
			Explosion* explosion = instance->AsExplosion();
			if (!explosion) {
				return;
			}
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
		if (!impact.actor) {
			return;
		}
		auto actor = impact.actor;

		float scale = impact.effective_scale;
		float minimal_size = 3.0;
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
			auto actor_data = Transient::GetSingleton().GetData(actor);
			if (actor_data) {
				if (actor_data->wearing_hh()) {
					scale *= 1.25;
				}
			}

			for (NiAVObject* node: impact.nodes) {
				// First try casting a ray
				NiPoint3 foot_location = node->world.translate;

				float hh_offset = 0.0;
				auto temp_data = Transient::GetSingleton().GetData(actor);
				if (temp_data) {
					hh_offset = temp_data->total_hh_adjustment;
				}
				NiPoint3 ray_start = foot_location + NiPoint3(0.0, 0.0, meter_to_unit(0.05*impact.scale - hh_offset)); // Shift up a little then subtract the hh offset
				NiPoint3 ray_direction(0.0, 0.0, -1.0);
				bool success = false;
				float ray_length = meter_to_unit(std::max(1.05*impact.scale, 1.05));
				NiPoint3 explosion_pos = CastRay(actor, ray_start, ray_direction, ray_length, success);

				if (!success) {
					explosion_pos = foot_location;
					explosion_pos.z = actor->GetPosition().z;
				}
				if (actor->formID == 0x14 && Runtime::GetBool("PCAdditionalEffects") >= 1.0) {
					make_explosion_at(impact.kind, actor, explosion_pos, scale);
				}
				if (actor->formID != 0x14 && Runtime::GetBool("NPCSizeEffects") >= 1.0) {
					make_explosion_at(impact.kind, actor, explosion_pos, scale);
				}
			}
		}
	}

}
