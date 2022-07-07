#include "scale.h"
#include "util.h"
#include "GtsManager.h"

using namespace Gts;

namespace Gts {
	void set_ref_scale(Actor* actor, float target_scale) {
		// This is how the game sets scale with the `SetScale` command
		// It is limited to x10 and messes up all sorts of things like actor damage
		// and anim speeds
		float refScale = static_cast<float>(actor->refScale) / 100.0F;
		log::info("REF Scale: {}", refScale);
		if (fabs(refScale - target_scale) > 1e-5) {
			actor->refScale = static_cast<std::uint16_t>(target_scale * 100.0F);
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
			auto task = SKSE::GetTaskInterface();
			task->AddTask([model]() {
				if (model) {
					NiUpdateData ctx;
					// ctx.flags |= NiUpdateData::Flag::kDirty;
					model->UpdateWorldData(&ctx);
				}
			});
		}

		auto first_model = actor->Get3D(true);
		if (first_model) {
			result = true;
			first_model->local.scale = target_scale;
			auto task = SKSE::GetTaskInterface();
			task->AddTask([first_model]() {
				if (first_model) {
					NiUpdateData ctx;
					// ctx.flags |= NiUpdateData::Flag::kDirty;
					first_model->UpdateWorldData(&ctx);
				}
			});
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
			auto task = SKSE::GetTaskInterface();
			task->AddTask([node]() {
				if (node) {
					NiUpdateData ctx;
					// ctx.flags |= NiUpdateData::Flag::kDirty;
					node->UpdateWorldData(&ctx);
				}
			});
		}

		auto first_node = find_node(actor, node_name, true);
		if (first_node) {
			result = true;
			first_node->local.scale = target_scale;
			auto task = SKSE::GetTaskInterface();
			task->AddTask([first_node]() {
				if (first_node) {
					NiUpdateData ctx;
					// ctx.flags |= NiUpdateData::Flag::kDirty;
					first_node->UpdateWorldData(&ctx);
				}
			});
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
		return static_cast<float>(actor->refScale) / 100.0F;
	}

	float get_scale(Actor* actor) {
		auto& size_method = GtsManager::GetSingleton().size_method;
		switch (size_method) {
		case SizeMethod::ModelScale:
			return get_model_scale(actor);
			break;
		case SizeMethod::RootScale:
			return get_npcnode_scale(actor);
			break;
		case SizeMethod::RefScale:
			return get_ref_scale(actor);
			break;
		case SizeMethod::All:
			float ref_scale = get_ref_scale(actor);
			if (ref_scale < 0.0) {
				break;
			}
			float mode_scale = get_model_scale(actor);
			if (mode_scale < 0.0) {
				break;
			}
			float node_scale = get_npcnode_scale(actor);
			if (node_scale < 0.0) {
				break;
			}
			return get_ref_scale(actor) * get_model_scale(actor) * get_npcnode_scale(actor);
			break;
		}
		return -1.0;
	}

	bool set_scale(Actor* actor, float scale) {
		auto& size_method = GtsManager::GetSingleton().size_method;
		switch (size_method) {
		case SizeMethod::ModelScale:
			return set_model_scale(actor, scale);
			break;
		case SizeMethod::RootScale:
			return set_npcnode_scale(actor, scale);
			break;
		case SizeMethod::RefScale:
			get_ref_scale(actor);
			return true;
			break;
		case SizeMethod::All:
			return set_model_scale(actor, scale/(get_ref_scale(actor)*get_npcnode_scale(actor)));
			break;
		}
		return false;
	}
}
