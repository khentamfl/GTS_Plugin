#include "managers/contact.h"
#include "data/persistent.h"

#include "util.h"

using namespace SKSE;
using namespace RE;
using namespace REL;
using namespace Gts;

namespace {
	// From https://github.com/ersh1/Precision, https://github.com/adamhynek/activeragdoll/ and https://github.com/adamhynek/higgs
	enum class WorldExtensionIds : int32_t
	{
		kAnonymous = -1,
		kBreakOffParts = 1000,
		kCollisionCallback = 1001
	};


	hkpWorldExtension* findWorldExtension(hkpWorld* a_world, WorldExtensionIds a_id) {
		using func_t = decltype(&findWorldExtension);
		REL::Relocation<func_t> func{ RELOCATION_ID(60549, 61397) };
		return func(a_world, a_id);
	}

	bool requireCollisionCallbackUtil(hkpWorld* a_world) {
		using func_t = decltype(&requireCollisionCallbackUtil);
		REL::Relocation<func_t> func{ RELOCATION_ID(60588, 61437) };
		return func(a_world);
	}

	bool releaseCollisionCallbackUtil(hkpWorld* a_world) {
		using func_t = decltype(&releaseCollisionCallbackUtil);
		REL::Relocation<func_t> func{ RELOCATION_ID(61800, 62715) };
		return func(a_world);
	}

	void addContactListener(RE::hkpWorld* a_world, RE::hkpContactListener* a_worldListener){
		using func_t = decltype(&addContactListener);
		REL::Relocation<func_t> func{ RELOCATION_ID(60543, 61383) };
		return func(a_world, a_worldListener);
	}

	void removeContactListener(hkpWorld* a_this, hkpContactListener* a_worldListener)
	{
		hkArray<hkpContactListener*>& listeners = a_this->contactListeners;

		for (int i = 0; i < listeners.size(); i++) {
			hkpContactListener* listener = listeners[i];
			if (listener == a_worldListener) {
				listeners[i] = nullptr;
				return;
			}
		}
	}

	void addWorldPostSimulationListener(RE::hkpWorld* a_world, RE::hkpWorldPostSimulationListener* a_worldListener) {
		using func_t = decltype(&addWorldPostSimulationListener);
		REL::Relocation<func_t> func{ RELOCATION_ID(60538, 61366) };
		return func(a_world, a_worldListener);
	}

	void removeWorldPostSimulationListener(RE::hkpWorld* a_world, RE::hkpWorldPostSimulationListener* a_worldListener) {
		using func_t = decltype(&removeWorldPostSimulationListener);
		REL::Relocation<func_t> func{ RELOCATION_ID(60539, 61367) };
		return func(a_world, a_worldListener);
	}

