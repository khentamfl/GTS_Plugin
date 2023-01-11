#include "managers/highheel.hpp"
#include "data/runtime.hpp"
#include "node.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "util.hpp"
#include "actorUtils.hpp"
#include "scale/scale.hpp"
#include <articuno/articuno.h>
#include <articuno/archives/ryml/ryml.h>
#include <articuno/types/auto.h>

using namespace articuno;
using namespace articuno::ryml;
using namespace RE;
using namespace Gts;

namespace Gts {
	HighHeelManager& HighHeelManager::GetSingleton() noexcept {
		static HighHeelManager instance;
		return instance;
	}

	std::string HighHeelManager::DebugName() {
		return "HighHeelManager";
	}

	void HighHeelManager::PapyrusUpdate() {
		const bool FORCE_APPLY = false;
		auto actors = find_actors();
		for (auto actor: actors) {
			//ApplyHH(actor, FORCE_APPLY);
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

		NiPoint3 new_hh = this->GetHHOffset(actor);
		if (IsProne(actor)) {
			new_hh = NiPoint3();
		}
		float hh_length = new_hh.Length();

		for (bool person: {false, true}) {
			auto npc_root_node = find_node(actor, "NPC", person);

			if (npc_root_node) {
				NiPoint3 current_value = npc_root_node->local.translate;
				NiPoint3 delta = current_value - new_hh;

				if (delta.Length() > 1e-5 || force) {
					npc_root_node->local.translate = new_hh;
					update_node(npc_root_node);
				}
			}
		}
	}


	struct RaceMenuSDTA {
		std::string name;
		std::vector<float> pos;
		articuno_serde(ar) {
			ar <=> kv(name, "name");
			ar <=> kv(pos, "pos");
		}
	};

	NiPoint3 HighHeelManager::GetBaseHHOffset(Actor* actor) {
		auto models = GetModelsForSlot(actor, BGSBipedObjectForm::BipedObjectSlot::kFeet);
		NiPoint3 result = NiPoint3();
		for (auto model: models) {
			if (model) {
				VisitExtraData<NiFloatExtraData>(model, "HH_OFFSET", [&result](NiAVObject& currentnode, NiFloatExtraData& data) {
					result.z = fabs(data.value);
					return false;
				});
				VisitExtraData<NiStringExtraData>(model, "SDTA", [&result](NiAVObject& currentnode, NiStringExtraData& data) {
					std::string stringDataStr = data.value;
					std::stringstream jsonData(stringDataStr);
					yaml_source ar(jsonData);
					vector<RaceMenuSDTA> alterations;
					ar >> alterations;
					for (auto alteration: alterations) {
						if (alteration.name == "NPC") {
							if (alteration.pos.size() > 2) {
								result = NiPoint3(alteration.pos[0], alteration.pos[1], alteration.pos[2]);
								return false;
							}
						}
					}
					return true;
				});
			}
		}
		return result;
	}

	NiPoint3 HighHeelManager::GetHHOffset(Actor* actor) {
		if (actor) {
			auto scale = get_visual_scale(actor);
			return HighHeelManager::GetBaseHHOffset(actor) * scale;
		}
		return NiPoint3();
	}

	bool HighHeelManager::IsWearingHH(Actor* actor) {
		return HighHeelManager::GetBaseHHOffset(actor).Length() > 1e-3;
	}
}
