#include "Config.hpp"
#include "hooks/hooks.hpp"
#include "papyrus/papyrus.hpp"
#include "data/plugin.hpp"

#include "events.hpp"
#include "managers/register.hpp"
#include "UI/DebugAPI.hpp"
#include "data/runtime.hpp"
#include "data/persistent.hpp"
#include "data/transient.hpp"
#include "spring.hpp"

#include <stddef.h>
#include <thread>

using namespace RE::BSScript;
using namespace Gts;
using namespace SKSE;
using namespace SKSE::log;
using namespace SKSE::stl;

namespace {


	/**
	 * Setup logging.
	 *
	 * <p>
	 * Logging is important to track issues. CommonLibSSE bundles functionality for spdlog, a common C++ logging
	 * framework. Here we initialize it, using values from the configuration file. This includes support for a debug
	 * logger that shows output in your IDE when it has a debugger attached to Skyrim, as well as a file logger which
	 * writes data to the standard SKSE logging directory at <code>Documents/My Games/Skyrim Special Edition/SKSE</code>
	 * (or <code>Skyrim VR</code> if you are using VR).
	 * </p>
	 */
	void InitializeLogging()
	{
		auto path = log_directory();

		if (!path) {
			report_and_fail("Unable to lookup SKSE logs directory.");
		}
		*path /= PluginDeclaration::GetSingleton()->GetName();
		*path += L".log";

		std::shared_ptr <spdlog::logger> log;

		if (IsDebuggerPresent()) {
			log = std::make_shared <spdlog::logger>(
				"Global", std::make_shared <spdlog::sinks::msvc_sink_mt>());
		} else {
			log = std::make_shared <spdlog::logger>(
				"Global", std::make_shared <spdlog::sinks::basic_file_sink_mt>(path->string(), true));
		}

		log->set_level(spdlog::level::level_enum::info);
		log->flush_on(spdlog::level::level_enum::trace);
		log->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
		log->info("Logging started");
		const auto& debugConfig = Gts::Config::GetSingleton().GetDebug();

		log->set_level(debugConfig.GetLogLevel());
		log->flush_on(debugConfig.GetFlushLevel());

		spdlog::set_default_logger(std::move(log));
		spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
	}


	/**
	 * Register to listen for messages.
	 *
	 * <p>
	 * SKSE has a messaging system to allow for loosely coupled messaging. This means you don't need to know about or
	 * link with a message sender to receive their messages. SKSE itself will send messages for common Skyrim lifecycle
	 * events, such as when SKSE plugins are done loading, or when all ESP plugins are loaded.
	 * </p>
	 *
	 * <p>
	 * Here we register a listener for SKSE itself (because we have not specified a message source). Plugins can send
	 * their own messages that other plugins can listen for as well, although that is not demonstrated in this example
	 * and is not common.
	 * </p>
	 *
	 * <p>
	 * The data included in the message is provided as only a void pointer. It's type depends entirely on the type of
	 * message, and some messages have no data (<code>dataLen</code> will be zero).
	 * </p>
	 */
	OnPluginMessage("SKSE", MessagingInterface::kDataLoaded, 15, msg) {
		// It is now safe to access form data.
		Cprint("[GTSPlugin.dll]: [ Giantess Mod v 1.78 was succesfully initialized. Waiting for New Game/Save Load. ]");
		EventDispatcher::DoDataReady();
	}

	OnPluginMessage("SKSE", MessagingInterface::kPostLoadGame, 15, msg) {
		// Player's selected save game has finished loading.
		// Data will be a boolean indicating whether the load was successful.
		Plugin::SetInGame(true);
		Cprint(" [GTSPlugin.dll]: [ Giantess Mod was succesfully initialized and loaded. ]");
	}
	OnPluginMessage("SKSE", MessagingInterface::kNewGame, 15, msg) {
		// Player starts a new game from main menu.
		Plugin::SetInGame(true);
		EventDispatcher::DoReset();
		Cprint(" [GTSPlugin.dll]: [ Giantess Mod was succesfully initialized and loaded. ]");
	}
	OnPluginMessage("SKSE", MessagingInterface::kPreLoadGame, 15, msg) {
		// Player selected a game to load, but it hasn't loaded yet.
		// Data will be the name of the loaded save.
		Plugin::SetInGame(false);
		EventDispatcher::DoReset();
	}

	void InitializeSerialization() {
		log::trace("Initializing cosave serialization...");
		auto* serde = GetSerializationInterface();
		serde->SetUniqueID(_byteswap_ulong('GTSP'));
		serde->SetSaveCallback(Persistent::OnGameSaved);
		serde->SetRevertCallback(Persistent::OnRevert);
		serde->SetLoadCallback(Persistent::OnGameLoaded);
		log::info("Cosave serialization initialized.");
	}

	void InitializePapyrus() {
		log::trace("Initializing Papyrus binding...");
		if (GetPapyrusInterface()->Register(Gts::register_papyrus)) {
			log::info("Papyrus functions bound.");
		} else {
			stl::report_and_fail("Failure to register Papyrus bindings.");
		}
	}

	void InitializeEventSystem() {
		EventDispatcher::AddListener(&DebugOverlayMenu::GetSingleton());
		EventDispatcher::AddListener(&Runtime::GetSingleton()); // Stores spells, globals and other important data
		EventDispatcher::AddListener(&Persistent::GetSingleton());
		EventDispatcher::AddListener(&Transient::GetSingleton());

		EventDispatcher::AddListener(&SpringManager::GetSingleton());
		log::info("Adding Default Listeners");
		RegisterManagers();
	}

	/**
	 * This if the main callback for initializing your SKSE plugin, called just before Skyrim runs its main function.
	 *
	 * <p>
	 * This is your main entry point to your plugin, where you should initialize everything you need. Many things can't be
	 * done yet here, since Skyrim has not initialized and the Windows loader lock is not released (so don't do any
	 * multithreading). But you can register to listen for messages for later stages of Skyrim startup to perform such
	 * tasks.
	 * </p>
	 */
	OnSKSEPluginLoad(-10)
	{
		InitializeLogging();
		auto *plugin  = PluginDeclaration::GetSingleton();
		auto version = plugin->GetVersion();

		log::info("{} {} is loading...", plugin->GetName(), version);
	}


	OnSKSEPluginLoad(10) {
		Hooks::Install();
		InitializePapyrus();
		InitializeSerialization();
		InitializeEventSystem();

		log::info("{} has finished loading.", plugin->GetName());
	}
}