	void print_collision_groups(std::uint64_t flags) {
		std::map<std::string, COL_LAYER> named_layers {
			{ "kStatic", COL_LAYER::kStatic },
			{ "kAnimStatic", COL_LAYER::kAnimStatic },
			{ "kTransparent", COL_LAYER::kTransparent },
			{ "kClutter", COL_LAYER::kClutter },
			{ "kWeapon", COL_LAYER::kWeapon },
			{ "kProjectile", COL_LAYER::kProjectile },
			{ "kSpell", COL_LAYER::kSpell },
			{ "kBiped", COL_LAYER::kBiped },
			{ "kTrees", COL_LAYER::kTrees },
			{ "kProps", COL_LAYER::kProps },
			{ "kWater", COL_LAYER::kWater },
			{ "kTrigger", COL_LAYER::kTrigger },
			{ "kTerrain", COL_LAYER::kTerrain },
			{ "kTrap", COL_LAYER::kTrap },
			{ "kNonCollidable", COL_LAYER::kNonCollidable },
			{ "kCloudTrap", COL_LAYER::kCloudTrap },
			{ "kGround", COL_LAYER::kGround },
			{ "kPortal", COL_LAYER::kPortal },
			{ "kDebrisSmall", COL_LAYER::kDebrisSmall },
			{ "kDebrisLarge", COL_LAYER::kDebrisLarge },
			{ "kAcousticSpace", COL_LAYER::kAcousticSpace },
			{ "kActorZone", COL_LAYER::kActorZone },
			{ "kProjectileZone", COL_LAYER::kProjectileZone },
			{ "kGasTrap", COL_LAYER::kGasTrap },
			{ "kShellCasting", COL_LAYER::kShellCasting },
			{ "kTransparentWall", COL_LAYER::kTransparentWall },
			{ "kInvisibleWall", COL_LAYER::kInvisibleWall },
			{ "kTransparentSmallAnim", COL_LAYER::kTransparentSmallAnim },
			{ "kClutterLarge", COL_LAYER::kClutterLarge },
			{ "kCharController", COL_LAYER::kCharController },
			{ "kStairHelper", COL_LAYER::kStairHelper },
			{ "kDeadBip", COL_LAYER::kDeadBip },
			{ "kBipedNoCC", COL_LAYER::kBipedNoCC },
			{ "kAvoidBox", COL_LAYER::kAvoidBox },
			{ "kCollisionBox", COL_LAYER::kCollisionBox },
			{ "kCameraSphere", COL_LAYER::kCameraSphere },
			{ "kDoorDetection", COL_LAYER::kDoorDetection },
			{ "kConeProjectile", COL_LAYER::kConeProjectile },
			{ "kCamera", COL_LAYER::kCamera },
			{ "kItemPicker", COL_LAYER::kItemPicker },
			{ "kLOS", COL_LAYER::kLOS },
			{ "kPathingPick", COL_LAYER::kPathingPick },
			{ "kSpellExplosion", COL_LAYER::kSpellExplosion },
			{ "kDroppingPick", COL_LAYER::kDroppingPick },
		};
		for (const auto& [key, value] : named_layers) {
			auto layer_flag = (static_cast<uint64_t>(1) << static_cast<uint64_t>(value));
			if ((flags & layer_flag) != 0) {
				log::info(" - Collides with {}", key);
			}
		}

	}

	void sync_camera_collision_groups(NiPointer<bhkWorld> world) {
		// Default groups:
		//  CameraSphere Collision Groups
		//   - Collides with kAcousticSpace
		//   - Collides with kDebrisLarge
		//   - Collides with kDroppingPick
		//   - Collides with kItemPicker
		//   - Collides with kPortal
		//   - Collides with kShellCasting
		//   - Collides with kWater
		//  Camera Collision Groups
		//   - Collides with kAnimStatic
		//   - Collides with kBiped
		//   - Collides with kCharController
		//   - Collides with kCloudTrap
		//   - Collides with kDebrisLarge
		//   - Collides with kGround
		//   - Collides with kItemPicker
		//   - Collides with kLOS
		//   - Collides with kStatic
		//   - Collides with kTerrain
		//   - Collides with kTransparent
		//   - Collides with kTransparentSmallAnim
		//   - Collides with kTransparentWall
		//   - Collides with kTrap
		//   - Collides with kTrees
		if (!world) return;
		PlayerCharacter* player = PlayerCharacter::GetSingleton();
		if (!player) return;
		auto player_data = Persistent::GetSingleton().GetData(player);
		if (!player_data) return;
		auto& camera_collisions = Persistent::GetSingleton().camera_collisions;

		float scale = player_data->target_scale;
		BSWriteLockGuard lock(world->worldLock);

		RE::bhkCollisionFilter* filter = static_cast<bhkCollisionFilter*>(world->GetWorld2()->collisionFilter);

		if (!camera_collisions.enable_actor && scale >= camera_collisions.above_scale) {
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] &= ~(static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kBiped));
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] &= ~(static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kCharController));
		} else {
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] |= (static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kBiped));
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] |= (static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kCharController));
		}
		if (!camera_collisions.enable_debris && scale >= camera_collisions.above_scale) {
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] &= ~(static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kDebrisLarge));
		} else {
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] |= (static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kDebrisLarge));
		}
		if (!camera_collisions.enable_trees && scale >= camera_collisions.above_scale) {
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] &= ~(static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kTrees));
		} else {
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] |= (static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kTrees));
		}
		if (!camera_collisions.enable_terrain && scale >= camera_collisions.above_scale) {
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] &= ~(static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kTerrain));
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] &= ~(static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kGround));
		} else {
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] |= (static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kTerrain));
			filter->layerBitfields[static_cast<uint8_t>(COL_LAYER::kCamera)] |= (static_cast<uint64_t>(1) << static_cast<uint64_t>(COL_LAYER::kGround));
		}
	}
}

