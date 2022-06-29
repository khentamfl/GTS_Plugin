#include <SKSE/SKSE.h>
#include <Config.h>
#include <GtsManager.h>
#include <vector>
#include <string>

using namespace Gts;
using namespace RE;
using namespace SKSE;
using namespace std;

namespace {
	/**
	 * Find actors in ai manager that are loaded
	 */
	vector<ActorHandle> find_actors() {
		vector<ActorHandle> result;

		auto process_list = ProcessLists::GetSingleton();
		for (ActorHandle actor_handle: process_list->highActorHandles)
		{
			auto actor = actor_handle.get();
			if (actor && actor->Is3DLoaded())
			{
				result.push_back(actor_handle);
			}
		}

		return result;
	}

	float unit_to_meter(float unit) {
		// Game reports that the height of a slaughterfish is 0.31861934
		// From inspecting the bounding box of the slaughterfish and applying
		// base actor scales the unit height is 22.300568
		// Assuming 0.31861934 is meters and that bouding box is in model unit space
		// then the conversion factor is 70
		// Slaughterfish was chosen because it has scales of 1.0 (and was in my worldspace)
		// The scaling factor of 70 also applies to actor heights (once you remove)
		// race specific height scaling
		return unit / 70.0;
	}

	float meter_to_unit(float meter) {
		// Game reports that the height of a slaughterfish is 0.31861934
		// From inspecting the bounding box of the slaughterfish and applying
		// base actor scales the unit height is 22.300568
		// Assuming 0.31861934 is meters and that bouding box is in model unit space
		// then the conversion factor is 70
		// Slaughterfish was chosen because it has scales of 1.0 (and was in my worldspace)
		// The scaling factor of 70 also applies to actor heights (once you remove)
		// race specific height scaling
		return meter * 70.0;
	}

	void walk_nodes(Actor* actor) {
		if (!actor->Is3DLoaded()) {
			return;
		}
		auto model = actor->Get3D();
		auto name = model->name;

		std::deque<NiAVObject*> queue;
		queue.push_back(model);


		while (!queue.empty()) {
			auto currentnode = queue.front();
			queue.pop_front();
			try {
				if (currentnode) {
					auto ninode = currentnode->AsNode();
					if (ninode) {
						for (auto child: ninode->GetChildren()) {
							// Bredth first search
							queue.push_back(child.get());
							// Depth first search
							//queue.push_front(child.get());
						}
					}
					// Do smth
					log::trace("Node {}", currentnode->name);
				}
			}
			catch (const std::overflow_error& e) {
				log::warn("Overflow: {}", e.what());
			} // this executes if f() throws std::overflow_error (same type rule)
			catch (const std::runtime_error& e) {
				log::warn("Underflow: {}", e.what());
			} // this executes if f() throws std::underflow_error (base class rule)
			catch (const std::exception& e) {
				log::warn("Exception: {}", e.what());
			} // this executes if f() throws std::logic_error (base class rule)
			catch (...) {
				log::warn("Exception Other");
			}
		}
	}

