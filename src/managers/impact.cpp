#include "managers/impact.hpp"
#include "managers/footstep.hpp"
#include "managers/explosion.hpp"
#include "managers/tremor.hpp"
#include "managers/modevent.hpp"
#include "events.hpp"

#include "scale/scale.hpp"
#include "util.hpp"
#include "node.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	FootEvent get_foot_kind(Actor* actor, std::string_view tag) {
		FootEvent foot_kind = FootEvent::Unknown;
		bool is_jumping = actor ? IsJumping(actor) : false;
		bool in_air = actor ? actor->IsInMidair() : false;
		if (matches(tag, ".*Foot.*Left.*") && !is_jumping && !in_air) {
			foot_kind = FootEvent::Left;
		} else if (matches(tag, ".*Foot.*Right.*") && !is_jumping && !in_air) {
			foot_kind = FootEvent::Right;
		} else if (matches(tag, ".*Foot.*Front.*") && !is_jumping && !in_air) {
			foot_kind = FootEvent::Front;
		} else if (matches(tag, ".*Foot.*Back.*") && !is_jumping && !in_air) {
			foot_kind = FootEvent::Back;
		} else if (matches(tag, ".*Jump.*(Down|Land).*")) {
			foot_kind = FootEvent::JumpLand;
		}
		return foot_kind;
	}

	std::vector<NiAVObject*> get_landing_nodes(Actor* actor, const FootEvent& foot_kind) {
		std::vector<NiAVObject*> results;
		const std::string_view left_foot = ".*(L.*Foot|L.*Leg.*Tip).*";
		const std::string_view right_foot = ".*(R.*Foot|R.*Leg.*Tip).*";
		const std::string_view left_arm = ".*(L.*Foot|L.*Leg.*Tip).*";
		const std::string_view right_arm = ".*(R.*Hand|R.*Arm.*Tip).*";

		NiAVObject* result;
		switch (foot_kind) {
			case FootEvent::Left:
				result = find_node_regex_any(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Right:
				result = find_node_regex_any(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Front:
				result = find_node_regex_any(actor, left_arm);
				if (result) {
					results.push_back(result);
				}
				result = find_node_regex_any(actor, right_arm);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Back:
				result = find_node_regex_any(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				result = find_node_regex_any(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::JumpLand:
				result = find_node_regex_any(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				result = find_node_regex_any(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
		}
		return results;
	}
}
namespace Gts {
	ImpactManager& ImpactManager::GetSingleton() noexcept {
		static ImpactManager instance;
		return instance;
	}

	void ImpactManager::HookProcessEvent(BGSImpactManager* impact, const BGSFootstepEvent* a_event, BSTEventSource<BGSFootstepEvent>* a_eventSource) {
		if (a_event) {
			auto actor = a_event->actor.get().get();
			if (actor) {
				auto base_actor = actor->GetActorBase();
			}
			std::string tag = a_event->tag.c_str();
			auto event_manager = ModEventManager::GetSingleton();
			event_manager.m_onfootstep.SendEvent(actor,tag);

			auto kind = get_foot_kind(actor, tag);
			Impact impact_data = Impact {
				.actor = actor,
				.kind = kind,
				.scale = get_visual_scale(actor),
				.effective_scale = get_effective_scale(actor),
				.nodes = get_landing_nodes(actor, kind),
			};

			EventDispatcher::DoOnImpact(impact_data);

			const float BASE_DISTANCE = 60.0;
			const float BASE_FOOT_DISTANCE = 10.0;
			const float SCALE_RATIO = 3.0;
			float bonusscale = 1.0;
			if (!impact_data.nodes.empty() && actor != nullptr) {

				float giantScale = get_visual_scale(actor);

				if (actor->IsSprinting()) {
					giantScale *= 1.5;
				}

				if (impact_data.nodes.results == FootEvent::JumpLand) {
					giantScale *= 2.0;
				}

				for (auto otherActor: find_actors()) {
					if (otherActor != actor) {
						float tinyScale = get_visual_scale(otherActor);
						if (giantScale / tinyScale > SCALE_RATIO) {
							NiPoint3 actorLocation = otherActor->GetPosition();
							for (auto foot: impact_data.nodes) {
								NiPoint3 footLocatation = foot->world.translate;
								//log::info("Checking {}'s is over {}", actor->GetDisplayFullName(), otherActor->GetDisplayFullName());
								float distance = (footLocatation - actorLocation).Length();
								if (distance < BASE_DISTANCE * giantScale) {
									log::info("  - Maybe");
									// Close enough for more advance checks
									auto model = otherActor->GetCurrent3D();
									if (model) {
										log::info("  - Model");
										std::vector<NiAVObject*> bodyParts = {};
										float force = 0.0;
										float footDistance = BASE_DISTANCE*giantScale;
										VisitNodes(model, [footLocatation, footDistance, &bodyParts, &force](NiAVObject& a_obj) {
											float distance = (a_obj.world.translate - footLocatation).Length();
											log::info("    - Distance of node from foot {} needs to be {}", distance, footDistance);
											if (distance < footDistance) {
												log::info("    - Passed");
												bodyParts.push_back(&a_obj);
												force += 1.0 - distance / footDistance;
											}
											log::info("Maybe two");
											return true;
										});
										if (!bodyParts.empty()) {
											log::info("  - Yes Underfoot");
											// Under Foot
											float aveForce = force / bodyParts.size();
											UnderFoot underfoot = UnderFoot {
												.giant = impact_data.actor,
												.tiny = otherActor,
												.force = aveForce,
												.foot = foot,
												.bodyParts = bodyParts,
												.footEvent = kind,
											};
											EventDispatcher::DoUnderFootEvent(underfoot);
										}
									}
								}
							}
						}
					}
				}
			}
		}
	}
}
