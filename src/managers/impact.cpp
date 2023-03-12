#include "managers/impact.hpp"
#include "managers/footstep.hpp"
#include "managers/highheel.hpp"
#include "managers/explosion.hpp"
#include "managers/tremor.hpp"
#include "managers/modevent.hpp"
#include "data/runtime.hpp"
#include "events.hpp"
#include "UI/DebugAPI.hpp"


#include "scale/scale.hpp"
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

			const float BASE_DISTANCE = 75.0; // Checks the distance of the tiny against giant. Should be large to encompass giant's general area
			const float BASE_FOOT_DISTANCE = 40.0; // Checks the distance of foot squishing
			const float SCALE_RATIO = 3.0;
			float bonusscale = 1.0;
			if (!impact_data.nodes.empty() && actor != nullptr) {

				float actualGiantScale = get_visual_scale(actor);
				float giantScale = actualGiantScale;

				if (actor->IsSneaking()) {
					giantScale *= 0.5;
				}
				if (actor->AsActorState()->IsSprinting()) {
					giantScale *= 1.5;
				}
				if (impact_data.kind == FootEvent::JumpLand) {
					giantScale *= 2.0;
				}

				if (Runtime::HasMagicEffect(actor, "SmallMassiveThreat")) {
					giantScale *= 2.0;
				}

				NiPoint3 hhOffset = HighHeelManager::GetHHOffset(actor);
				std::vector<NiPoint3> points = {
					NiPoint3(0.0, 0.0, 0.0), // The standard at the foot position
					NiPoint3(0.5, 2.0, 7.5),
				};
				float maxFootDistance = BASE_FOOT_DISTANCE * giantScale;

				for (auto foot: impact_data.nodes) {
					// Make a list of points to check
					std::vector<NiPoint3> footPoints = {};
					for (NiPoint3 point:  points) {
						footPoints.push_back(foot->world*point);
						if (hhOffset.Length() > 1e-4) {
							footPoints.push_back(foot->world*(point-hhOffset)); // Add HH offsetted version
						}
					}
					for (auto otherActor: find_actors()) {
						if (otherActor != actor) {
							float tinyScale = get_visual_scale(otherActor);
							if (giantScale / tinyScale > SCALE_RATIO) {
								NiPoint3 actorLocation = otherActor->GetPosition();
								// Check the tiny's nodes against the giant's foot points
								for (auto point: footPoints) {
									float distance = (point - actorLocation).Length();
									if (distance < maxFootDistance) {
										// Under Foot
										float aveForce = 1.0 - distance / maxFootDistance;
										UnderFoot underfoot = UnderFoot {
											.giant = impact_data.actor,
											.tiny = otherActor,
											.force = aveForce,
											.foot = foot,
											.bodyParts = {otherActor->GetCurrent3D()},
											.footEvent = kind,
										};
										EventDispatcher::DoUnderFootEvent(underfoot);
										break;
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
