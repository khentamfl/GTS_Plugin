#include "managers/explosion.h"
#include "data/runtime.h"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace Gts {
	ExplosionManager& ExplosionManager::GetSingleton() noexcept {
		static ExplosionManager instance;
		return instance;
	}

	void ExplosionManager::make_explosion(ExplosionKind kind, Actor* actor, NiAVObject* node, NiPoint3 offset, float scale) {
		if (!actor) return;
		if (!node) return;

		BGSExplosion* base_explosion = nullptr;
		switch (kind) {
			case ExplosionKind::Footstep:
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
		}
	}
}
