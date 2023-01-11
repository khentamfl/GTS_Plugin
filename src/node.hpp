#pragma once
// Module that finds nodes and node realated data
using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	void walk_nodes(Actor* actor);
	NiAVObject* find_node(Actor* actor, std::string_view node_name, bool first_person = false);
	NiAVObject* find_node_regex(Actor* actor, std::string_view node_regex, bool first_person = false);
	NiAVObject* find_node_any(Actor* actor, std::string_view node_name);
	NiAVObject* find_node_regex_any(Actor* actor, std::string_view node_regex);
	void scale_hkpnodes(Actor* actor, float prev_scale, float new_scale);
	void clone_bound(Actor* actor);
	BSBound* get_bound(Actor* actor);
	NiAVObject* get_bumper(Actor* actor);

	std::vector<NiAVObject*> GetModelsForSlot(Actor* actor, BGSBipedObjectForm::BipedObjectSlot slot);
	void VisitNodes(NiAVObject* root, std::function<void(NiAVObject& a_obj)> a_visitor);
	template<class T>
	void VisitExtraData(NiAVObject* root, std::string_view name, std::function<void(NiAVObject& a_obj, T& data)> a_visitor) {
		VisitNodes(root, [](node) {
			NiExtraData* extraData = root.GetExtraData(name);
			if (extraData) {
				T* targetExtraData = netimmerse_cast<T*>(extraData);
				if (targetExtraData) {
					a_visitor(node, *targetExtraData);
				}
			}
		});
	}

	void update_node(NiAVObject* node);
}
