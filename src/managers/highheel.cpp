#include "managers/highheel.hpp"
#include "data/runtime.hpp"
#include "node.hpp"
#include "scale/modscale.hpp"
#include "managers/GtsManager.hpp"
#include "data/persistent.hpp"
#include "scale/scale.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "profiler.hpp"
#include <nlohmann/json.hpp>

using json = nlohmann::json;
using namespace RE;
using namespace Gts;

namespace {
	bool DisableHighHeels(Actor* actor) {
		bool disable = (
			AnimationManager::HHDisabled(actor) || !Persistent::GetSingleton().highheel_correction ||
			BehaviorGraph_DisableHH(actor) || IsCrawling(actor) || 
			IsProning(actor)
		);
		return disable;
	}
}

namespace Gts {
	HighHeelManager& HighHeelManager::GetSingleton() noexcept {
		static HighHeelManager instance;
		return instance;
	}

	std::string HighHeelManager::DebugName() {
		return "HighHeelManager";
	}

	void HighHeelManager::HavokUpdate() {
		auto profiler = Profilers::Profile("HH: HavokUpdate");
		auto actors = FindSomeActors("HHHavokUpdate", 1);
		for (auto actor: actors) {
			ApplyHH(actor, false);
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

	void HighHeelManager::OnAddPerk(const AddPerkEvent& evt) {
		//log::info("Add Perk fired");
		if (evt.perk == Runtime::GetPerk("hhBonus")) {
			for (auto actor: find_actors()) {
				if (actor) {
					log::info("HH perk was added");
					this->data.try_emplace(actor);
					auto& hhData = this->data[actor];
					hhData.wasWearingHh = false;
				}
			}
		}
	}

	void HighHeelManager::ApplyHH(Actor* actor, bool force) {
		auto profiler = Profilers::Profile("HH: ApplyHH");
		if (!actor) {
			return;
		}
		if (!actor->Is3DLoaded()) {
			return;
		}

		if (Persistent::GetSingleton().highheel_furniture == false && actor->AsActorState()->GetSitSleepState() == SIT_SLEEP_STATE::kIsSitting) {
			return;
		}
		this->data.try_emplace(actor);
		auto& hhData = this->data[actor];
		float speedup = 1.0;
		if (IsCrawling(actor) || IsProning(actor) || BehaviorGraph_DisableHH(actor)) {
			speedup = 4.0; // To shift down a lot faster
		} else if (!IsGtsBusy(actor)) {
			speedup = 3.0;
		}
		// Should disable HH?
		bool disableHH = DisableHighHeels(actor);

		if (disableHH) {
			hhData.multiplier.target = 0.0;
			hhData.multiplier.halflife = 1 / (AnimationManager::GetAnimSpeed(actor) * AnimationManager::GetHighHeelSpeed(actor) * speedup);
		} else {
			hhData.multiplier.target = 1.0;
			hhData.multiplier.halflife = 1 / (AnimationManager::GetAnimSpeed(actor) * AnimationManager::GetHighHeelSpeed(actor) * speedup);
		}

		NiPoint3 new_hh;
		this->UpdateHHOffset(actor);
		if (Persistent::GetSingleton().size_method != SizeMethod::ModelScale) {
			new_hh = this->GetHHOffset(actor) * hhData.multiplier.value;
		} else {
			// With model scale do it in unscaled coords
			new_hh = this->GetBaseHHOffset(actor) * hhData.multiplier.value;
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
				bool wasWearingHh = hhData.wasWearingHh;
				bool isWearingHH = fabs(new_hh.Length()) > 1e-4;
				if (isWearingHH != wasWearingHh) {
					// Just changed hh
					HighheelEquip hhEvent = HighheelEquip {
						.actor = actor,
						.equipping = isWearingHH,
						.hhLength = new_hh.Length(),
						.hhOffset = new_hh,
						.shoe = actor->GetWornArmor(BGSBipedObjectForm::BipedObjectSlot::kFeet),
					};
					EventDispatcher::DoHighheelEquip(hhEvent);
					hhData.wasWearingHh = isWearingHH;
				}
			}
		}
	}


	void HighHeelManager::UpdateHHOffset(Actor* actor) {
		auto profiler = Profilers::Profile("HH: UpdateHHOffset");
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
					//std::stringstream jsonData(stringDataStr);
					//json j = json::parse(jsonData);
					log::info("STD Data: {}", stringDataStr);
					/*for (const auto& alteration: stringDataStr) {
						if (alteration.contains("name") && alteration.contains("pos") && alteration["name"] == "NPC" && alteration["pos"].size() > 2) {
							auto p = alteration["pos"].template get<std::vector<float> >();
							result = NiPoint3(p[0], p[1], p[2]);
							return false;
						}
					}*/
					return true;
				});
			}
		}
		//log::info("Base HHOffset: {}", Vector2Str(result));
		auto npcNodeScale = get_npcparentnode_scale(actor);

		auto& me = HighHeelManager::GetSingleton();
		me.data.try_emplace(actor);
		auto& hhData = me.data[actor];
		hhData.lastBaseHHOffset = result * npcNodeScale;
	}

	NiPoint3 HighHeelManager::GetBaseHHOffset(Actor* actor) {
		auto profiler = Profilers::Profile("HH: GetBaseHHOffset");
		auto& me = HighHeelManager::GetSingleton();
		me.data.try_emplace(actor);
		auto& hhData = me.data[actor];
		return hhData.lastBaseHHOffset;
	}

	NiPoint3 HighHeelManager::GetHHOffset(Actor* actor) {
		auto profiler = Profilers::Profile("HH: GetHHOffset");
		auto npcRootNodeScale = get_npcnode_scale(actor);
		return HighHeelManager::GetBaseHHOffset(actor) * npcRootNodeScale;
	}

	bool HighHeelManager::IsWearingHH(Actor* actor) {
		return HighHeelManager::GetBaseHHOffset(actor).Length() > 1e-3;
	}
}
