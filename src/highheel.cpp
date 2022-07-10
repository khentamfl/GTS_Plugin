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
		return npc_node->local.translate.z;
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
		float base_heel = base_highheel(actor);
		// If there is any scale on the ModelScale
		// or refscale then it already corrects the
		// high heels
		// therefore we just grab the effects of the npc node
		// scale
		float scale = get_npcnode_scale(actor);
		float new_hh = scale * base_heel;
		// We are going to translate NPC Root [Root]
		// rather than NPC node
		// This will  leave the NPC node as having a
		// the base high heel height
		// But so we don't double it up we subtract base_heel
		// from the new_hh height that we will apply to
		// NPC Root [Root] only a correction
		new_hh -= base_heel;
		// Now to set it for third person
		std::string node_name = "NPC Root [Root]";
		auto npc_root_node = find_node(actor, node_name, false);
		if (npc_root_node) {
			npc_root_node->local.translate.z = new_hh;
		}
		auto npc_root_node_fp = find_node(actor, node_name, true);
		if (npc_root_node_fp) {
			npc_root_node_fp->local.translate.z = new_hh;
		}
	}
}