	NiAVObject* find_node(Actor* actor, string& node_name) {
		if (!actor->Is3DLoaded()) {
			return nullptr;
		}
		auto model = actor->Get3D();
		if (!model) {
			return nullptr;
		}
		auto node_lookup = model->GetObjectByName(node_name);
		if (node_lookup) {
			return node_lookup;
		}

		// Game lookup failed we try and find it manually
		std::deque<NiAVObject*> queue;
		queue.push_back(model);


		while (!queue.empty()) {
			auto currentnode = queue.front();
			queue.pop_front();
			try {
				if (currentnode) {
					auto ninode = currentnode->AsNode();
					if (ninode) {
						for (auto child: ninode->GetChildren()) {
							// Bredth first search
							queue.push_back(child.get());
							// Depth first search
							//queue.push_front(child.get());
						}
					}
					// Do smth
					if  (currentnode->name.c_str() == node_name) {
						return currentnode;
					}
				}
			}
			catch (const std::overflow_error& e) {
				log::warn("Overflow: {}", e.what());
			} // this executes if f() throws std::overflow_error (same type rule)
			catch (const std::runtime_error& e) {
				log::warn("Underflow: {}", e.what());
			} // this executes if f() throws std::underflow_error (base class rule)
			catch (const std::exception& e) {
				log::warn("Exception: {}", e.what());
			} // this executes if f() throws std::logic_error (base class rule)
			catch (...) {
				log::warn("Exception Other");
			}
		}

		return nullptr;
	}

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
		log::info("Set REF Scale: {}, {}", actor->refScale, static_cast<float>(actor->refScale));
	}

	void set_model_scale(Actor* actor, float target_scale) {
		// This will set the scale of the model root (not the root npc node)
		if (!actor->Is3DLoaded()) {
			return;
		}
		auto model = actor->Get3D();
		if (!model) {
			return;
		}
		model->local.scale = target_scale;
	}

	void set_npcnode_scale(Actor* actor, float target_scale) {
		// This will set the scale of the root npc node
		string node_name = "NPC Root [Root]";
		auto node = find_node(actor, node_name);
		if (node) {
			node->local.scale = target_scale;
		}
	}

	void clone_bound(Actor* actor) {
		// This is the bound on the NiExtraNodeData
		// This data is shared between all skeletons and this hopes to correct this
		auto model = actor->Get3D();
		if (model) {
			auto extra_bbx = model->GetExtraData("BBX");
			if (extra_bbx) {
				BSBound* bbx = static_cast<BSBound*>(extra_bbx);
				model->RemoveExtraData("BBX");
				auto new_extra_bbx = NiExtraData::Create<BSBound>();
				new_extra_bbx->name = bbx->name;
				new_extra_bbx->center = bbx->center;
				new_extra_bbx->extents = bbx->extents;
				//model->AddExtraData("BBX",  new_extra_bbx);
				model->InsertExtraData(new_extra_bbx);
			}
		}
	}

	BSBound* get_bound(Actor* actor) {
		// This is the bound on the NiExtraNodeData
		auto model = actor->Get3D();
		if (model) {
			auto extra_bbx = model->GetExtraData("BBX");
			if (extra_bbx) {
				BSBound* bbx = static_cast<BSBound*>(extra_bbx);
				return bbx;
			}
		}
		return nullptr;
	}

	void update_height(Actor* actor) {
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}

		auto actor_data = GtsManager::GetSingleton().get_actor_extra_data(actor);
		if (actor_data) {
			if (!actor_data->initialised) {
				auto& base_height_data = actor_data->base_height;

				auto base_actor = actor->GetActorBase();
				auto actor_name = base_actor->GetFullName();
				log::info("Updating height of: {}", actor_name);

				auto char_controller = actor->GetCharController();
				if (!char_controller) {
					log::info("No char controller: {}", actor_name);
					return;
				}
				clone_bound(actor);
				auto bsbound = get_bound(actor);
				if (!bsbound) {
					log::info("No bound: {}", actor_name);
					return;
				}
				auto ai_process = actor->currentProcess;

				log::info("Current Bounding box: {},{},{}", bsbound->extents.x, bsbound->extents.y, bsbound->extents.z);
				float scale = Gts::Config::GetSingleton().GetTest().GetScale();
				char_controller->scale = scale;
				uncache_bound(&base_height_data.collisionBound, &char_controller->collisionBound);
				char_controller->collisionBound.extents *= scale;
				char_controller->collisionBound.center *= scale;
				uncache_bound(&base_height_data.bumperCollisionBound, &char_controller->bumperCollisionBound);
				char_controller->bumperCollisionBound.extents *= scale;
				char_controller->bumperCollisionBound.center *= scale;
				uncache_bound(&base_height_data.collisionBound, bsbound);
				bsbound->extents *= scale;
				bsbound->center *= scale;

				float model_height = bsbound->extents.z * 2 * actor->GetBaseHeight();
				float meter_height = unit_to_meter(model_height);
				char_controller->actorHeight = meter_height;
				char_controller->swimFloatHeight = meter_height * base_height_data.swimFloatHeightRatio;
				if (ai_process) {
					ai_process->SetCachedHeight(model_height);
					ai_process->cachedValues->cachedEyeLevel = model_height * 0.95;
				}
				log::info("Data updated");


				if (ai_process) {
					ai_process->Update3DModel(actor);
				} else {
					log::info("No ai: {}", actor_name);
				}
				actor->DoReset3D(false);

				log::info("New Bounding box: {},{},{}", bsbound->extents.x, bsbound->extents.y, bsbound->extents.z);

				set_model_scale(actor, scale);
				actor_data->initialised = true;
			}
		}
	}
}

