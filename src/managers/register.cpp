#include "managers/register.hpp"
#include "events.hpp"
#include "magic/magic.hpp"
#include "managers/ShrinkToNothingManager.hpp"
#include "managers/damage/AccurateDamage.hpp"
#include "managers/animation/AnimationManager.hpp"
#include "managers/animation/ActorVore.hpp"
#include "managers/animation/Grab.hpp"
#include "managers/highheel.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/contact.hpp"
#include "managers/RandomGrowth.hpp"
#include "managers/GtsQuest.hpp"
#include "managers/GtsManager.hpp"
#include "managers/reloader.hpp"
#include "managers/camera.hpp"
#include "managers/hitmanager.hpp"
#include "managers/vore.hpp"
#include "managers/CrushManager.hpp"
#include "managers/tremor.hpp"
#include "managers/footstep.hpp"
#include "managers/explosion.hpp"
#include "managers/rumble.hpp"

namespace Gts {
	void RegisterManagers() {
		EventDispatcher::AddListener(&GtsManager::GetSingleton()); // Manages Game Mode, smooth size increase and animation & movement speed
		EventDispatcher::AddListener(&SizeManager::GetSingleton()); // Manages Max Scale of everyone
		EventDispatcher::AddListener(&HighHeelManager::GetSingleton()); // Applies high heels
		EventDispatcher::AddListener(&CameraManager::GetSingleton()); // Edits the camera
		EventDispatcher::AddListener(&ReloadManager::GetSingleton()); // Handles Skyrim Events
		EventDispatcher::AddListener(&AccurateDamage::GetSingleton()); // Handles precise size-related damage

		EventDispatcher::AddListener(&MagicManager::GetSingleton()); // Manages spells and size changes in general
		EventDispatcher::AddListener(&Vore::GetSingleton()); // Manages vore
		EventDispatcher::AddListener(&CrushManager::GetSingleton()); // Manages crushing
		EventDispatcher::AddListener(&ShrinkToNothingManager::GetSingleton()); // Shrink to nothing manager

		EventDispatcher::AddListener(&FootStepManager::GetSingleton()); // Manages footstep sounds
		EventDispatcher::AddListener(&TremorManager::GetSingleton()); // Manages tremors on footstop
		EventDispatcher::AddListener(&ExplosionManager::GetSingleton()); // Manages clouds/exposions on footstep
		EventDispatcher::AddListener(&Rumble::GetSingleton()); // Manages rumbling of contoller/camera for multiple frames

		EventDispatcher::AddListener(&AttributeManager::GetSingleton()); // Adjusts most attributes
		EventDispatcher::AddListener(&RandomGrowth::GetSingleton()); // Manages random growth perk
		EventDispatcher::AddListener(&QuestManager::GetSingleton()); // Quest is currently empty and not needed
		EventDispatcher::AddListener(&HitManager::GetSingleton()); // Hit Manager for handleing papyrus hit events

		EventDispatcher::AddListener(&AnimationManager::GetSingleton()); // Manages Animation Events

		// EventDispatcher::AddListener(&Stomp::GetSingleton()); // Manages Stomp Anim events
		// EventDispatcher::AddListener(&ThighCrush::GetSingleton()); // Manages Thigh Crushing
		EventDispatcher::AddListener(&Grab::GetSingleton()); // Manages grabbing
		EventDispatcher::AddListener(&ActorVore::GetSingleton()); // Manages Vore animation Events

		EventDispatcher::AddListener(&ContactManager::GetSingleton()); // Manages collisions
		EventDispatcher::AddListener(&InputManager::GetSingleton()); // Manages keyboard and mouse input
	}
}
