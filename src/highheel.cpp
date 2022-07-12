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

	float npcnode_z(Actor* actor) {
		if (!actor) {
			return 0.0;
		}
		std::string node_name = "NPC Root [Root]";
		auto npc_node = find_node(actor, node_name, false);
		if (!npc_node) {
			npc_node = find_node(actor, node_name, true);
		}
		if (!npc_node) {
			return 0.0;
		}
		return npc_node->local.translate.z;
	}

	NiAVObject* find_any_node(Actor* actor, std::string_view name) {
		for (bool person: {false, true}) {
			auto found = find_node(actor, name, person);
			if (found) {
				return found;
			}
		}
	}
	return nullptr;
}

namespace Gts {
	void apply_high_heel_scale(Actor* actor) {
		if (!actor) {
			return;
		}
		if (!Persistent::GetSingleton().highheel_correction) {
			return;
		}
		int method = 1;

		float new_hh = 0.0;
		std::string node_name;
		if (method == 0) {
			float base_hh = get_hh_offset(actor);
			log::info("Paresed hh offset: {}", base_hh);
			float scale = get_npcnode_scale(actor);
			if (scale < 0.0) {
				return;
			}
			new_hh =base_hh * scale;
			node_name = "NPC";
		} else if (method == 1) {
			NiAVObject* npc_node = find_any_node(actor, "NPC");
			if (!npc_node) {
				return;
			}

			NiAVObject* root_node = find_any_node(actor, "NPC Root [Root]");
			if (!root_node) {
				return;
			}

			NiAVObject* com_node = find_any_node(actor, "NPC COM [COM ]");
			if (!com_node) {
				return;
			}

			NiAVObject* body_node = find_any_node(actor, "CME Body [Body]");
			if (!body_node) {
				return;
			}

			float base_hh = npc_node->local.translate.z;
			float scale = root_node->local.scale;
			new_hh = (scale * base_hh - base_hh) / (com_node->local.scale * root_node->local.scale * npc_node->local.scale);
			node_name = "CME Body [Body]";
		}

		if (node_name) {
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
}
