#include "hooks/jump.hpp"
#include "hooks/callhook.hpp"
#include "scale/scale.hpp"
#include "data/plugin.hpp"

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
		REL::Relocation<uintptr_t> hook{REL::RelocationID(41811, 42892)};
		_GetScaleJumpHook = trampoline.write_call<5>(hook.address() + RELOCATION_OFFSET(0x4d, 0x4d), GetScaleJumpHook);

		/*static FunctionHook<bool(IAnimationGraphManagerHolder* graph, const BSFixedString& a_variableName, const float a_in)> SkyrimSetGraphVarFloat( 
			REL::RelocationID(32143, 32887),
			[](auto* graph, const auto& a_variableName, auto a_in) {
				if (a_variableName == "VelocityZ") {
					if (a_in < 0) {
						log::info("Found Velocity: {}", a_in);
						auto actor = skyrim_cast<Actor*>(graph);
						if (actor) {
							auto scale = get_giantess_scale(actor);
							if (actor->formID == 0x14) {
								log::info("Dividing velocity by scale: {}", scale);
								log::info("Actor: {}", actor->GetDisplayFullName());
							}
							a_in /= scale;
							if (actor->formID == 0x14) {
								log::info(" new V: {}", a_in);
							}
						}
					}
				}
				return SkyrimSetGraphVarFloat(graph, a_variableName, a_in);
			});*/
	}

	float Hook_Jumping::GetScaleJumpHook(TESObjectREFR* a_this) {
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
	}
}
