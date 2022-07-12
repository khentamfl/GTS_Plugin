#include "highheel.h"
#include "node.h"
#include "scale.h"
#include "GtsManager.h"
#include "persistent.h"

using namespace RE;
using namespace Gts;

namespace {
	NiAVObject* find_any_node(Actor* actor, std::string_view name) {
		for (bool person: {false, true}) {
			auto found = find_node(actor, name, person);
			if (found) {
				return found;
			}
		}
		return nullptr;
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
		log::info("Base HH: {}", base_hh);
		float scale = root_node->local.scale;
		log::info("NPC Root Scale: {}", scale);
		float new_hh = (scale * base_hh - base_hh) / (com_node->local.scale * root_node->local.scale * npc_node->local.scale);
		log::info("CME Body.z = {}", new_hh);
		for (bool person: {false, true}) {
			auto npc_root_node = find_node(actor, "CME Body [Body]", person);
			if (npc_root_node) {
				npc_root_node->local.translate.z = new_hh;
				NiUpdateData ctx;
				npc_root_node->UpdateWorldData(&ctx);
				log::info("CME updated");
			}
		}
	}
}
