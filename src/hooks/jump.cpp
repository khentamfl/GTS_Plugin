#include "hooks/jump.hpp"
#include "hooks/callhook.hpp"
#include "scale/scale.hpp"
#include "data/plugin.hpp"
#include "managers/Attributes.hpp"

using namespace RE;
using namespace SKSE;

namespace {
	Actor* FindActor(bhkCharacterController* charCont) {
		for (auto actor: find_actors()) {
			if (charCont == actor->GetCharController()) {
				return actor;
			}
		}
		return nullptr;
	}

	float FallModifier(float scale) {
		float k = 1.0;
		float a = 0.0;
		float n = 0.75;
		float s = 1.0;
		// https://www.desmos.com/calculator/ygoxbe7hjg
		float result = k*pow(s*(scale-a), n);
		log::info("Result: {}, scale {}", result, scale);
		if (scale < 1.0) {
			return 1.0;
		}
		return result;
	}
}

namespace Hooks {

	void Hook_Jumping::Hook(Trampoline& trampoline) {


		static FunctionHook<float(bhkCharacterController* a_this)> GetFallDistance(
			REL::RelocationID(76430, 78269),
			[](auto* a_this){
			float result = GetFallDistance(a_this);
			// log::info("GetFallDistance: {}", result);
			auto actor = FindActor(a_this);
			if (actor) {
				if (actor->formID == 0x14) {// Apply to Player only
					float scale = get_giantess_scale(actor);
					if (scale > 1e-4) {
						result /= scale;
						//log::info("  - Changed to {} for {}", result, actor->GetDisplayFullName());
					}
				}
			}

			return result;
			}
		);

		// AE 1402bc7c3
		// SE 1402aa40c
		//
		// Is used in the jump anim event handler
		//
		//REL::Relocation<uintptr_t> hook{REL::RelocationID(41811, 42892)};
		//_GetScaleJumpHook = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x4d, 0x4d), GetScaleJumpHook);

		static FunctionHook<bool(IAnimationGraphManagerHolder* graph, const BSFixedString& a_variableName, const float a_in)> SkyrimSetGraphVarFloat( 
			REL::RelocationID(32143, 32887),
			[](auto* graph, const auto& a_variableName, auto a_in) {
				if (a_variableName == "VelocityZ") {
					if (a_in < 0) {
						log::info("Found Velocity: {}", a_in);
						auto actor = skyrim_cast<Actor*>(graph);
						if (actor) {
							float new_jump = AttributeManager::GetSingleton().GetJumpHeight(actor);
							float jump_factor = pow(76.0/new_jump, 0.5);
							if (actor->formID == 0x14) {
								log::info("Dividing velocity by scale: {}", jump_factor);
								log::info("Actor: {}", actor->GetDisplayFullName());
							}
							if (actor->formID == 0x14) {
								a_in *= jump_factor;
								log::info(" new V: {}", a_in);
							}
						}
					}
				}
				return SkyrimSetGraphVarFloat(graph, a_variableName, a_in);
			});



		
		static CallHook<float(Actor*)> SkyrimJumpHeight(RELOCATION_ID(36271, 37257),  REL::Relocate(0xBE, 0x17F),
		// SE: 0x1405d2110 -0 x1405d1f80   = 0x190 (offset) .  36271 = 5D1F80
		[](auto* actor) {
		    float result = SkyrimJumpHeight(actor);
			log::info("Original jump height: {}", result);
		    if (actor) {
		      if (actor->formID == 0x14) {
				log::info("form id is of the player");
			  }
			  log::info("Raw Name: {}", GetRawName(actor));
			  log::info("Value: {}", result);
		    }
		    return result;
		});
	}

	

	/*float Hook_Jumping::GetScaleJumpHook(TESObjectREFR* a_this) {
		float result = _GetScaleJumpHook(a_this);
		Actor* actor = skyrim_cast<Actor*>(a_this);
		if (actor) {
			float scale = get_visual_scale(actor);
			if (scale > 1e-4) {
				log::info("Jump Hook: {} for {}", scale, actor->GetDisplayFullName());
				result *= scale;
			}
		}
		return result;
	}*/
}
