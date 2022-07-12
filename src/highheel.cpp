#include "highheel.h"
#include "node.h"
#include "scale.h"
#include "GtsManager.h"
#include "persistent.h"

using namespace RE;
using namespace Gts;

namespace {
	float base_highheel(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		std::string node_name = "NPC";
		auto npc_node = find_node(actor, node_name, false);
		if (!npc_node) {
			npc_node = find_node(actor, node_name, true);
		}
		if (!npc_node) {
			return 0.0;
		}
		log::info("NPC Scale {}", npc_node->local.scale);
		float parent_hh = npc_node->local.translate.z;
		// we are going to apply this to the child of this node
		// which is NPCRoot Node. we need to put this height
		// in child space
		float child_hh = parent_hh / npc_node->local.scale;
		// Trying also refscale
		// This is maybe applied indirectly (maybe via animation) to the NPC node
		// float ref_scale = get_ref_scale(actor);
		// log::info("refscale: {}", ref_scale);
		// child_hh /= ref_scale;
		return child_hh;
	}
}

namespace Gts {
	void apply_high_heel_scale(Actor* actor) {
		if (!actor) {
			return;
		}
		if (!Persistent::GetSingleton().highheel_correction) {
			return;
		}
		float new_hh = 0.0;
		bool experiment = true;
		std::string node_name = "NPC Root [Root]";
		if (experiment) {
			float base_heel = 8.0;
			float scale = get_npcnode_scale(actor);
			new_hh = scale * base_heel;
			node_name = "NPC";
		} else {
			float base_heel = base_highheel(actor);
			log::info("Base HH: {}", base_heel);
			// If there is any scale on the ModelScale
			// or refscale then it already corrects the
			// high heels
			// therefore we just grab the effects of the npc node
			// scale
			float scale = get_npcnode_scale(actor);
			new_hh = scale * base_heel;
			log::info("New HH (Unadjusted): {}", new_hh);
			// We are going to translate NPC Root [Root]
			// rather than NPC node
			// This will  leave the NPC node as having a
			// the base high heel height
			// But so we don't double it up we subtract base_heel
			// from the new_hh height that we will apply to
			// NPC Root [Root] only a correction
			new_hh -= base_heel;
			log::info("New HH (Adjusted): {}", new_hh);
			// The true foot position should lie between
			// base_heel and (base_heel -1) because most meshes put it slighly under the floor
			// We apply maximal error approximations to remove the scaled error
			float maximal_error = -1.0;
			float scaled_error = maximal_error*scale;
			float scaled_error_delta = scaled_error - maximal_error;
			log::info("Scaled maximal error delta: {}", scaled_error_delta);
			new_hh -= scaled_error_delta;
			log::info("New HH (Error corrected): {}", new_hh);
			// Now to set it for third person
		}

		for (bool person: {false, true}) {
			auto npc_root_node = find_node(actor, node_name, person);
			if (npc_root_node) {
				npc_root_node->local.translate.z = new_hh;
				NiUpdateData ctx;
				npc_root_node->UpdateWorldData(&ctx);
			}
		}
	}
}
