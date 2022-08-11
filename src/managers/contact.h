#pragma once
// Module that handles footsteps
#include <SKSE/SKSE.h>

using namespace std;
using namespace SKSE;
using namespace RE;

namespace Gts {
	class ContactListener : public hkpContactListener, public hkpWorldPostSimulationListener
	{

		public:
			void ContactPointCallback(const hkpContactPointEvent& a_event) override;

			void CollisionAddedCallback(const hkpCollisionEvent& a_event) override;

			void CollisionRemovedCallback(const hkpCollisionEvent& a_event) override;

			void PostSimulationCallback(hkpWorld* a_world) override;

			RE::NiPointer<RE::bhkWorld> world = nullptr;

			void detach();
			void attach(NiPointer<bhkWorld> world);
			void ensure_last();
			void sync_camera_collision_groups();
	};

	class ContactManager {
		public:
			[[nodiscard]] static ContactManager& GetSingleton() noexcept;

			void Update();
			void UpdateCameraContacts();

			ContactListener listener{};
	};
}
