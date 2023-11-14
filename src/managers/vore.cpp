#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/Utils/AnimationUtils.hpp"
#include "managers/ai/aifunctions.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/SurvivalMode.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "scale/modscale.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "profiler.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <cmath>
#include <random>

using namespace RE;
using namespace Gts;

namespace {
	const float MINIMUM_VORE_DISTANCE = 94.0;
	const float MINIMUM_VORE_SCALE_RATIO = 6.0;
	const float VORE_ANGLE = 76;
	const float PI = 3.14159;

	[[nodiscard]] inline RE::NiPoint3 RotateAngleAxis(const RE::NiPoint3& vec, const float angle, const RE::NiPoint3& axis)
	{
		float S = sin(angle);
		float C = cos(angle);

		const float XX = axis.x * axis.x;
		const float YY = axis.y * axis.y;
		const float ZZ = axis.z * axis.z;

		const float XY = axis.x * axis.y;
		const float YZ = axis.y * axis.z;
		const float ZX = axis.z * axis.x;

		const float XS = axis.x * S;
		const float YS = axis.y * S;
		const float ZS = axis.z * S;

		const float OMC = 1.f - C;

		return RE::NiPoint3(
			(OMC * XX + C) * vec.x + (OMC * XY - ZS) * vec.y + (OMC * ZX + YS) * vec.z,
			(OMC * XY + ZS) * vec.x + (OMC * YY + C) * vec.y + (OMC * YZ - XS) * vec.z,
			(OMC * ZX - YS) * vec.x + (OMC * YZ + XS) * vec.y + (OMC * ZZ + C) * vec.z
			);
	}

	void VoreInputEvent(const InputEventData& data) {
		static Timer voreTimer = Timer(0.25);
		auto pred = PlayerCharacter::GetSingleton();
		if (IsGtsBusy(pred)) {
			return;
		}

		if (voreTimer.ShouldRunFrame()) {
			auto& VoreManager = Vore::GetSingleton();
			std::size_t numberOfPrey = 1;
			if (Runtime::HasPerk(pred, "MassVorePerk")) {
				numberOfPrey = 1 + (get_visual_scale(pred)/3);
			}
			std::vector<Actor*> preys = VoreManager.GetVoreTargetsInFront(pred, numberOfPrey);
			for (auto prey: preys) {
				VoreManager.StartVore(pred, prey);
			}
		}
	}

	void AdjustGiantessSkill(Actor* Caster, float tinysize) { // Adjust Matter Of Size skill
		if (Caster->formID != 0x14) {
			return; //Bye
		}
		auto GtsSkillLevel = Runtime::GetGlobal("GtsSkillLevel");
		auto GtsSkillRatio = Runtime::GetGlobal("GtsSkillRatio");
		auto GtsSkillProgress = Runtime::GetGlobal("GtsSkillProgress");


		int random = (100 + (rand()% 25 + 1)) / 100;

		if (GtsSkillLevel->value >= 100.0) {
			GtsSkillLevel->value = 100.0;
			GtsSkillRatio->value = 0.0;
			return;
		}

		float skill_level = GtsSkillLevel->value;

		float ValueEffectiveness = std::clamp(1.0 - GtsSkillLevel->value/100, 0.10, 1.0);

		float absorbedSize = tinysize/50;

		float oldvaluecalc = 1.0 - GtsSkillRatio->value; //Attempt to keep progress on the next level
		float Total = (((0.30 * random) + absorbedSize) * ValueEffectiveness);
		GtsSkillRatio->value += Total * GetXpBonus();

		if (GtsSkillRatio->value >= 1.0) {
			float transfer = clamp(0.0, 1.0, Total - oldvaluecalc);
			GtsSkillRatio->value = transfer;
			GtsSkillLevel->value = skill_level + 1.0;
			GtsSkillProgress->value = GtsSkillLevel->value;
			AddPerkPoints(GtsSkillLevel->value);
		}
	}

	void BuffAttributes(Actor* giant, float tinyscale) {
		if (!giant) {
			return;
		}
		if (Runtime::HasPerk(giant, "SoulVorePerk")) { // Permamently increases random AV after eating someone
			float TotalMod = (0.75 * tinyscale);
			int Boost = rand() % 3;
			if (Boost == 0) {
				giant->AsActorValueOwner()->ModActorValue(ActorValue::kHealth, TotalMod);
			} else if (Boost == 1) {
				giant->AsActorValueOwner()->ModActorValue(ActorValue::kMagicka, TotalMod);
			} else if (Boost >= 2) {
				giant->AsActorValueOwner()->ModActorValue(ActorValue::kStamina, TotalMod);
			}
		}
	}

