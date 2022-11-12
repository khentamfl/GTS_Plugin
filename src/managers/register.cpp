#include "managers/register.hpp"

#include "events.hpp"
#include "magic/magic.hpp"
#include "managers/highheel.hpp"
#include "managers/GtsSizeManager.hpp"
#include "managers/InputManager.hpp"
#include "managers/Attributes.hpp"
#include "managers/contact.hpp"
#include "managers/RandomGrowth.hpp"
#include "managers/GtsQuest.hpp"
#include "managers/GtsManager.hpp"
#include "managers/reloader.hpp"
#include "managers/altcamera.hpp"
#include "managers/hitmanager.hpp"
#include "managers/vore.hpp"
#include "managers/CrushManager.hpp"
#include "managers/ShrinkToNothingManager.hpp"

namespace Gts {
	void RegisterManagers() {
		EventDispatcher::AddListener(&GtsManager::GetSingleton()); // Manages Game Mode, smooth size increase and animation & movement speed
		EventDispatcher::AddListener(&SizeManager::GetSingleton()); // Manages Max Scale of everyone
		EventDispatcher::AddListener(&HighHeelManager::GetSingleton()); // Applies high heels
		EventDispatcher::AddListener(&CameraManager::GetSingleton()); // Edits the camera
		EventDispatcher::AddListener(&ReloadManager::GetSingleton()); // Handles Skyrim Events

		EventDispatcher::AddListener(&MagicManager::GetSingleton()); // Manages spells and size changes in general
		EventDispatcher::AddListener(&Vore::GetSingleton()); // Manages vore
		EventDispatcher::AddListener(&CrushManager::GetSingleton()); // Manages crushing
		EventDispatcher::AddListener(&ShrinkToNothingManager::GetSingleton()); // Shrink to nothing manager

		EventDispatcher::AddListener(&AttributeManager::GetSingleton()); // Adjusts most attributes
		EventDispatcher::AddListener(&RandomGrowth::GetSingleton()); // Manages random growth perk
		EventDispatcher::AddListener(&QuestManager::GetSingleton()); // Quest is currently empty and not needed
		EventDispatcher::AddListener(&HitManager::GetSingleton()); // Hit Manager for handleing papyrus hit events

		EventDispatcher::AddListener(&ContactManager::GetSingleton()); // Manages collisions
		EventDispatcher::AddListener(&InputManager::GetSingleton()); // Manages keyboard and mouse input
	}
}
