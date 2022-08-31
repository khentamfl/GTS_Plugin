#include "hooks/BShkbAnimationGraph.hpp"
#include "util.hpp"
#include "data/persistent.hpp"

using namespace RE;
using namespace SKSE;
using namespace Gts;

namespace {
	Actor* GetActorForGraph(BShkbAnimationGraph* this_graph) {
		for (auto actor: find_actors()) {
			BSAnimationGraphManagerPtr animGraphManager;
			if (actor->GetAnimationGraphManager(animGraphManager)) {
				for (auto& graph : animGraphManager->graphs) {
					if (this_graph == graph.get()) {
						return actor;
					}
				}
			}
		}
		return nullptr;
	}

}
namespace Hooks
{
	void Hook_BShkbAnimationGraph::Hook() {
		logger::info("Hooking BShkbAnimationGraph");
		REL::Relocation<std::uintptr_t> vtbl{ RE::VTABLE_BShkbAnimationGraph[0] };

		_AddRagdollToWorld = vtbl.write_vfunc(0x02, AddRagdollToWorld);
		_RemoveRagdollFromWorld = vtbl.write_vfunc(0x03, RemoveRagdollFromWorld);
		_ResetRagdoll = vtbl.write_vfunc(0x05, ResetRagdoll);
		_SetRagdollConstraintsFromBhkConstraints = vtbl.write_vfunc(0x07, SetRagdollConstraintsFromBhkConstraints);
	}

	void Hook_BShkbAnimationGraph::AddRagdollToWorld(BShkbAnimationGraph* a_this) {
		Actor* actor = GetActorForGraph(a_this);
		if (actor) {
			if (actor->formID == 0x14) {
				log::info("+ AddRagdollToWorld");
			}
		}
		_AddRagdollToWorld(a_this);
		if (actor) {
			if (actor->formID == 0x14) {
				log::info("- AddRagdollToWorld");
			}
		}
	}

	void Hook_BShkbAnimationGraph::RemoveRagdollFromWorld(BShkbAnimationGraph* a_this) {
		Actor* actor = GetActorForGraph(a_this);
		if (actor) {
			if (actor->formID == 0x14) {
				log::info("+ RemoveRagdollFromWorld");
			}
		}
		_RemoveRagdollFromWorld(a_this);
		if (actor) {
			if (actor->formID == 0x14) {
				log::info("- RemoveRagdollFromWorld");
			}
		}
	}

	void Hook_BShkbAnimationGraph::ResetRagdoll(BShkbAnimationGraph* a_this) {
		Actor* actor = GetActorForGraph(a_this);
		if (actor) {
			if (actor->formID == 0x14) {
				log::info("+ ResetRagdoll");
			}
		}
		_ResetRagdoll(a_this);
		if (actor) {
			if (actor->formID == 0x14) {
				log::info("- ResetRagdoll");
			}
		}
	}

	void Hook_BShkbAnimationGraph::SetRagdollConstraintsFromBhkConstraints(BShkbAnimationGraph* a_this) {
		Actor* actor = GetActorForGraph(a_this);
		if (actor) {
			if (actor->formID == 0x14) {
				log::info("+ SetRagdollConstraintsFromBhkConstraints");
			}
		}
		_SetRagdollConstraintsFromBhkConstraints(a_this);
		if (actor) {
			if (actor->formID == 0x14) {
				log::info("- SetRagdollConstraintsFromBhkConstraints");
			}
		}
	}
}