	void Vore_TransferItems(Actor* pred, Actor* prey) {
		TransferInventory(prey, pred, 1.0, false, true, DamageSource::Vored);
	}

	void VoreMessage_SwallowedAbsorbing(Actor* pred, Actor* prey) {
		if (!pred) {
			return;
		}
		int random = rand() % 4;
		if (!prey->IsDead() && !Runtime::HasPerk(pred, "SoulVorePerk") || random <= 1) {
			Cprint("{} was Swallowed and is now being slowly absorbed by {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 2) {
			Cprint("{} is now absorbing {}", pred->GetDisplayFullName(), prey->GetDisplayFullName());
		} else if (random >= 3) {
			Cprint("{} will soon be completely absorbed by {}", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		}
	}

	void VoreMessage_Absorbed(Actor* pred, std::string_view prey, bool WasDragon, bool WasGiant) {
		if (!pred) {
			return;
		}
		int random = rand() % 3;
		if (!AllowDevourment() && pred->formID == 0x14 && WasDragon) {
			log::info("{} was dragon", prey);
			CompleteDragonQuest();
		}
		if (WasGiant) {
			AdvanceQuestProgression(pred, 7, 1);
		} else {
			AdvanceQuestProgression(pred, 6, 1);
		}
		if (!Runtime::HasPerk(pred, "SoulVorePerk") || random == 0) {
			Cprint("{} was absorbed by {}", prey, pred->GetDisplayFullName());
		} else if (Runtime::HasPerk(pred, "SoulVorePerk") && random == 1) {
			Cprint("{} became one with {}", prey, pred->GetDisplayFullName());
		} else if (Runtime::HasPerk(pred, "SoulVorePerk") && random >= 2) {
			Cprint("{} was greedily devoured by {}", prey, pred->GetDisplayFullName());
		} else {
			Cprint("{} was absorbed by {}", prey, pred->GetDisplayFullName());
		}
	}
}

namespace Gts {
	VoreData::VoreData(Actor* giant) : giant(giant? giant->CreateRefHandle() : ActorHandle()) {

	}

	void VoreData::AddTiny(Actor* tiny) {
		this->tinies.try_emplace(tiny->formID, tiny->CreateRefHandle());
	}

	void VoreData::EnableMouthShrinkZone(bool enabled) {
		this->killZoneEnabled = enabled;
	}
	void VoreData::Swallow() {
		for (auto& [key, tinyref]: this->tinies) {
			auto tiny = tinyref.get().get();
			auto giant = this->giant.get().get();
			Vore::GetSingleton().AddVoreBuff(this->giant, tinyref);
			VoreMessage_SwallowedAbsorbing(giant, tiny);
			CallGainWeight(giant, 3.0 * get_visual_scale(tiny));
			if (giant->formID == 0x14) {
				CallVampire();

				bool Living = IsLiving(tiny);
				bool Dragon = IsDragon(tiny);
				float DefaultScale = Get_Other_Scale(tiny);

				SurvivalMode_AdjustHunger(this->giant.get().get(), get_visual_scale(tiny), DefaultScale, Dragon, Living, 0);
			}
		}
	}
	void VoreData::KillAll() {
		if (!AllowDevourment()) {
			for (auto& [key, tinyref]: this->tinies) {
				auto tiny = tinyref.get().get();
				auto giantref = this->giant;
				AddSMTDuration(giantref.get().get(), 6.0);
				if (tiny->formID != 0x14) {
					KillActor(giantref.get().get(), tiny);
					Disintegrate(tiny);
				} else if (tiny->formID == 0x14) {
					//DamageAV(tiny, ActorValue::kHealth, 900000.0);
					InflictSizeDamage(giantref.get().get(), tiny, 900000);
					KillActor(giantref.get().get(), tiny);
					TriggerScreenBlood(50);
					tiny->SetAlpha(0.0); // Player can't be disintegrated: simply nothing happens. So we Just make player Invisible instead.
				}

				std::string taskname = std::format("VoreAbsorb {}", tiny->formID);

				TaskManager::RunOnce(taskname, [=](auto& update) {
					if (!tinyref) {
						return;
					}
					if (!giantref) {
						return;
					}
					auto giant = giantref.get().get();
					auto smoll = tinyref.get().get();
					Vore_TransferItems(giant, smoll);
				});
			}
		}
		this->tinies.clear();
	}

	void VoreData::AllowToBeVored(bool allow) {
		for (auto& [key, tinyref]: this->tinies) {
			auto tiny = tinyref.get().get();
			auto transient = Transient::GetSingleton().GetData(tiny);
			if (transient) {
				transient->can_be_vored = allow;
			}
		}
	}

	bool VoreData::GetTimer() {
		return this->moantimer.ShouldRun();
	}

	void VoreData::GrabAll() {
		this->allGrabbed = true;
	}

	void VoreData::ReleaseAll() {
		this->allGrabbed = false;
	}

	std::vector<Actor*> VoreData::GetVories() {
		std::vector<Actor*> result;
		for (auto& [key, actorref]: this->tinies) {
			auto actor = actorref.get().get();
			result.push_back(actor);
		}
		return result;
	}

	void VoreData::Update() {
		auto profiler = Profilers::Profile("Vore: Update");
		auto giant = this->giant.get().get();
		float giantScale = get_visual_scale(giant);
		// Stick them to the AnimObjectA
		for (auto& [key, tinyref]: this->tinies) {
			auto tiny = tinyref.get().get();
			if (!tiny) {
				return;
			}
			if (!giant) {
				return;
			}

			if (this->allGrabbed && !giant->IsDead()) {
				AttachToObjectA(giant, tiny);
			}
		}
		// Hide Actor
		if (this->killZoneEnabled) {
			auto headNodeName = "NPC Head [Head]";
			auto headNode = find_node_any(giant, headNodeName);
			if (headNode) {
				NiPoint3 headCenter = headNode->world.translate;
				float headKillRadius = 10.0 * giantScale;
				for (auto& [key, tinyref]: this->tinies) {
					auto tiny = tinyref.get().get();
					// Get all nodes in range
					std::vector<NiAVObject*> nodes_inrange = {};
					std::vector<std::string> names_inrange = {};
					auto root = tiny->GetCurrent3D();
					VisitNodes(root, [&nodes_inrange, &headCenter, &headKillRadius, &names_inrange](NiAVObject& node) {
						float distance = (node.world.translate - headCenter).Length();
						if (distance < headKillRadius) {
							nodes_inrange.push_back(&node);
							names_inrange.push_back(node.name.c_str());
						}
						return true;
					});
					//log::info("Trying to shink {} nodes", nodes_inrange.size());

					// Check all children of the nodes
					std::vector<NiAVObject*> excludedChildren = {};
					excludedChildren.push_back(find_node(tiny, "NPC ROOT [ROOT]", false));
					excludedChildren.push_back(find_node(tiny, "NPC COM [COM]", false));
					excludedChildren.push_back(find_node(tiny, "NPC Pelvis [Pelv]", false));
					excludedChildren.push_back(find_node(tiny, "NPC Spine [Spn0]", false));
					excludedChildren.push_back(find_node(tiny, "Camera Control", false));
					excludedChildren.push_back(find_node(tiny, "NPC Translate", false));
					for (auto& node: nodes_inrange) {
						bool anyInvalid = false;
						VisitNodes(node, [&anyInvalid, &excludedChildren](NiAVObject& node_child) {
							for (auto excludedNode: excludedChildren) {
								if (excludedNode == &node_child) {
									anyInvalid = true;
									return false;
								}
							}
							return true;
						});
						if (!anyInvalid) {

							tiny->SetAlpha(0.0);//node->local.scale = 0.50;
							update_node(node);
						} else {
						}
					}
				}
			}
		}
	}

	VoreBuff::VoreBuff(Actor* giant, Actor* tiny) : factor(Spring(0.0, 1.0)), giant(giant ? giant->CreateRefHandle() : ActorHandle()), tiny(tiny ? tiny->CreateRefHandle() : ActorHandle()) {
		this->duration = 40.0;
		float mealEffiency = 0.2; // Normal pred has 20% efficent stomach
		float perkbonus = 1.0;
		if (Runtime::HasPerkTeam(giant, "Gluttony")) {
			this->duration = 60.0;
			mealEffiency = 0.3;
		}
		if (Runtime::HasPerkTeam(giant, "AdditionalGrowth")) {
			perkbonus = 1.25;
		}
		if (Runtime::HasPerkTeam(giant, "AdditionalGrowth_p2")) {
			perkbonus += 0.30;
		}
		if (IsDragon(tiny)) {
			mealEffiency *= 6.0;
		} if (IsGiant(tiny)) {
			mealEffiency *= 2.6;
		}
		this->appliedFactor = 0.0;
		this->state = VoreBuffState::Starting;

		if (tiny) {
			float tiny_scale = get_visual_scale(tiny);
			// Amount of health we apply depends on their vitality
			// and their size
			if (Runtime::HasPerkTeam(giant, "Gluttony")) {
				this->restorePower = GetMaxAV(tiny, ActorValue::kHealth) * 8 * mealEffiency;
			} else {
				this->restorePower = 0.0;
			}
			this->WasGiant = IsGiant(tiny);
			this->WasDragon = IsDragon(tiny);
			this->WasLiving = IsLiving(tiny);
			this->sizePower = tiny_scale * mealEffiency * perkbonus;
			this->tinySize = tiny_scale;
			this->naturalsize = Get_Other_Scale(tiny);
			this->tiny_name = tiny->GetDisplayFullName();
		}
	}
	void VoreBuff::Update() {
		auto giant = this->giant.get().get();
		if (!this->giant) {
			this->state = VoreBuffState::Done;
			return;
		}
		switch (this->state) {
			case VoreBuffState::Starting: {
				this->factor.value = 0.0;
				this->factor.velocity = 0.0;
				this->factor.target = 1.0;
				this->factor.halflife = this->duration * 0.5;
				this->state = VoreBuffState::Running;
				break;
			}
			case VoreBuffState::Running: {
				float healthToApply = this->restorePower/4000;
				float sizeToApply = this->sizePower/5500;

				DamageAV(giant, ActorValue::kHealth, -healthToApply);
				DamageAV(giant, ActorValue::kStamina, -healthToApply);

				mod_target_scale(giant, sizeToApply);
				AddStolenAttributes(giant, sizeToApply);
				if (this->factor.value >= 0.99) {
					this->state = VoreBuffState::Finishing;
				}
				break;
			}
			case VoreBuffState::Finishing: {
				if (!AllowDevourment()) {
					if (this->giant) {
						AdjustGiantessSkill(giant, this->tinySize);
						VoreMessage_Absorbed(giant, this->tiny_name, this->WasDragon, this->WasGiant);
						BuffAttributes(giant, this->tinySize);
						mod_target_scale(giant, this->sizePower * 0.5);
						AdjustSizeReserve(giant, this->sizePower);
						if (giant->formID == 0x14) {
							AdjustSizeLimit(0.0260, giant);
							AdjustMassLimit(0.0106, giant);
							SurvivalMode_AdjustHunger(giant, this->tinySize, this->naturalsize,this->WasDragon, this->WasLiving, 1);
						}
						Rumble::Once("GrowthRumble", giant, 2.45, 0.30);
						Rumble::Once("VoreShake", giant, this->sizePower * 4, 0.05);
						if (Vore::GetSingleton().GetVoreData(giant).GetTimer() == true) {
							Runtime::PlaySoundAtNode("MoanSound", giant, 1.0, 1.0, "NPC Head [Head]");
						}
					}
				}

				this->state = VoreBuffState::Done;
				break;
			}
			case VoreBuffState::Done: {
			}
		}
	}

	bool VoreBuff::Done() {
		return this->state == VoreBuffState::Done;
	}

	Vore& Vore::GetSingleton() noexcept {
		static Vore instance;
		return instance;
	}

	std::string Vore::DebugName() {
		return "Vore";
	}

	void Vore::DataReady() {
		InputManager::RegisterInputEvent("Vore", VoreInputEvent);
	}

	void Vore::Update() {
		auto player = PlayerCharacter::GetSingleton();
		auto& persist = Persistent::GetSingleton();

		static Timer timer = Timer(2.50); // Random Vore once per 2.5 sec
		if (timer.ShouldRunFrame()) { //Try to not call it too often
			std::vector<Actor*> AbleToVore = {};
			for (auto actor: find_actors()) {
				if (actor->formID != 0x14 && IsTeammate(actor) && (actor->IsInCombat() || !persist.vore_combatonly)) {
					AbleToVore.push_back(actor);
				}
			}
			if (!AbleToVore.empty()) {
				int idx = rand() % AbleToVore.size();
				Actor* voreActor = AbleToVore[idx];
				if (voreActor) {
					RandomVoreAttempt(voreActor);
				}
			}
		}

		for (auto& [key, voreData]: this->data) {
			voreData.Update();
		}
		for (auto& [key, voreBuff]: this->buffs) {
			voreBuff.Update();
			if (voreBuff.Done()) {
				this->buffs.erase(key);
			}
		}
	}

	void Vore::RandomVoreAttempt(Actor* pred) {
		if (!Persistent::GetSingleton().Vore_Ai) {
			log::info("Vore Ai is False");
			return;
		}
		log::info("Trying Random Vore for {}", pred->GetDisplayFullName());
		Actor* player = PlayerCharacter::GetSingleton();
		auto& VoreManager = Vore::GetSingleton();
		if (IsGtsBusy(pred)) {
			return; // No Vore attempts if in GTS_Busy
		}

		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerk(player, "MassVorePerk")) {
			numberOfPrey = 1 + (get_visual_scale(pred)/3);
		}
		for (auto actor: find_actors()) {
			if (!actor->Is3DLoaded() || actor->IsDead()) {
				return;
			}
			float Gigantism = 1.0 / (1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(pred)/100);
			int Requirement = (8 * Gigantism) * SizeManager::GetSingleton().BalancedMode();

			int random = rand() % Requirement;
			int decide_chance = 2;
			if (random <= decide_chance) {
				std::vector<Actor*> preys = VoreManager.GetVoreTargetsInFront(pred, numberOfPrey);
				for (auto prey: preys) {
					VoreManager.StartVore(pred, prey);
				}
			}
		}
	}

	Actor* Vore::GeVoreTargetCrossHair(Actor* pred) {
		auto preys = this->GeVoreTargetsCrossHair(pred, 1);
		if (preys.size() > 0) {
			return preys[0];
		} else {
			return nullptr;
		}
	}

	std::vector<Actor*> Vore::GeVoreTargetsCrossHair(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for player
		if (!pred) {
			return {};
		}
		auto playerCamera = PlayerCamera::GetSingleton();
		if (!playerCamera) {
			return {};
		}
		auto crosshairPick = RE::CrosshairPickData::GetSingleton();
		if (!crosshairPick) {
			return {};
		}
		auto cameraNode = playerCamera->cameraRoot.get();
		if (!cameraNode) {
			return {};
		}
		NiPoint3 start = cameraNode->world.translate;
		NiPoint3 end = crosshairPick->collisionPoint;

		auto preys = find_actors();
		auto predPos = pred->GetPosition();

		// Sort prey by distance
		sort(preys.begin(), preys.end(),
		     [predPos](const Actor* preyA, const Actor* preyB) -> bool
		{
			float distanceToA = (preyA->GetPosition() - predPos).Length();
			float distanceToB = (preyB->GetPosition() - predPos).Length();
			return distanceToA < distanceToB;
		});

		// Filter out invalid targets
		preys.erase(std::remove_if(preys.begin(), preys.end(),[pred, this](auto prey)
		{
			return !this->CanVore(pred, prey);
		}), preys.end());;

		// Filter out actors not in front
		NiPoint3 predDir = end - start;
		predDir = predDir / predDir.Length();
		preys.erase(std::remove_if(preys.begin(), preys.end(),[predPos, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - predPos;
			if (preyDir.Length() <= 1e-4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= 0; // 180 degress
		}), preys.end());

		NiPoint3 coneStart = start;
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= 1e-4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= cos(VORE_ANGLE*PI/180.0);
		}), preys.end());

		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}

		return preys;
	}

	Actor* Vore::GetVoreTargetInFront(Actor* pred) {
		auto preys = this->GetVoreTargetsInFront(pred, 1);
		if (preys.size() > 0) {
			return preys[0];
		} else {
			return nullptr;
		}
	}

	std::vector<Actor*> Vore::GetVoreTargetsInFront(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for actor
		if (!pred) {
			return {};
		}
		auto charController = pred->GetCharController();
		if (!charController) {
			return {};
		}

		NiPoint3 predPos = pred->GetPosition();

		auto preys = find_actors();

		// Sort prey by distance
		sort(preys.begin(), preys.end(),
		     [predPos](const Actor* preyA, const Actor* preyB) -> bool
		{
			float distanceToA = (preyA->GetPosition() - predPos).Length();
			float distanceToB = (preyB->GetPosition() - predPos).Length();
			return distanceToA < distanceToB;
		});

		// Filter out invalid targets
		preys.erase(std::remove_if(preys.begin(), preys.end(),[pred, this](auto prey)
		{
			return !this->CanVore(pred, prey);
		}), preys.end());;

		// Filter out actors not in front
		auto actorAngle = pred->data.angle.z;
		RE::NiPoint3 forwardVector{ 0.f, 1.f, 0.f };
		RE::NiPoint3 actorForward = RotateAngleAxis(forwardVector, -actorAngle, { 0.f, 0.f, 1.f });

		NiPoint3 predDir = actorForward;
		predDir = predDir / predDir.Length();
		preys.erase(std::remove_if(preys.begin(), preys.end(),[predPos, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - predPos;
			if (preyDir.Length() <= 1e-4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= 0; // 180 degress
		}), preys.end());

		// Filter out actors not in a truncated cone
		// \      x   /
		//  \  x     /
		//   \______/  <- Truncated cone
		//   | pred |  <- Based on width of pred
		//   |______|
		float predWidth = 70 * get_visual_scale(pred);
		float shiftAmount = fabs((predWidth / 2.0) / tan(VORE_ANGLE/2.0));

		NiPoint3 coneStart = predPos - predDir * shiftAmount;
		preys.erase(std::remove_if(preys.begin(), preys.end(),[coneStart, predWidth, predDir](auto prey)
		{
			NiPoint3 preyDir = prey->GetPosition() - coneStart;
			if (preyDir.Length() <= predWidth*0.4) {
				return false;
			}
			preyDir = preyDir / preyDir.Length();
			float cosTheta = predDir.Dot(preyDir);
			return cosTheta <= cos(VORE_ANGLE*PI/180.0);
		}), preys.end());

		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}

		return preys;
	}

	Actor* Vore::GetVoreTargetAround(Actor* pred) {
		auto preys = this->GetVoreTargetsAround(pred, 1);
		if (preys.size() > 0) {
			return preys[0];
		} else {
			return nullptr;
		}
	}

	std::vector<Actor*> Vore::GetVoreTargetsAround(Actor* pred, std::size_t numberOfPrey) {
		// Get vore target for actor
		// around them
		if (!pred) {
			return {};
		}
		NiPoint3 predPos = pred->GetPosition();

		auto preys = find_actors();

		// Sort prey by distance
		sort(preys.begin(), preys.end(),
		     [predPos](const Actor* preyA, const Actor* preyB) -> bool
		{
			float distanceToA = (preyA->GetPosition() - predPos).Length();
			float distanceToB = (preyB->GetPosition() - predPos).Length();
			return distanceToA < distanceToB;
		});

		// Filter out invalid targets
		preys.erase(std::remove_if(preys.begin(), preys.end(),[pred, this](auto prey)
		{
			return !this->CanVore(pred, prey);
		}), preys.end());

		// Reduce vector size
		if (preys.size() > numberOfPrey) {
			preys.resize(numberOfPrey);
		}

		return preys;
	}


	bool Vore::CanVore(Actor* pred, Actor* prey) {
		if (pred == prey) {
			return false;
		}
		if (!CanPerformAnimation(pred, 3)) {
			return false;
		}
		auto transient = Transient::GetSingleton().GetData(prey);
		if (prey->IsDead()) {
			return false;
		}

		if (transient) {
			if (transient->can_be_vored == false) {
				Notify("{} is already being eaten by someone else", prey->GetDisplayFullName());
				Cprint("{} is already being eaten by someone else", prey->GetDisplayFullName());
				return false;
			}
		}
		float MINIMUM_VORE_SCALE = MINIMUM_VORE_SCALE_RATIO;
		float MINIMUM_DISTANCE = MINIMUM_VORE_DISTANCE;
		if (Runtime::HasPerkTeam(pred, "VorePerk")) {
			MINIMUM_VORE_SCALE = 6.0;
		}

		if (HasSMT(pred)) {
			MINIMUM_VORE_SCALE = 0.7;
			MINIMUM_DISTANCE *= 1.75;
		}

		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);
		if (IsDragon(prey)) {
			prey_scale *= 3.0;
		} if (IsGiant(prey)) {
			prey_scale *= 2.2;
		}if (IsMammoth(prey)) {
			prey_scale *= 4.0;
		}

		float sizedifference = pred_scale/prey_scale;

		float balancemode = SizeManager::GetSingleton().BalancedMode();
		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();

		if (IsInsect(prey, true) || IsBlacklisted(prey) || IsUndead(prey)) {
			std::string_view message = std::format("{} has no desire to eat {}", pred->GetDisplayFullName(), prey->GetDisplayFullName());
			TiredSound(pred, message);
			return false;
		}

		if (balancemode == 2.0) { // This is checked only if Balance Mode is enabled. Size requirement is bigger with it.
			MINIMUM_VORE_SCALE *= 1.15;
		}
		if (prey_distance <= (MINIMUM_DISTANCE * pred_scale) && pred_scale/prey_scale < MINIMUM_VORE_SCALE) {
			if (pred->formID == 0x14) {
				Notify("{} is too big to be eaten.", prey->GetDisplayFullName());
			}
			return false;
		}
		if (prey_distance <= (MINIMUM_DISTANCE * pred_scale) && pred_scale/prey_scale > MINIMUM_VORE_SCALE) {
			if ((prey->formID != 0x14 && IsEssential(prey) && !AllowActionsWithFollowers(pred, prey))) {
				Notify("{} is important and shouldn't be eaten.", prey->GetDisplayFullName());
				return false;
			} else {
				return true;
			}
		} else {
			return false;
		}
	}

	void Vore::Reset() {
		this->data.clear();
		this->buffs.clear();
	}

	void Vore::ResetActor(Actor* actor) {
		this->data.erase(actor->formID);
		this->buffs.erase(actor->formID);
	}

	void Vore::StartVore(Actor* pred, Actor* prey) {
		if (pred->formID != 0x14) {
			if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore) {
				log::info("{} vore immunity is on", prey->GetDisplayFullName());
				return;
			}
			if (!AllowActionsWithFollowers(pred, prey)) {
				log::info("Actions with {} and {} are disallowed", pred->GetDisplayFullName(), prey->GetDisplayFullName());
				return;
			}
		}

		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);

		float sizedifference = pred_scale/prey_scale;

		float wastestamina = 45; // Drain stamina, should be 300 once tests are over
		float staminacheck = pred->AsActorValueOwner()->GetActorValue(ActorValue::kStamina);

		if (pred->formID != 0x14) {
			wastestamina = 30; // Less tamina drain for non Player
		}

		if (!CanVore(pred, prey)) {
			log::info("{} can't do Vore", pred->GetDisplayFullName());
			return;
		}
		if (!Runtime::HasPerkTeam(pred, "VorePerk")) { // Damage stamina if we don't have perk
			if (staminacheck < wastestamina) {
				Notify("{} is too tired for vore.", pred->GetDisplayFullName());
				DamageAV(prey, ActorValue::kHealth, 3 * sizedifference);
				if (pred->formID == 0x14) {
					Runtime::PlaySound("VoreSound_Fail", pred, 1.8, 0.0);
				}
				StaggerActor(prey);
				return;
			}
			DamageAV(pred, ActorValue::kStamina, wastestamina);
		}

		ShrinkUntil(pred, prey, 8.0); // Shrink if we have SMT to allow 'same-size' vore

		if (pred->formID == 0x14) {
			Runtime::PlaySound("VoreSound_Success", pred, 0.6, 0.0);
		}
		auto& voreData = this->GetVoreData(pred);
		voreData.AddTiny(prey);
		log::info("{} is starting Vore", pred->GetDisplayFullName());

		AnimationManager::GetSingleton().StartAnim("StartVore", pred);
	}

	// Gets the current vore data of a giant
	VoreData& Vore::GetVoreData(Actor* giant) {
		// Create it now if not there yet
		this->data.try_emplace(giant->formID, giant);

		return this->data.at(giant->formID);
	}

	void Vore::AddVoreBuff(ActorHandle giantref, ActorHandle tinyref) {
		auto giant = giantref.get().get();
		auto tiny = tinyref.get().get();
		this->buffs.try_emplace(tiny->formID, giant, tiny);
	}
}
