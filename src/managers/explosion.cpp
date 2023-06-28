#include "managers/explosion.hpp"
#include "managers/impact.hpp"
#include "managers/GtsManager.hpp"
#include "scale/scale.hpp"
#include "data/runtime.hpp"
#include "managers/highheel.hpp"
#include "raycast.hpp"


using namespace SKSE;
using namespace RE;
using namespace Gts;
using namespace std;

namespace {
	void make_explosion_at(FootEvent kind, Actor* actor, NiPoint3 position, float scale) {
		if (!actor) {
			return;
		}
		BGSExplosion* base_explosion = nullptr;
		switch (kind) {
			case FootEvent::Left:
			case FootEvent::Right:
			case FootEvent::Front:
			case FootEvent::Back:
				base_explosion = Runtime::GetExplosion("footstepExplosion");
			case FootEvent::JumpLand:
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
			explosion->GetExplosionRuntimeData().radius *= scale;
			explosion->GetExplosionRuntimeData().imodRadius *= scale;
			explosion->GetExplosionRuntimeData().negativeVelocity *= 0.0;
			explosion->GetExplosionRuntimeData().unk11C *= 0.0;
		}
	}
}

namespace Gts {
	ExplosionManager& ExplosionManager::GetSingleton() noexcept {
		static ExplosionManager instance;
		return instance;
	}

	std::string ExplosionManager::DebugName() {
		return "ExplosionManager";
	}

	void ExplosionManager::OnImpact(const Impact& impact) {
		if (!impact.actor) {
			return;
		}
		auto actor = impact.actor;

		float scale = impact.scale;
		float minimal_size = 3.0;
		if (actor->formID == 0x14) {
			if (HasSMT(actor)) {
				minimal_size = 1.0;
				scale += 0.33;
			}
		}
		if (scale > minimal_size && !actor->AsActorState()->IsSwimming()) {
			if (actor->AsActorState()->IsSprinting()) {
				scale *= 1.2; // Sprinting makes you seem bigger
			}
			if (actor->AsActorState()->IsWalking()) {
				scale *= 0.75; // Walking makes you seem smaller
			}
			if (actor->IsSneaking()) {
				scale *= 0.60; // Sneaking makes you seem smaller
			}
			FootEvent foot_kind = impact.kind;
			if (foot_kind == FootEvent::JumpLand) {
				scale *= 2.0; // Jumping makes you sound bigger
			}
			if (HighHeelManager::IsWearingHH(actor)) {
				scale *= 1.25; // Wearing High Heels makes you bigger
			}

			for (NiAVObject* node: impact.nodes) {
				// First try casting a ray
				NiPoint3 foot_location = node->world.translate;

				float hh_offset = HighHeelManager::GetHHOffset(actor).Length();
				NiPoint3 ray_start = foot_location + NiPoint3(0.0, 0.0, meter_to_unit(0.05*scale - hh_offset)); // Shift up a little then subtract the hh offset
				NiPoint3 ray_direction(0.0, 0.0, -1.0);
				bool success = false;
				float ray_length = meter_to_unit(std::max(1.05*scale, 1.05));
				NiPoint3 explosion_pos = CastRay(actor, ray_start, ray_direction, ray_length, success);

				if (!success) {
					explosion_pos = foot_location;
					explosion_pos.z = actor->GetPosition().z;
				}
				if (actor->formID == 0x14 && Runtime::GetBool("PCAdditionalEffects")) {
					/*if (HighHeelManager::IsWearingHH(actor)) {
						NiPoint3 FootPos = explosion_pos + NiPoint3(meter_to_unit(0.05*scale), meter_to_unit(0.05*scale), 0.0);
						NiPoint3 HeelPos = explosion_pos + NiPoint3(meter_to_unit(-0.05*scale), meter_to_unit(-0.05*scale), 0.0);
						make_explosion_at(impact.kind, actor, FootPos, scale);
						make_explosion_at(impact.kind, actor, HeelPos, scale);
					} else {
						make_explosion_at(impact.kind, actor, explosion_pos, scale);
					}*/
					/// Sermit To-do: spawn 2 dust effects: at the tip of feet and under the heel, when we have HH off. Currently misses rotation math.
					make_explosion_at(impact.kind, actor, explosion_pos, scale);
				}
				if (actor->formID != 0x14 && Runtime::GetBool("NPCSizeEffects")) {
					make_explosion_at(impact.kind, actor, explosion_pos, scale);
				}
			}
		}
	}
}
