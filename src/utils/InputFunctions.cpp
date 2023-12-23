#include "managers/animation/AnimationManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/CrushManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/InputFunctions.hpp"
#include "managers/Attributes.hpp"
#include "managers/highheel.hpp"
#include "utils/actorUtils.hpp"
#include "data/persistent.hpp"
#include "managers/Rumble.hpp"
#include "data/transient.hpp"
#include "data/runtime.hpp"
#include "data/plugin.hpp"
#include "scale/scale.hpp"
#include "data/time.hpp"
#include "utils/av.hpp"
#include "timer.hpp"

using namespace RE;
using namespace Gts;


namespace {
	void RapidGrowthEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!Runtime::HasPerk(player, "TotalControl")) {
			return;
		}
		if (IsCrawling(player) || !player->IsSneaking()) {
			AnimationManager::StartAnim("TriggerGrowth", player);
		}
	}
	void RapidShrinkEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		if (!Runtime::HasPerk(player, "TotalControl")) {
			return;
		}
		if (IsCrawling(player) || !player->IsSneaking()) {
			AnimationManager::StartAnim("TriggerShrink", player);
		}
	}

	void SizeReserveEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto Cache = Persistent::GetSingleton().GetData(player);
		if (!Cache) {
			return;
		}
		if (Cache->SizeReserve > 0.0) {
			float duration = data.Duration();
			GRumble::Once("SizeReserve", player, Cache->SizeReserve/15 * duration, 0.05);

			if (duration >= 1.2 && Runtime::HasPerk(player, "SizeReserve") && Cache->SizeReserve > 0) {
				float SizeCalculation = duration - 1.2;
				float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(player)/100;
				float Volume = clamp(0.10, 2.0, get_visual_scale(player) * Cache->SizeReserve/10);
				static Timer timergrowth = Timer(2.00);
				if (timergrowth.ShouldRunFrame()) {
					Runtime::PlaySoundAtNode("growthSound", player, Cache->SizeReserve/50 * duration, 1.0, "NPC Pelvis [Pelv]");
					Runtime::PlaySoundAtNode("MoanSound", player, Volume, 0.0, "NPC Head [Head]");
				}

				update_target_scale(player, SizeCalculation/80 * gigantism, false);
				Cache->SizeReserve -= SizeCalculation/80;
				if (Cache->SizeReserve <= 0) {
					Cache->SizeReserve = 0.0; // Protect against negative values.
				}
			}
		}
	}

	void DisplaySizeReserveEvent(const InputEventData& data) {
		auto player = PlayerCharacter::GetSingleton();
		auto Cache = Persistent::GetSingleton().GetData(player);
		if (Cache) {
			if (Runtime::HasPerk(player, "SizeReserve")) { //F
				float gigantism = 1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(player)/100;
				float Value = Cache->SizeReserve * gigantism;
				Notify("Reserved Size: {:.2f}", Value);
			}
		}
	}

	void PartyReportEvent(const InputEventData& data) {
		for (auto actor: find_actors()) {
			if (actor->formID != 0x14 && Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) {
				float hh = HighHeelManager::GetBaseHHOffset(actor)[2]/100;
				float gigantism = SizeManager::GetSingleton().GetEnchantmentBonus(actor)/100;
				float naturalscale = get_natural_scale(actor);
				float scale = get_visual_scale(actor);
				float maxscale = get_max_scale(actor) * naturalscale;
				Cprint("{} Height: {:.2f} m / {:.2f} ft; Weight: {:.2f} kg / {:.2f} lb;", actor->GetDisplayFullName(), GetActorHeight(actor, true), GetActorHeight(actor, false), GetActorWeight(actor, true), GetActorWeight(actor, false));
				Cprint("{} Scale: {:.2f}  (Natural Scale: {:.2f}; Size Limit: {:.2f}; Aspect Of Giantess: {:.2f})", actor->GetDisplayFullName(), scale, naturalscale, maxscale, gigantism);
				if (hh > 0.0) { // if HH is > 0, print HH info
					Cprint("{} High Heels: {:.2f} (+{:.2f} cm / +{:.2f} ft)", actor->GetDisplayFullName(), hh, hh, hh*3.28);
				}
			}
		}
	}

	bool ShouldTimerRun(Actor* actor) {
		static Timer ExplosionTimer_Normal = Timer(12);
		static Timer ExplosionTimer_Perk = Timer(8);
		bool DarkArts3 = Runtime::HasPerk(actor, "DarkArts_Aug3");
		if (DarkArts3) {
			return ExplosionTimer_Perk.ShouldRunFrame();
		} else {
			return ExplosionTimer_Normal.ShouldRunFrame();
		}
	}

	void ShrinkOutburstEvent(const InputEventData& data) {

		auto player = PlayerCharacter::GetSingleton();
		bool DarkArts = Runtime::HasPerk(player, "DarkArts");
		if (!DarkArts) {
			return; // no perk, do nothing
		}

		bool DarkArts2 = Runtime::HasPerk(player, "DarkArts_Aug2");
		bool DarkArts3 = Runtime::HasPerk(player, "DarkArts_Aug3");

		float gigantism = std::clamp(1.0f + SizeManager::GetSingleton().GetEnchantmentBonus(player)*0.01f, 1.0f, 20.0f);

		float multi = AttributeManager::GetSingleton().GetAttributeBonus(player, ActorValue::kHealth);

		float healthMax = GetMaxAV(player, ActorValue::kHealth);
		float healthCur = GetAV(player, ActorValue::kHealth);
		float damagehp = 80.0;

		if (DarkArts2) {
			damagehp -= 10; // less hp drain
		}
		if (DarkArts3) {
			damagehp -= 10; // even less hp drain
		}

		damagehp *= multi;
		damagehp /= gigantism;

		if (healthCur < damagehp * 1.10) {
			Notify("Your health is too low");
			return; // don't allow us to die from own shrinking
		}

		static Timer NotifyTimer = Timer(2.0);

		if (!ShouldTimerRun(player)) {
			if (NotifyTimer.ShouldRunFrame()) {
				Runtime::PlaySound("VoreSound_Fail", player, 1.2, 0.0);
				Notify("Shrink Outburst is on a cooldown");
			}
			return;
		}
		DamageAV(player, ActorValue::kHealth, damagehp);
		ShrinkOutburstExplosion(player, false);
	}

	void AnimSpeedUpEvent(const InputEventData& data) {
		AnimationManager::AdjustAnimSpeed(0.045); // Increase speed and power
	}
	void AnimSpeedDownEvent(const InputEventData& data) {
		AnimationManager::AdjustAnimSpeed(-0.060); // Decrease speed and power
	}
	void AnimMaxSpeedEvent(const InputEventData& data) {
		AnimationManager::AdjustAnimSpeed(0.090); // Strongest attack speed buff
	}
}

namespace Gts
{
	void InputFunctions::RegisterEvents() {
		InputManager::RegisterInputEvent("SizeReserve", SizeReserveEvent);
		InputManager::RegisterInputEvent("DisplaySizeReserve", DisplaySizeReserveEvent);
		InputManager::RegisterInputEvent("PartyReport", PartyReportEvent);
		InputManager::RegisterInputEvent("AnimSpeedUp", AnimSpeedUpEvent);
		InputManager::RegisterInputEvent("AnimSpeedDown", AnimSpeedDownEvent);
		InputManager::RegisterInputEvent("AnimMaxSpeed", AnimMaxSpeedEvent);
		InputManager::RegisterInputEvent("RapidGrowth", RapidGrowthEvent);
		InputManager::RegisterInputEvent("RapidShrink", RapidShrinkEvent);
		InputManager::RegisterInputEvent("ShrinkOutburst", ShrinkOutburstEvent);
	}
}