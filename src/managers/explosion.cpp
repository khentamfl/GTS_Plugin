#include "managers/explosion.h"
#include "managers/impact.h"
#include "data/runtime.h"
#include "data/transient.h"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	void make_explosion(Foot kind, Actor* actor, NiAVObject* node, NiPoint3 offset, float scale) {
		if (!actor) return;
		if (!node) return;

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
			explosion->MoveToNode(actor, node);
			NiPoint3 new_pos = explosion->GetPosition() + offset;
			explosion->SetPosition(new_pos);
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
				scale *= 1.2; // Sprinting makes you sound bigger
			} else if (actor->IsSneaking()) {
				scale *= 0.55; // Sneaking makes you sound quieter
			} else if (actor->IsWalking()) {
				scale *= 0.85; // Walking makes you sound quieter
			}
			Foot foot_kind = impact.kind;
			if (foot_kind == Foot::JumpLand) {
				scale *= 1.2; // Jumping makes you sound bigger
			}
			for (NiAVObject* node: impact.nodes) {
				NiPoint3 offset = NiPoint3();
				auto temp_data = Transient::GetSingleton().GetActorData(impact.actor);
				if (temp_data) {
					offset.z -= temp_data->total_hh_adjustment;
				}
				offset.z-=3;
				make_explosion(impact.kind, actor, node, offset, scale);
			}
		}
	}

}
