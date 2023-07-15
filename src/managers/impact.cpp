#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/explosion.hpp"
#include "managers/modevent.hpp"
#include "managers/footstep.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "managers/impact.hpp"
#include "managers/tremor.hpp"
#include "data/runtime.hpp"
#include "UI/DebugAPI.hpp"
#include "events.hpp"


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
				.nodes = get_landing_nodes(actor, kind),
			};

			EventDispatcher::DoOnImpact(impact_data); // Calls Explosions and sounds. A Must.

			float bonus = 1.0;

			if (actor->IsSneaking()) {
				bonus *= 0.5;
			} if (actor->AsActorState()->IsSprinting()) {
				bonus *= 1.75;
				if (Runtime::HasPerkTeam(actor, "LethalSprint")) {
					bonus *= 2.0;
				}
			} if (HasSMT(actor)) {
				bonus += 1.0;
			}
			if (kind != FootEvent::JumpLand) { // We already do it for Jump Land inside Compat.cpp. We do NOT want to apply it for Jump Land because of it!
				DoDamageEffect(actor, 1.75, 1.65 * bonus, 25, 0.035, kind);
				DoLaunch(actor, 0.85 * bonus, 2.0, 1.0, kind);
			}
		}
	}
}
