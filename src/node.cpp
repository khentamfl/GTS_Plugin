#include "node.hpp"
#include "util.hpp"
#include <regex>

using namespace SKSE;
using namespace Gts;

namespace Gts {
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

	NiAVObject* find_node(Actor* actor, std::string_view node_name, Person person) {
		if (!actor->Is3DLoaded()) {
			return nullptr;
		}
		std::vector<NiAVObject*> models;
		switch (person) {
			case Person::First: {
				auto model = actor->Get3D(true);
				if (model) {
					models.push_back(model);
				}
			}
			case Person::Third: {
				auto model = actor->Get3D(false);
				if (model) {
					models.push_back(model);
				}
			}
			case Person::Any: {
				auto model = actor->Get3D(false);
				if (model) {
					models.push_back(model);
				}
				auto modelb = actor->Get3D(true);
				if (modelb) {
					models.push_back(modelb);
				}
			}
			case Person::Current: {
				auto model = actor->GetCurrent3D();
				if (model) {
					models.push_back(model);
				}
			}
		}
		if (models.empty()) {
			return nullptr;
		}

		for (auto model: models) {
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
		}

		return nullptr;
	}

	NiAVObject* find_node_regex(Actor* actor, std::string_view node_regex, Person person) {
		if (!actor->Is3DLoaded()) {
			return nullptr;
		}
		std::vector<NiAVObject*> models;
		switch (person) {
			case Person::First: {
				auto model = actor->Get3D(true);
				if (model) {
					models.push_back(model);
				}
			}
			case Person::Third: {
				auto model = actor->Get3D(false);
				if (model) {
					models.push_back(model);
				}
			}
			case Person::Any: {
				auto model = actor->Get3D(false);
				if (model) {
					models.push_back(model);
				}
				auto modelb = actor->Get3D(true);
				if (modelb) {
					models.push_back(modelb);
				}
			}
			case Person::Current: {
				auto model = actor->GetCurrent3D();
				if (model) {
					models.push_back(model);
				}
			}
		}
		if (models.empty()) {
			return nullptr;
		}

		std::regex the_regex(std::string(node_regex).c_str());

		for (auto model: models) {
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
						if  (std::regex_match(currentnode->name.c_str(), the_regex)) {
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
		}
		return nullptr;
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
		auto model = actor->Get3D(false);
		if (model) {
			auto extra_bbx = model->GetExtraData("BBX");
			if (extra_bbx) {
				BSBound* bbx = static_cast<BSBound*>(extra_bbx);
				return bbx;
			}
		}
		auto model_first = actor->Get3D(true);
		if (model_first) {
			auto extra_bbx = model_first->GetExtraData("BBX");
			if (extra_bbx) {
				BSBound* bbx = static_cast<BSBound*>(extra_bbx);
				return bbx;
			}
		}
		return nullptr;
	}

	NiAVObject* get_bumper(Actor* actor) {
		string node_name = "CharacterBumper";
		return find_node(actor, node_name, Person::Third);
	}

	void update_node(NiAVObject* node) {
		if (node) {
			if (on_mainthread()) {
				NiUpdateData ctx;
				node->UpdateWorldData(&ctx);
			} else {
				node->IncRefCount();
				auto task = SKSE::GetTaskInterface();
				task->AddTask([node]() {
					if (node) {
						NiUpdateData ctx;
						node->UpdateWorldData(&ctx);
						node->DecRefCount();
					}
				});
			}
		}
	}
}
