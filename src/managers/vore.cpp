#include "managers/animation/AnimationManager.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "magic/effects/common.hpp"
#include "utils/actorUtils.hpp"
#include "managers/Rumble.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "managers/vore.hpp"
#include "data/runtime.hpp"
#include "scale/scale.hpp"
#include "timer.hpp"
#include "node.hpp"
#include <cmath>

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

	void AdjustGiantessSkill(Actor* Caster, Actor* Target) { // Adjust Matter Of Size skill
		if (Caster->formID !=0x14) {
			return; //Bye
		}
		auto GtsSkillLevel = Runtime::GetGlobal("GtsSkillLevel");
		if (!GtsSkillLevel) {
			return;
		}
		auto GtsSkillRatio = Runtime::GetGlobal("GtsSkillRatio");
		if (!GtsSkillRatio) {
			return;
		}
		auto GtsSkillProgress = Runtime::GetGlobal("GtsSkillProgress");
		if (!GtsSkillProgress) {
			return;
		}

		int random = (100 + (rand()% 85 + 1)) / 100;

		if (GtsSkillLevel->value >= 100) {
			GtsSkillLevel->value = 100.0;
			GtsSkillRatio->value = 0.0;
			return;
		}

		float ValueEffectiveness = std::clamp(1.0 - GtsSkillLevel->value/100, 0.05, 1.0);

		float absorbedSize = (get_visual_scale(Target));
		float oldvaluecalc = 1.0 - GtsSkillRatio->value; //Attempt to keep progress on the next level
		float Total = (((0.50 * random) + absorbedSize/50) * ValueEffectiveness);
		GtsSkillRatio->value += Total;

		if (GtsSkillRatio->value >= 1.0) {
			float transfer = clamp(0.0, 1.0, Total - oldvaluecalc);
			GtsSkillLevel->value += 1.0;
			GtsSkillProgress->value = GtsSkillLevel->value;
			GtsSkillRatio->value = 0.0 + transfer;
		}
	}

	void BuffAttributes(Actor* giant, Actor* tiny) {
		if (Runtime::HasPerk(giant, "SoulVorePerk")) { // Permamently increases random AV after eating someone
			float TotalMod = (0.75 * get_visual_scale(tiny));
			int Boost = rand() % 2;
			if (Boost == 0) {
				giant->AsActorValueOwner()->ModActorValue(ActorValue::kHealth, TotalMod);
			} else if (Boost == 1) {
				giant->AsActorValueOwner()->ModActorValue(ActorValue::kMagicka, TotalMod);
			} else if (Boost == 2) {
				giant->AsActorValueOwner()->ModActorValue(ActorValue::kStamina, TotalMod);
			}
			//log::info("Buffing Attributes {}, Target: {}, Caster: {}", Boost, Target->GetDisplayFullName(), Caster->GetDisplayFullName());
		}
	}


	void VoreMessage_SwallowedAbsorbing(Actor* pred, Actor* prey) {
		int random = rand() % 4;
		if (!prey->IsDead() && !Runtime::HasPerk(pred, "SoulVorePerk") || random <= 1) {
			ConsoleLog::GetSingleton()->Print("%s was Swallowed and is now being slowly absorbed by %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (random == 2) {
			ConsoleLog::GetSingleton()->Print("%s is now absorbing %s", pred->GetDisplayFullName(), prey->GetDisplayFullName());
		} else if (random >= 3) {
			ConsoleLog::GetSingleton()->Print("%s will soon be completely absorbed by %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		}
	}

	void VoreMessage_Absorbed(Actor* pred, Actor* prey) {
		int random = rand() % 2;
		if (!AllowDevourment() && pred->formID == 0x14 && IsDragon(prey)) {
			CompleteDragonQuest();
		}
		if (!prey->IsDead() && !Runtime::HasPerk(pred, "SoulVorePerk") || random == 0) {
			ConsoleLog::GetSingleton()->Print("%s was completely absorbed by %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (!prey->IsDead() && Runtime::HasPerk(pred, "SoulVorePerk") && random == 1) {
			ConsoleLog::GetSingleton()->Print("%s became one with %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (!prey->IsDead() && Runtime::HasPerk(pred, "SoulVorePerk") && random == 2) {
			ConsoleLog::GetSingleton()->Print("%s both body and soul were greedily devoured by %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		} else if (prey->IsDead()) {
			ConsoleLog::GetSingleton()->Print("%s was absorbed by %s", prey->GetDisplayFullName(), pred->GetDisplayFullName());
		}
	}

  hkaRagdollInstance* GetRagdoll(Actor* actor) {
		BSAnimationGraphManagerPtr animGraphManager;
		if (actor->GetAnimationGraphManager(animGraphManager)) {
			for (auto& graph : animGraphManager->graphs) {
				if (graph) {
					auto& character = graph->characterInstance;
					auto ragdollDriver = character.ragdollDriver.get();
					if (ragdollDriver) {
						auto ragdoll = ragdollDriver->ragdoll;
						if (ragdoll) {
							return ragdoll;
						}
					}
				}
			}
		}
		return nullptr;
	}
}

namespace Gts {
	VoreData::VoreData(Actor* giant) : giant(giant) {

	}

	void VoreData::AddTiny(Actor* tiny) {
		this->tinies.try_emplace(tiny, tiny);
	}

	void VoreData::EnableMouthShrinkZone(bool enabled) {
		this->killZoneEnabled = enabled;
	}
	void VoreData::Swallow() {
		for (auto& [key, tiny]: this->tinies) {
			Vore::GetSingleton().AddVoreBuff(giant, tiny);
			VoreMessage_SwallowedAbsorbing(giant, tiny);
			CallGainWeight(giant, 3.0 * get_visual_scale(tiny));
			if (giant->formID == 0x14) {
				CallVampire();
			}
		}
	}
	void VoreData::KillAll() {
		if (!AllowDevourment()) {
		for (auto& [key, tiny]: this->tinies) {
			if (tiny->formID != 0x14) {
				Disintegrate(tiny);
				///this->tinies.erase(tiny);
			} else if (tiny->formID == 0x14) {
				tiny->KillImmediate();
				TriggerScreenBlood(50);
				tiny->SetAlpha(0.0); // Player can't be disintegrated: simply nothing happens. So we Just make player Invisible instead.
				}
			}
		}
		this->tinies.clear();
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
		for (auto& [key, actor]: this->tinies) {
			result.push_back(actor);
		}
		return result;
	}

	void VoreData::Update() {
		auto giant = this->giant;
    	float giantScale = get_visual_scale(giant);

		// Stick them to the AnimObjectA
		for (auto& [key, tiny]: this->tinies) {
			if (!tiny) {
				return;
			}

			auto bone = find_node(giant, "AnimObjectA");
			if (!bone) {
				return;
			}

			if (this->allGrabbed) {
				NiPoint3 giantLocation = giant->GetPosition();
				NiPoint3 tinyLocation = tiny->GetPosition();
				NiPoint3 targetLocation = bone->world.translate;
        		NiPoint3 deltaLocation = targetLocation - tinyLocation;
        		float deltaLength = deltaLocation.Length();

				tiny->SetPosition(targetLocation, true);
				tiny->SetPosition(targetLocation, false);
				//log::info("Setting Position");
				Actor* tiny_is_actor = skyrim_cast<Actor*>(tiny);
				if (tiny_is_actor) {
					auto charcont = tiny_is_actor->GetCharController();
					if (charcont) {
						charcont->SetLinearVelocityImpl((0.0, 0.0, -5.0, 0.0)); // Needed so Actors won't fall down.
					}


          /*if (deltaLength >= 70.0) {
            // WARP if > 1m
            auto ragDoll = GetRagdoll(tiny_is_actor);
            hkVector4 delta = hkVector4(deltaLocation.x/70.0, deltaLocation.y/70.0, deltaLocation.z/70, 1.0);
            for (auto rb: ragDoll->rigidBodies) {
              if (rb) {
                auto ms = rb->GetMotionState();
                if (ms) {
                  hkVector4 currentPos = ms->transform.translation;
                  hkVector4 newPos = currentPos + delta;
                  rb->motion.SetPosition(newPos);
                  rb->motion.SetLinearVelocity(hkVector4(0.0, 0.0, 0.0, 0.0));
                }

              }
            }
          } else {
            // Just move the hand if <1m
            std::string_view handNodeName = "NPC HAND L [L Hand]";
            auto handBone = find_node(tiny, handNodeName);
            if (handBone) {
              auto collisionHand = handBone->GetCollisionObject();
              if (collisionHand) {
                auto handRbBhk = collisionHand->GetRigidBody();
                if (handRbBhk) {
                  auto handRb = static_cast<hkpRigidBody*>(handRbBhk->referencedObject.get());
                  if (handRb) {
                    auto ms = handRb->GetMotionState();
                    if (ms) {
                      hkVector4 targetLocationHavok = hkVector4(targetLocation.x/70.0, targetLocation.y/70.0, targetLocation.z/70, 1.0);
                      handRb->motion.SetPosition(targetLocationHavok);
                      handRb->motion.SetLinearVelocity(hkVector4(0.0, 0.0, 0.0, 0.0));
                    }
                  }
                }
              }
            }
          }*/
				}
			}
		}

		// Shrink nodes
		if (this->killZoneEnabled) {
			auto headNodeName = "NPC Head [Head]";
			auto headNode = find_node_any(giant, headNodeName);
			if (headNode) {
				NiPoint3 headCenter = headNode->world.translate;
				float headKillRadius = 10.0 * giantScale;
				for (auto& [key, tiny]: this->tinies) {
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
					//
					// This ensure that we are not going to shink
					// the wrong nodes like the NPC Root which would mess
					// with our scales
					//
					// Or the camera nodes that mess with the rendering
					//
					// Or the root of the upper and lower bodies
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
              //log::info("  - Shrinking Node: {}", node->name.c_str());
							tiny->SetAlpha(0.0);//node->local.scale = 0.50;
              update_node(node);
						} else {
              //log::info("  - NOT Shrinking Node: {}", node->name.c_str());
            }
					}
				}
			}
		}
	}

	VoreBuff::VoreBuff(Actor* giant, Actor* tiny) : factor(Spring(0.0, 1.0)) {
		this->giant = giant;
		this->tiny = tiny;
		this->duration = 40.0;
		float mealEffiency = 0.2; // Normal pred has 20% efficent stomach
		if (Runtime::HasPerkTeam(giant, "AdditionalAbsorption")) {
			this->duration = 60.0;
			mealEffiency = 0.3;
		}
		if (IsDragon(tiny)) {
			mealEffiency *= 6.0;
		}
		this->appliedFactor = 0.0;
    	this->state = VoreBuffState::Starting;

		if (tiny) {
			float tiny_scale = get_visual_scale(tiny);
			// Amount of health we apply depends on their vitality
			// and their size
			if (Runtime::HasPerkTeam(giant, "VorePerkRegeneration")) {
				this->restorePower = GetMaxAV(tiny, ActorValue::kHealth) * 8 * mealEffiency;
			} else {
        		this->restorePower = 0.0;
      		}
			this->sizePower = tiny_scale * mealEffiency;
		}
	}
	void VoreBuff::Update() {
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
    			float healthToApply = this->restorePower/3800;
    			float sizeToApply = this->sizePower/4000;

    			DamageAV(this->giant, ActorValue::kHealth, -healthToApply);
    			DamageAV(this->giant, ActorValue::kStamina, -healthToApply);

    			mod_target_scale(this->giant, sizeToApply);
				if (this->factor.value >= 0.99) {
          			this->state = VoreBuffState::Finishing;
				}
        	break;
		}
		case VoreBuffState::Finishing: {
			if (!AllowDevourment()) {
				AdjustGiantessSkill(this->giant, this->tiny);
				VoreMessage_Absorbed(this->giant, this->tiny);
				BuffAttributes(this->giant, this->tiny);
				mod_target_scale(this->giant, this->sizePower * 1.0);
				AdjustSizeReserve(this->giant, this->sizePower);
				Rumble::Once("GrowthRumble", this->giant, 2.45, 0.30);
				Rumble::Once("VoreShake", this->giant, this->sizePower * 4, 0.05);

				if (VoreData(this->giant).GetTimer() {
					Runtime::PlaySoundAtNode("MoanSound", this->giant, 1.0, 1.0, "NPC Head [Head]");
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
		if (!Runtime::HasPerk(player, "VorePerk")) {
			return;
		}
		static Timer timer = Timer(3.00); // Random Vore once per 3 sec
		if (timer.ShouldRunFrame()) { //Try to not call it too often
			for (auto actor: find_actors()) {
				if ((Runtime::InFaction(actor, "FollowerFaction") || actor->IsPlayerTeammate()) && (actor->IsInCombat() || !persist.vore_combatonly)) {
					RandomVoreAttempt(actor);
					log::info("Actor {} trying to do vore", actor->GetDisplayFullName());
				}
			}
		}

		for (auto& [key, voreData]: this->data) {
			voreData.Update();
		}
		for (auto& [key, voreBuff]: this->buffs) {
			voreBuff.Update();
			if (voreBuff.Done()) {
        		log::info("Erasing buff");
				this->buffs.erase(key);
			}
		}
	}

	void Vore::RandomVoreAttempt(Actor* caster) {
		Actor* player = PlayerCharacter::GetSingleton();
		auto transient = Transient::GetSingleton().GetData(caster);
		auto& VoreManager = Vore::GetSingleton();
		if (!transient) {
			return;
		}
		if (!transient->can_do_vore) {
			return;
		}

		std::size_t numberOfPrey = 1;
		if (Runtime::HasPerk(player, "MassVorePerk")) {
			numberOfPrey = 1 + (get_visual_scale(caster)/3);
		}
		for (auto actor: find_actors()) {
			if (!actor->Is3DLoaded() || actor->IsDead()) {
				return;
			}
			float Gigantism = 1.0 / (1.0 + SizeManager::GetSingleton().GetEnchantmentBonus(caster)/100);
			int Requirement = (10 * Gigantism) * SizeManager::GetSingleton().BalancedMode();

			int random = rand() % Requirement;
			int decide_chance = 2;
			if (random <= decide_chance) {
				Actor* pred = caster;
				log::info("random Vore for {} is true", caster->GetDisplayFullName());
				log::info("{} is looking for prey", caster->GetDisplayFullName());
				std::vector<Actor*> preys = VoreManager.GetVoreTargetsInFront(pred, numberOfPrey);
				for (auto prey: preys) {
					if (prey->formID == 0x14 && !Persistent::GetSingleton().vore_allowplayervore) {
						log::info("Player Vore is False");
						return;
					}
					log::info("Actor {} found Prey: {}, starting Vore", pred->GetDisplayFullName(), prey->GetDisplayFullName());
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

		if (!Runtime::HasPerkTeam(pred, "VorePerk")) {
			return false;
		}

		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);
		if (IsDragon(prey)) {
			prey_scale *= 2.6;
		}

		float sizedifference = pred_scale/prey_scale;

		float MINIMUM_VORE_SCALE = MINIMUM_VORE_SCALE_RATIO;

		float balancemode = SizeManager::GetSingleton().BalancedMode();
		float prey_distance = (pred->GetPosition() - prey->GetPosition()).Length();

		if (balancemode == 2.0) { // This is checked only if Balance Mode is enabled. Enables HP requirement on Vore.
			if (prey_distance <= (MINIMUM_VORE_DISTANCE * pred_scale) && pred_scale/prey_scale < MINIMUM_VORE_SCALE) {
				float getmaxhp = GetMaxAV(prey, ActorValue::kHealth);
				float gethp = GetAV(prey, ActorValue::kHealth);
				float healthrequirement = getmaxhp/pred_scale;
				if (gethp > healthrequirement) {
					DamageAV(prey, ActorValue::kHealth, 6 * sizedifference);
					DamageAV(pred, ActorValue::kStamina, 26/sizedifference);
					if (pred->formID == 0x14) {
						Notify("{} is too healthy to be eaten", prey->GetDisplayFullName());
					}
					return false;
				}
			}
		}
		if (pred->formID == 0x14 && prey_distance <= (MINIMUM_VORE_DISTANCE * pred_scale) && pred_scale/prey_scale < MINIMUM_VORE_SCALE) {
			Notify("{} is too big to be eaten.", prey->GetDisplayFullName());
			return false;
		}
		if (prey_distance <= (MINIMUM_VORE_DISTANCE * pred_scale) && pred_scale/prey_scale > MINIMUM_VORE_SCALE) {
				if ((prey->IsEssential() && Runtime::GetBool("ProtectEssentials")) || Runtime::HasSpell(prey, "StartVore")) {
					return false;
				} else {
					return true;
				}
			}
		else {
			return false;
		}
	}

	void Vore::Reset() {
		this->data.clear();
		this->buffs.clear();
	}

	void Vore::ResetActor(Actor* actor) {
		this->data.erase(actor);
		this->buffs.erase(actor);
	}

	void Vore::StartVore(Actor* pred, Actor* prey) {
		float pred_scale = get_visual_scale(pred);
		float prey_scale = get_visual_scale(prey);

		float sizedifference = pred_scale/prey_scale;

		float wastestamina = 140/sizedifference; // Drain stamina, should be 300 once tests are over
		float staminacheck = pred->AsActorValueOwner()->GetActorValue(ActorValue::kStamina);

		if (pred->formID != 0x14) {
			wastestamina = 40/sizedifference; // Less tamina drain for non Player
		}



		if (!CanVore(pred, prey)) {
			return;
		}
		if (prey->IsEssential() && Runtime::GetBool("ProtectEssentials")) {
			Notify("{} is important, i shouldn't eat {}.", prey->GetDisplayFullName(), prey->GetDisplayFullName());
		}
		if (staminacheck < wastestamina) {
			Notify("{} is too tired for vore.", pred->GetDisplayFullName());
			DamageAV(prey, ActorValue::kHealth, 3 * sizedifference);
			Runtime::PlaySound("VoreSound_Fail", pred, 1.8, 0.0);
			Runtime::CastSpell(pred, prey, "gtsStaggerSpell");
			return;
		}

		DamageAV(pred, ActorValue::kStamina, wastestamina);
		Runtime::PlaySound("VoreSound_Success", pred, 0.6, 0.0);

		if (pred->formID == 0x14) {
			AdjustSizeLimit(0.0260, pred);
		}
		//Runtime::CastSpell(pred, prey, "StartVore");
		auto& voreData = this->GetVoreData(pred);
		voreData.AddTiny(prey);

		AnimationManager::GetSingleton().StartAnim("StartVore", pred);
	}

	// Gets the current vore data of a giant
	VoreData& Vore::GetVoreData(Actor* giant) {
		// Create it now if not there yet
		this->data.try_emplace(giant, giant);

		return this->data.at(giant);
	}

	void Vore::AddVoreBuff(Actor* giant, Actor* tiny) {
		this->buffs.try_emplace(tiny, giant, tiny);
	}
}