GtsManager& GtsManager::GetSingleton() noexcept {
	static GtsManager instance;
	return instance;
}

// Poll for updates
void GtsManager::poll() {
	auto player_char = RE::PlayerCharacter::GetSingleton();
	if (!player_char) {
		return;
	}
	if (!player_char->Is3DLoaded()) {
		return;
	}

	auto ui = RE::UI::GetSingleton();
	if (!ui->GameIsPaused()) {
		const auto& frame_config = Gts::Config::GetSingleton().GetFrame();
		auto init_delay = frame_config.GetInitDelay();
		auto step = frame_config.GetStep() + 1; // 1 Based index

		auto current_frame = this->frame_count.fetch_add(1);
		if (current_frame < init_delay) {
			return;
		}
		if ((current_frame - init_delay) % step != 0) {
			return;
		}

		auto actor_handles = find_actors();
		for (auto actor_handle: actor_handles) {
			auto sptr_actor = actor_handle.get();
			if (sptr_actor) {
				auto actor = sptr_actor.get();

				// Do smth
			}
		}

	}
}

void GtsManager::poll_actor(Actor* actor) {
	if (actor) {
		auto base_actor = actor->GetActorBase();
		auto actor_name = base_actor->GetFullName();

		auto race = actor->GetRace();
		auto race_name = race->GetFullName();


		// log::trace("Updating height of {}", actor_name);
		update_height(actor);
		// walk_nodes(actor);
	}
}

ActorExtraData* GtsManager::get_actor_extra_data(Actor* actor) {
	auto& umap = this->actor_data;
	auto key = actor->GetFormID();
	if (umap.find(key) == umap.end()) {
		// Try to add
		log::info("Init bounding box");
		ActorExtraData result;

		auto bsbound = get_bound(actor);
		if (!bsbound) {
			return nullptr;
		}
		auto char_controller = actor->GetCharController();
		if (!char_controller) {
			return nullptr;
		}

		cache_bound(bsbound, &result.base_height.collisionBound);
		cache_bound(&char_controller->bumperCollisionBound, &result.base_height.bumperCollisionBound);
		result.base_height.actorHeight = char_controller->actorHeight;
		result.base_height.swimFloatHeightRatio = char_controller->swimFloatHeight / char_controller->actorHeight;
		result.initialised = false;
		umap[key] = result;
	}
	log::info("Success add");
	return &umap[key];
}

void Gts::cache_bound(BSBound* src, CachedBound* dst) {
	dst->center[0] = src->center.x;
	dst->center[1] = src->center.y;
	dst->center[2] = src->center.z;
	dst->extents[0] = src->extents.x;
	dst->extents[1] = src->extents.y;
	dst->extents[2] = src->extents.z;
}
void Gts::uncache_bound(CachedBound* src, BSBound* dst) {
	dst->center.x = src->center[0];
	dst->center.y = src->center[1];
	dst->center.z = src->center[2];
	dst->extents.x = src->extents[0];
	dst->extents.y = src->extents[1];
	dst->extents.z = src->extents[2];
}
