#include "magic/effects/smallmassivethreat.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "magic/magic.hpp"
#include "scale/scale.hpp"
#include "node.hpp"

namespace {
	float GetSMTBonus(Actor* actor) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			return transient->SMT_Bonus_Duration;
		}
		return 0.0;
	}

	float GetSMTPenalty(Actor* actor) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			return transient->SMT_Penalty_Duration;
		}
		return 0.0;
	}

	void NullifySMTDuration(Actor* actor) {
		auto transient = Transient::GetSingleton().GetData(actor);
		if (transient) {
			transient->SMT_Bonus_Duration = 0.0;
			transient->SMT_Penalty_Duration = 0.0;
		}
	}
}

namespace Gts {

	std::string SmallMassiveThreat::GetName() {
		return "SmallMassiveThreat";
	}

	void SmallMassiveThreat::OnStart() {
		//std::string message = std::format("While Tiny Calamity is active, your size-related actions are massively empowered, but your max scale is limited. You can perform all size-related actions (Vore, Grab, Hug Crush, etc) while being same size, but performing them wastes some of Tiny Calamity's duration.");
		//TutorialMessage(message, "Calamity");
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		Runtime::PlaySoundAtNode("TinyCalamitySound", caster, 1.0, 1.0, "NPC COM [COM ]");
		auto node = find_node(caster, "NPC Root [Root]");
		if (node) {
			float scale = get_visual_scale(caster);
			NiPoint3 position = node->world.translate;
			SpawnParticle(caster, 6.00, "GTS/Effects/TinyCalamity.nif", NiMatrix3(), position, scale * 3.0, 7, nullptr); // Spawn
			SpawnParticle(caster, 6.00, "GTS/Effects/TinyCalamity.nif", NiMatrix3(), position, scale * 3.4, 7, nullptr); // Spawn
			SpawnParticle(caster, 6.00, "GTS/Effects/TinyCalamity.nif", NiMatrix3(), position, scale * 3.8, 7, nullptr); // Spawn
			Rumble::For("TinyCalamity", caster, 32.0, 0.12, "NPC COM [COM ]", 0.10);
		}
	}

	void SmallMassiveThreat::OnUpdate() {
		const float BASE_POWER = 0.00035;
		const float DUAL_CAST_BONUS = 2.0;
		auto caster = GetCaster();
		if (!caster) {
			return;
		}
		static Timer warningtimer = Timer(3.0);
		float CasterScale = get_target_scale(caster);
		float bonus = GetSMTBonus(caster);
		float penalty = GetSMTPenalty(caster);
		if (bonus > 0.5) {
			GetActiveEffect()->duration += bonus;

			NullifySMTDuration(caster);
		}
		if (penalty > 0.5) {
			GetActiveEffect()->duration -= penalty;
			NullifySMTDuration(caster);
		}
		if (CasterScale < get_natural_scale(caster))  {// Disallow to be smaller than natural scale to avoid weird interactions with others
			mod_target_scale(caster, 0.0030);
		}
		else if (CasterScale >= 1.50) {
			mod_target_scale(caster, -0.0300);
			if (warningtimer.ShouldRun() && caster->formID == 0x14) {
				Notify("Im getting too big, it becomes hard to handle such power.");
			}
		} // <- Disallow having it when scale is > 1.50
	}
}
