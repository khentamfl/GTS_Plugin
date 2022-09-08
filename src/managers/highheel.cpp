#include "managers/highheel.hpp"
#include "node.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "util.hpp"

using namespace RE;
using namespace Gts;

namespace Gts {
	HighHeelManager& HighHeelManager::GetSingleton() noexcept {
		static HighHeelManager instance;
		return instance;
	}

	void HighHeelManager::PapyrusUpdate() {
		const bool FORCE_APPLY = false;
		auto actors = find_actors();
		for (auto actor: actors) {
			ApplyHH(actor, FORCE_APPLY);
		}
	}

	void HighHeelManager::ActorEquip(Actor* actor) {
		const bool FORCE_APPLY = true;
		ApplyHH(actor, FORCE_APPLY);
	}
	void HighHeelManager::ActorLoaded(Actor* actor) {
		const bool FORCE_APPLY = true;
		ApplyHH(actor, FORCE_APPLY);
	}

	void HighHeelManager::ApplyHH(Actor* actor, bool force) {
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}
		auto temp_data = Transient::GetSingleton().GetData(actor);
		if (!temp_data) {
			return;
		}

		float new_hh = 0.0;
		float base_hh;
		float last_hh_adjustment = temp_data->last_hh_adjustment;
		if (!Persistent::GetSingleton().highheel_correction) {
			if (fabs(last_hh_adjustment) > 1e-5) {
				log::trace("Last hh adjustment to turn it off");
			} else {
				return;
			}
		} else {
			NiAVObject* npc_node = find_node_any(actor, "NPC");
			if (!npc_node) {
				return;
			}

			NiAVObject* root_node = find_node_any(actor, "NPC Root [Root]");
			if (!root_node) {
				return;
			}

			NiAVObject* com_node = find_node_any(actor, "NPC COM [COM ]");
			if (!com_node) {
				return;
			}

			NiAVObject* body_node = find_node_any(actor, "CME Body [Body]");
			if (!body_node) {
				return;
			}

			base_hh = npc_node->local.translate.z;
			float scale = root_node->local.scale;
			new_hh = (scale * base_hh - base_hh) / (com_node->local.scale * root_node->local.scale * npc_node->local.scale);
		}

		bool adjusted = false;
		for (bool person: {false, true}) {
			auto npc_root_node = find_node(actor, "CME Body [Body]", person);
			if (npc_root_node) {
				float current_value = npc_root_node->local.translate.z;
				if ((fabs(last_hh_adjustment - new_hh) > 1e-5) || (fabs(current_value - new_hh) > 1e-5) || force) {
					npc_root_node->local.translate.z = new_hh;
					update_node(npc_root_node);
					adjusted = true;
				}
			}
		}
		if (adjusted) {
			temp_data->last_hh_adjustment = new_hh;
			temp_data->total_hh_adjustment = new_hh + base_hh;
		}
	}
}
