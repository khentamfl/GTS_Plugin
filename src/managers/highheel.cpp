#include "managers/highheel.hpp"
#include "node.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "util.hpp"

using namespace RE;
using namespace Gts;

namespace {
	NiAVObject* find_any_node(Actor* actor, std::string_view name) {
		for (auto person: {Person::Third, Person::First}) {
			auto found = find_node(actor, name, person);
			if (found) {
				return found;
			}
		}
		return nullptr;
	}
}

namespace Gts {
	void apply_highheel(Actor* actor, TempActorData* temp_data, bool force) {
		if (!actor) {
			return;
		}
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

			base_hh = npc_node->local.translate.z;
			float scale = root_node->local.scale;
			new_hh = (scale * base_hh - base_hh) / (com_node->local.scale * root_node->local.scale * npc_node->local.scale);
		}

		bool adjusted = false;
		for (auto person: {Person::Third, Person::First}) {
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
