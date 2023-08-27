#pragma once

#include "profiler.hpp"

using namespace std;
using namespace RE;
using namespace SKSE;

namespace Gts {
	enum FootEvent {
		Left,
		Right,
		Front,
		Back,
		JumpLand,
		Unknown,
		Butt,
		Breasts,
	};

	enum DamageSource {
		HandCrawl,
		HandSwipe,
		HandSlam,
		Knee,
		Breast,
		Hugs,
		Booty,
		Vored,
		Spell,
		Melted,
		Explode,
		Crushed,
		Overkill,
		Shrinked,
		Collision,
		FootGrinded,
		HandCrushed,
		ThighSandwiched,
		ThighCrushed,
		BlockDamage,
		HitSteal,
	};

	enum CrawlEvent {
		LeftKnee,
		RightKnee,
		LeftHand,
		RightHand,
	};

	struct Impact {
		Actor* actor;
		FootEvent kind;
		float scale;
		std::vector<NiAVObject*> nodes;
	};
	struct UnderFoot {
		Actor* giant;
		Actor* tiny;
		float force;
		/// Giant foot that is doing the squishing
		NiAVObject* foot;
		/// Tiny's body parts that are underfoot
		std::vector<NiAVObject*> bodyParts;
		FootEvent footEvent;
	};

	struct HighheelEquip {
		Actor* actor;
		bool equipping;
		float hhLength;
		NiPoint3 hhOffset;
		TESObjectARMO* shoe;
	};

	struct AddPerkEvent {
		Actor* actor;
		BGSPerk* perk;
		std::uint32_t rank;
	};

	struct RemovePerkEvent {
		Actor* actor;
		BGSPerk* perk;
	};

	class EventListener {
		public:
			EventListener() = default;
			~EventListener() = default;
			EventListener(EventListener const&) = delete;
			EventListener& operator=(EventListener const&) = delete;

			// Get name used for debug prints
			virtual std::string DebugName() = 0;

			// Called on Live (non paused) gameplay
			virtual void Update();

			// Called on Papyrus OnUpdate
			virtual void PapyrusUpdate();

			// Called on Havok update (when processing hitjobs)
			virtual void HavokUpdate();

			// Called when the camera update event is fired (in the TESCameraState)
			virtual void CameraUpdate();

			// Called on game load started (not yet finished)
			// and when new game is selected
			virtual void Reset();

			// Called when game is enabled (while not paused)
			virtual void Enabled();

			// Called when game is disabled (while not paused)
			virtual void Disabled();

			// Called when a game is started after a load/newgame
			virtual void Start();

			// Called when all forms are loaded (during game load before mainmenu)
			virtual void DataReady();

			// Called when an actor is reset
			virtual void ResetActor(Actor* actor);

			// Called when an actor has an item equipped
			virtual void ActorEquip(Actor* actor);

			// Called when an actor has is fully loaded
			virtual void ActorLoaded(Actor* actor);

			// Called when a papyrus hit event is fired
			virtual void HitEvent(const TESHitEvent* evt);

			// Called when an actor is squashed underfoot
			virtual void UnderFootEvent(const UnderFoot& evt);

			// Fired when a foot lands
			virtual void OnImpact(const Impact& impact);

			// Fired when a highheel is (un)equiped or when an actor is loaded with HH
			virtual void OnHighheelEquip(const HighheelEquip& evt);

			// Fired when a perk is added
			virtual void OnAddPerk(const AddPerkEvent& evt);

			// Fired when a perk about to be removed
			virtual void OnRemovePerk(const RemovePerkEvent& evt);

			// Fired when a skyrim menu event occurs
			virtual void MenuChange(const MenuOpenCloseEvent* menu_event);

			// Fired when a actor animation event occurs
			virtual void ActorAnimEvent(Actor* actor, const std::string_view& tag, const std::string_view& payload);
	};

	class EventDispatcher {
		public:
			// EventDispatcher() = default;
			// ~EventDispatcher() = default;
			// EventDispatcher(EventDispatcher const&) = delete;
			// EventDispatcher& operator=(EventDispatcher const&) = delete;

			static void AddListener(EventListener* listener);
			static void DoUpdate();
			static void DoPapyrusUpdate();
			static void DoHavokUpdate();
			static void DoCameraUpdate();
			static void DoReset();
			static void DoEnabled();
			static void DoDisabled();
			static void DoStart();
			static void DoDataReady();
			static void DoResetActor(Actor* actor);
			static void DoActorEquip(Actor* actor);
			static void DoActorLoaded(Actor* actor);
			static void DoHitEvent(const TESHitEvent* evt);
			static void DoUnderFootEvent(const UnderFoot& evt);
			static void DoOnImpact(const Impact& impact);
			static void DoHighheelEquip(const HighheelEquip& impact);
			static void DoAddPerk(const AddPerkEvent& evt);
			static void DoRemovePerk(const RemovePerkEvent& evt);
			static void DoMenuChange(const MenuOpenCloseEvent* menu_event);
			static void DoActorAnimEvent(Actor* actor, const BSFixedString& a_tag, const BSFixedString& a_payload);
		private:
			[[nodiscard]] static EventDispatcher& GetSingleton();
			std::vector<EventListener*> listeners;
	};
}
