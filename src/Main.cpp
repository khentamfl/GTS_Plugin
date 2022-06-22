#include "Config.h"

#include <stddef.h>

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
    void InitializeLogging() {
        auto path = log_directory();
        if (!path) {
            report_and_fail("Unable to lookup SKSE logs directory.");
        }
        *path /= PluginDeclaration::GetSingleton()->GetName();
        *path += L".log";

        std::shared_ptr<spdlog::logger> log;
        if (IsDebuggerPresent()) {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::msvc_sink_mt>());
        } else {
            log = std::make_shared<spdlog::logger>(
                "Global", std::make_shared<spdlog::sinks::basic_file_sink_mt>(path->string(), true));
        }
        const auto& debugConfig = Gts::Config::GetSingleton().GetDebug();
        log->set_level(debugConfig.GetLogLevel());
        log->flush_on(debugConfig.GetFlushLevel());

        spdlog::set_default_logger(std::move(log));
        spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%n] [%l] [%t] [%s:%#] %v");
    }
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
SKSEPluginLoad(const LoadInterface* skse) {
    // InitializeLogging();
    //
    // auto* plugin = PluginDeclaration::GetSingleton();
    // auto version = plugin->GetVersion();
    // log::info("{} {} is loading...", plugin->GetName(), version);
    //
    //
    // Init(skse);
    //
    // auto task = SKSE::GetTaskInterface();
    //
  	// // using a lambda
  	// task->AddTask([]() {
    //   auto* console_logger = RE::ConsoleLog::GetSingleton();
  	// 	console_logger->Print("This is a task implemented with a lambda!");
    //   log::info("Task Ran.");
  	// });
    //
    // log::info("{} has finished loading.", plugin->GetName());
    return true;
}