namespace Gts {

	void ContactListener::ContactPointCallback(const hkpContactPointEvent& a_event)
	{
		auto rigid_a = a_event.bodies[0];
		if (!rigid_a) return;
		auto rigid_b = a_event.bodies[1];
		if (!rigid_b) return;
		auto objref_a = rigid_a->GetUserData();
		if (!objref_a) return;
		auto objref_b = rigid_b->GetUserData();
		if (!objref_b) return;
		if (objref_a->GetFormType() == Actor::FORMTYPE && objref_b->GetFormType() == Actor::FORMTYPE) {
			Actor* actor_a = skyrim_cast<Actor*>(objref_a);
			if (!actor_a) return;
			Actor* actor_b = skyrim_cast<Actor*>(objref_b);
			if (!actor_b) return;
			if (actor_a == actor_b) return;
			auto name_a = actor_a->GetDisplayFullName();
			if (!name_a) return;
			auto name_b = actor_b->GetDisplayFullName();
			if (!name_b) return;
			log::info("Colliding: {} with: {}", name_a, name_b);
		}
		// log::info("ContactPointCallback");
	}

	void ContactListener::CollisionAddedCallback(const hkpCollisionEvent& a_event)
	{
		// log::info("CollisionAddedCallback");
	}

	void ContactListener::CollisionRemovedCallback(const hkpCollisionEvent& a_event)
	{
		// log::info("CollisionRemovedCallback");
	}

	void ContactListener::PostSimulationCallback(hkpWorld* a_world)
	{
		// log::info("PostSimulationCallback");
	}

	void ContactListener::detach() {
		if (world) {
			BSWriteLockGuard lock(world->worldLock);
			auto collisionCallbackExtension = findWorldExtension(world->GetWorld2(), WorldExtensionIds::kCollisionCallback);
			if (collisionCallbackExtension) {
				releaseCollisionCallbackUtil(world->GetWorld2());
			}
			removeContactListener(world->GetWorld2(), this);
			removeWorldPostSimulationListener(world->GetWorld2(), this);
			this->world = nullptr;
		}
	}
	void ContactListener::attach(NiPointer<bhkWorld> world) {
		// Only runs if current world is nullptr and new is not
		if (!this->world && world) {
			this->world = world;
			BSWriteLockGuard lock(world->worldLock);
			requireCollisionCallbackUtil(world->GetWorld2());
			addContactListener(world->GetWorld2(), this);
			addWorldPostSimulationListener(world->GetWorld2(), this);
		}
	}

	void ContactListener::ensure_last() {
		// Ensure our listener is the last one (will be called first)
		hkArray<hkpContactListener*>& listeners = world->GetWorld2()->contactListeners;
		if (listeners[listeners.size() - 1] != this) {
			BSWriteLockGuard lock(world->worldLock);

			int numListeners = listeners.size();
			int listenerIndex = -1;

			// get current index of our listener
			for (int i = 0; i < numListeners; ++i) {
				if (listeners[i] == this) {
					listenerIndex = i;
					break;
				}
			}

			if (listenerIndex >= 0) {
				for (int i = listenerIndex + 1; i < numListeners; ++i) {
					listeners[i - 1] = listeners[i];
				}
				listeners[numListeners - 1] = this;
			}
		}
	}


	ContactManager& ContactManager::GetSingleton() noexcept {
		static ContactManager instance;
		return instance;
	}

	void ContactManager::Update() {
		auto playerCharacter = PlayerCharacter::GetSingleton();

		auto cell = playerCharacter->GetParentCell();
		if (!cell) return;

		auto world = RE::NiPointer<RE::bhkWorld>(cell->GetbhkWorld());
		if (!world) return;
		ContactListener& contactListener = this->listener;

		if (contactListener.world != world) {
			contactListener.detach();
			contactListener.attach(world);
			contactListener.ensure_last();
		}
		if (world) {
			sync_camera_collision_groups(world);
		}
	}
}
