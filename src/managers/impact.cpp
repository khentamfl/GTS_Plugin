#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/explosion.hpp"
#include "managers/modevent.hpp"
#include "managers/footstep.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "managers/impact.hpp"
#include "managers/tremor.hpp"
#include "data/runtime.hpp"
#include "UI/DebugAPI.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "events.hpp"


#include "node.hpp"

using namespace SKSE;
using namespace RE;
using namespace Gts;

namespace {

	FootEvent get_foot_kind(Actor* actor, std::string_view tag) {
		auto profiler = Profilers::Profile("Impact: Get Foot Kind");
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
		auto profiler = Profilers::Profile("Impact: Get Landing Nodes");
		std::vector<NiAVObject*> results;
		const std::string_view left_foot = "NPC L Foot [Lft ]";
		const std::string_view right_foot = "NPC R Foot [Rft ]";
		const std::string_view left_arm = "NPC L Hand [LHnd]";
		const std::string_view right_arm = "NPC R Hand [LHnd]";

		NiAVObject* result;
		switch (foot_kind) {
			case FootEvent::Left:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Right:
				result = find_node(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Front:
				result = find_node(actor, left_arm);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_arm);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::Back:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_foot);
				if (result) {
					results.push_back(result);
				}
				break;
			case FootEvent::JumpLand:
				result = find_node(actor, left_foot);
				if (result) {
					results.push_back(result);
				}
				result = find_node(actor, right_foot);
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
			auto profiler = Profilers::Profile("Impact: HookProcess");
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
			if (actor->AsActorState()->IsWalking()) {
				bonus = 0.75;
			}
			if (actor->IsSneaking()) {
				bonus *= 0.5;
			}
			if (actor->AsActorState()->IsSprinting()) {
				bonus *= 1.50;
				if (Runtime::HasPerkTeam(actor, "LethalSprint")) {
					bonus *= 1.5;
				}
			}

			if (kind != FootEvent::JumpLand) {
				if (kind == FootEvent::Left) {
					DoDamageEffect(actor, 1.25, 1.65 * bonus, 25, 0.25, kind, 1.25, DamageSource::CrushedLeft);
				} if (kind == FootEvent::Right) {
					DoDamageEffect(actor, 1.25, 1.65 * bonus, 25, 0.25, kind, 1.25, DamageSource::CrushedRight);
				}
				//                     ^          ^
				//                 Damage         Radius
				DoLaunch(actor, 0.70 * bonus, 1.0 * bonus, kind);
				//               ^ radius      ^ push power
				return; // don't check further
			} 
			
			else if (kind == FootEvent::JumpLand) {
				float perk = GetPerkBonus_Basics(actor);
				auto& sizemanager = SizeManager::GetSingleton();
				float damage = sizemanager.GetSizeAttribute(actor, 2);

				DoDamageEffect(actor, 1.80 * damage, 6.6, 20, 0.25, FootEvent::Left, 1.0, DamageSource::CrushedLeft);
				DoDamageEffect(actor, 1.80 * damage, 6.6, 20, 0.25, FootEvent::Right, 1.0, DamageSource::CrushedRight);

				DoLaunch(actor, 0.9 * perk, 1.75, FootEvent::Left);
				DoLaunch(actor, 0.9 * perk, 1.75, FootEvent::Right);
			}
		}
	}
}
