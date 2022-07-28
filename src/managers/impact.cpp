#include "managers/impact.h"
#include "managers/footstep.h"
#include "managers/explosion.h"
#include "managers/tremor.h"
#include "managers/modevent.h"

#include "scale/scale.h"
#include "util.h"
#include "node.h"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {
	Foot get_foot_kind(Actor* actor, std::string_view tag) {
		Foot foot_kind = Foot::Unknown;
		bool is_jumping = actor ? IsJumping(actor) : false;
		bool in_air = actor ? actor->IsInMidair() : false;
		if (matches(tag, ".*Foot.*Left.*") && !is_jumping && !in_air) {
			foot_kind = Foot::Left;
		} else if (matches(tag, ".*Foot.*Right.*") && !is_jumping && !in_air) {
			foot_kind = Foot::Right;
		} else if (matches(tag, ".*Foot.*Front.*") && !is_jumping && !in_air) {
			foot_kind = Foot::Front;
		} else if (matches(tag, ".*Foot.*Back.*") && !is_jumping && !in_air) {
			foot_kind = Foot::Back;
		} else if (matches(tag, ".*Jump.*(Down|Land).*")) {
			foot_kind = Foot::JumpLand;
		}
		return foot_kind;
	}

	std::std::vector<NiAVObject*> get_landing_nodes(Actor* actor, const Foot& foot_kind) {
		std::std::vector<NiAVObject*> results;
		const std::string_view left_foot = ".*(L.*Foot|L.*Leg.*Tip).*";
		const std::string_view right_foot = ".*(R.*Foot|R.*Leg.*Tip).*";
		const std::string_view left_arm = ".*(L.*Foot|L.*Leg.*Tip).*";
		const std::string_view right_arm = ".*(R.*Hand|R.*Arm.*Tip).*";

		NiAVObject* result;
		switch (foot_kind) {
			case Foot::Left:
				result = find_node_regex_any(actor, left_foot);
				if (result) results.push(result);
				break;
			case Foot::Right:
				result = find_node_regex_any(actor, right_foot);
				if (result) results.push(result);
				break;
			case Foot::Front:
				result = find_node_regex_any(actor, left_arm);
				if (result) results.push(result);
				result = find_node_regex_any(actor, right_arm);
				if (result) results.push(result);
				break;
			case Foot::Back:
				result = find_node_regex_any(actor, left_foot);
				if (result) results.push(result);
				result = find_node_regex_any(actor, right_foot);
				if (result) results.push(result);
				break;
			case Foot::JumpLand:
				result = find_node_regex_any(actor, left_foot);
				if (result) results.push(result);
				result = find_node_regex_any(actor, right_foot);
				if (result) results.push(result);
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

			Impact impact_data = Impact {
				.actor = actor,
				.kind = get_foot_kind(actor, tag),
				.scale = get_visual_scale(actor),
				.effective_scale = get_effective_scale(actor),
				.nodes = get_landing_nodes(actor, foot_kind),
			};

			FootStepManager::GetSingleton().OnImpact(impact_data);
			ExplosionManager::GetSingleton().OnImpact(impact_data);
			TremorManager::GetSingleton().OnImpact(impact_data);
		}
	}
}
