#include "highheel.h"
#include "node.h"
#include "scale.h"
#include "GtsManager.h"
#include "persistent.h"

using namespace RE;
using namespace Gts;

namespace {
	void experiment(Actor* actor) {
		if (!actor) {
			return;
		}
		std::string node_name = "NPC";
		auto npc_node = find_node(actor, node_name, false);
		if (!npc_node) {
			return;
		}
		std::string rootnode_name = "NPC Root [Root]";
		auto rootnpc_node = find_node(actor, rootnode_name, false);
		if (!rootnpc_node) {
			return;
		}
		float current_hh = npc_node->local.translate.z;
		log::info("== Experiment HH ==");
		log::info("- HH size: {}", current_hh);
		auto world_pos = rootnpc_node->world.translate;
		log::info("- NPC Root World Pos: {},{},{}", world_pos.x, world_pos.y, world_pos.z);
		log::info("- Removing hh");
		npc_node->local.translate.z = 0.0;
        NiUpdateData ctx;
		// ctx.flags |= NiUpdateData::Flag::kDirty;
        npc_node->UpdateWorldData(&ctx);
		auto ref_world_pos = rootnpc_node->world.translate;
		log::info("- NPC Root World Pos: {},{},{}", ref_world_pos.x, ref_world_pos.y, ref_world_pos.z);
		auto delta = world_pos - ref_world_pos;
		log::info("- Delta: {},{},{}", delta.x, delta.y, delta.z);
		log::info("- Reapplying hh");
		npc_node->local.translate.z = current_hh;
        NiUpdateData ctx2;
		// ctx.flags |= NiUpdateData::Flag::kDirty;
        npc_node->UpdateWorldData(&ctx2);
	}
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
		return npc_node->local.translate.z;
	}

	float npc_scale(Actor* actor) {
		if (!actor) {
			return 1.0;
		}
		std::string node_name = "NPC";
		auto npc_node = find_node(actor, node_name, false);
		if (!npc_node) {
			npc_node = find_node(actor, node_name, true);
		}
		if (!npc_node) {
			return 1.0;
		}
		return npc_node->local.scale;
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
		experiment(actor);
		float base_heel = base_highheel(actor);
		log::info("Base HH: {}", base_heel);
		// If there is any scale on the ModelScale
		// or refscale then it already corrects the
		// high heels
		// therefore we just grab the effects of the npc node
		// scale
		float scale = get_npcnode_scale(actor);
		// We also have to account for and
		// undo the scale of the node between
		// [NPC..NPC Root Node)
		// ...Which is just NPC
		scale /= npc_scale(actor);
		// Trying also refscale
		// This maybe applied on the indirectly (maybe via animation) to the NPC node
		float ref_scale = get_ref_scale(actor);
		scale /= ref_scale;
		float new_hh = scale * base_heel;
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
