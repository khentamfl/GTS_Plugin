#include "managers/contact.h"

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

	typedef hkpWorldExtension* (*thkpWorld_findWorldExtension)(hkpWorld* a_world, hkpKnownWorldExtensionIds a_id);
	static Relocation<thkpWorld_findWorldExtension> findWorldExtension{ RELOCATION_ID(60549, 61397) };

	typedef bool (*thkpCollisionCallbackUtil_requireCollisionCallbackUtil)(hkpWorld* a_world);
	static Relocation<thkpCollisionCallbackUtil_requireCollisionCallbackUtil> requireCollisionCallbackUtil{ RELOCATION_ID(60588, 61437) };

	typedef bool (*thkpCollisionCallbackUtil_releaseCollisionCallbackUtil)(hkpWorld* a_world);
	static Relocation<thkpCollisionCallbackUtil_releaseCollisionCallbackUtil> releaseCollisionCallbackUtil{ RELOCATION_ID(61800, 62715) };

	typedef void* (*thkpWorld_addContactListener)(RE::hkpWorld* a_world, RE::hkpContactListener* a_worldListener);
	static REL::Relocation<thkpWorld_addContactListener> addContactListener{ RELOCATION_ID(60543, 61383) };

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

	typedef void* (*thkpWorld_addWorldPostSimulationListener)(RE::hkpWorld* a_world, RE::hkpWorldPostSimulationListener* a_worldListener);
	static REL::Relocation<thkpWorld_addWorldPostSimulationListener> addWorldPostSimulationListener{ RELOCATION_ID(60538, 61366) };

	typedef void* (*thkpWorld_removeWorldPostSimulationListener)(RE::hkpWorld* a_world, RE::hkpWorldPostSimulationListener* a_worldListener);
	static REL::Relocation<thkpWorld_removeWorldPostSimulationListener> removeWorldPostSimulationListener{ RELOCATION_ID(60539, 61367) };
}

namespace Gts {

	void ContactListener::ContactPointCallback(const hkpContactPointEvent& a_event)
	{
		log::info("ContactPointCallback");
	}

	void ContactListener::CollisionAddedCallback(const hkpCollisionEvent& a_event)
	{
		log::info("CollisionAddedCallback");
	}

	void ContactListener::CollisionRemovedCallback(const hkpCollisionEvent& a_event)
	{
		log::info("CollisionRemovedCallback");
	}

	void ContactListener::PostSimulationCallback(hkpWorld* a_world)
	{
		log::info("PostSimulationCallback");
	}

	void ContactListener::detach() {
		if (world) {
			BSWriteLockGuard lock(world->worldLock);
			auto collisionCallbackExtension = findWorldExtension(world, WorldExtensionIds::kCollisionCallback);
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
	}
}
