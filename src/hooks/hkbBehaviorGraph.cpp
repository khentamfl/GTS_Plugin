#include "managers/animation/AnimationManager.hpp"
#include "hooks/hkbBehaviorGraph.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace Hooks
{
	void Hook_hkbBehaviorGraph::Hook() {
		logger::info("Hooking hkbBehaviorGraph");
		REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_hkbBehaviorGraph[0] };

		_Update = vtbl.write_vfunc(0x05, Update);
	}

	void Hook_hkbBehaviorGraph::Update(hkbBehaviorGraph* a_this, const hkbContext& a_context, float a_timestep) {
		float anim_speed = 1.0;
		for (auto actor: find_actors()) {
			BSAnimationGraphManagerPtr animGraphManager;
			if (actor->GetAnimationGraphManager(animGraphManager)) {
				for (auto& graph : animGraphManager->graphs) {
					if (graph) {
						if (a_this == graph->behaviorGraph) {
							float bonus = 1.0;
							auto saved_data = Gts::Persistent::GetSingleton().GetData(actor);
							auto transient = Transient::GetSingleton().GetActorData(actor);
							if (transient) {
								bonus = transient->animspeedbonus;
							}
							if (saved_data) {
								if (saved_data->anim_speed > 0.0) {
									anim_speed = saved_data->anim_speed * bonus;
								}
							}
						}
					}
				}
			}
		}
		_Update(a_this, a_context, a_timestep * anim_speed);
	}
}
