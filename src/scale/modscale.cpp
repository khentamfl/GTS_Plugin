#include "scale/modscale.hpp"
#include "node.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "data/runtime.hpp"

using namespace Gts;

namespace Gts {
	void set_ref_scale(Actor* actor, float target_scale) {
		// This is how the game sets scale with the `SetScale` command
		// It is limited to x10 and messes up all sorts of things like actor damage
		// and anim speeds
		// Calling too fast also kills frames
		float refScale = static_cast<float>(actor->GetReferenceRuntimeData().refScale) / 100.0F;
		if (fabs(refScale - target_scale) > 1e-5) {
			actor->GetReferenceRuntimeData().refScale = static_cast<std::uint16_t>(target_scale * 100.0F);
			actor->DoReset3D(false);
		}
	}

	bool set_model_scale(Actor* actor, float target_scale) {
		// This will set the scale of the model root (not the root npc node)
		if (!actor->Is3DLoaded()) {
			return false;
		}
		bool result = false;

		auto model = actor->Get3D(false);
		if (model) {
			result = true;
			model->local.scale = target_scale;
			update_node(model);
		}

		auto first_model = actor->Get3D(true);
		if (first_model) {
			result = true;
			first_model->local.scale = target_scale;
			update_node(first_model);
		}
		return result;
	}

	bool set_npcnode_scale(Actor* actor, float target_scale) {
		// This will set the scale of the root npc node
		string node_name = "NPC Root [Root]";
		bool result = false;

		auto node = find_node(actor, node_name, false);
		if (node) {
			result = true;
			node->local.scale = target_scale;
			update_node(node);
		}

		auto first_node = find_node(actor, node_name, true);
		if (first_node) {
			result = true;
			first_node->local.scale = target_scale;
			update_node(first_node);
		}
		return result;
	}


	float get_npcnode_scale(Actor* actor) {
		// This will set the scale of the root npc node
		string node_name = "NPC Root [Root]";
		auto node = find_node(actor, node_name, false);
		if (node) {
			return node->local.scale;
		}
		auto first_node = find_node(actor, node_name, true);
		if (first_node) {
			return first_node->local.scale;
		}
		return -1.0;
	}

	float get_npcparentnode_scale(Actor* actor) {
		// This will set the scale of the root npc node
		string node_name = "NPC Root [Root]";
		auto childNode = find_node(actor, node_name, false);
		if (!childNode) {
			childNode = find_node(actor, node_name, true);
			if (!childNode) {
				return -1.0;
			}
		}
		auto parent = childNode->parent;
		if (parent) {
			return parent->local.scale;
		}
		return -1.0; //
	}

	float get_model_scale(Actor* actor) {
		// This will set the scale of the root npc node
		if (!actor->Is3DLoaded()) {
			return -1.0;
		}
		auto model = actor->Get3D(false);
		if (model) {
			return model->local.scale;
		}
		auto first_model = actor->Get3D(true);
		if (first_model) {
			return first_model->local.scale;
		}
		return -1.0;
	}

	float get_ref_scale(Actor* actor) {
		// This will set the scale of the root npc node
		return static_cast<float>(actor->GetReferenceRuntimeData().refScale) / 100.0F;
	}

	float get_scale(Actor* actor) {
		float ref_scale = 1.0;//get_ref_scale(actor);
		if (ref_scale < 0.0) {
			return -1.0;
		}
		float model_scale = get_model_scale(actor);
		if (model_scale < 0.0) {
			return -1.0;
		}
		float node_scale = get_npcnode_scale(actor);
		if (node_scale < 0.0) {
			return -1.0;
		}
		float npc_parentnode_scale = get_npcparentnode_scale(actor);
		if (npc_parentnode_scale < 0.0) {
			return -1.0;
		}
		return ref_scale * model_scale * node_scale * npc_parentnode_scale;
	}

	bool set_scale(Actor* actor, float scale) {
		auto& size_method = Persistent::GetSingleton().size_method;
		switch (size_method) {
			case SizeMethod::ModelScale:
				return set_model_scale(actor, scale * (get_npcnode_scale(actor)));
				break;
			case SizeMethod::RootScale:
				return set_npcnode_scale(actor, scale * (get_model_scale(actor)));
				break;
			case SizeMethod::RefScale:
				//set_ref_scale(actor, scale/(get_npcnode_scale(actor)*get_model_scale(actor)));
				if (actor->formID == 0x14) {
					return set_npcnode_scale(actor, scale * (get_model_scale(actor)));
				} else {
					return set_model_scale(actor, scale * (get_npcnode_scale(actor)));
				}
				return true;
				break;
		}
		return false;
	}
}
