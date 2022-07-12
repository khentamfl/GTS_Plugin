#include "highheel.h"
#include "node.h"
#include "scale.h"
#include "GtsManager.h"
#include "persistent.h"
#include "transient.h"

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
	void apply_high_heel_scale(Actor* actor, TempActorData* temp_data) {
		if (!actor) {
			return;
		}
		if (!temp_data) {
			return;
		}


		float new_hh = 0.0;
		float last_hh_adjustment = temp_data->last_hh_adjustment;
		if (!Persistent::GetSingleton().highheel_correction) {
			if (last_hh_adjustment > 1e-5) {
				NiAVObject* body_node = find_any_node(actor, "CME Body [Body]");
				if (!body_node) {
					return;
				}

				float current_cme = body_node->local.translate.z;
				if ((current_cme - last_hh_adjustment) > 1e-5) {
					new_hh = current_cme - last_hh_adjustment;
				}
			} else {
				return;
			}
		} else {
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
			new_hh = (scale * base_hh - base_hh) / (com_node->local.scale * root_node->local.scale * npc_node->local.scale);
			log::info("CME Body.z = {}", new_hh);
		}

		if (fabs(last_hh_adjustment - new_hh) > 1e-5) {
			temp_data->last_hh_adjustment = new_hh;
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
}
