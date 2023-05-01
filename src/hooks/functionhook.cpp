#include <hooks/functionhook.hpp>

#include <stdexcept>
#include "hooks/Detours/detours.h"

#define MAX_RETRY 3

namespace Hooks {

  void FunctionHook<void>::Attach(void** target, void* hook) {
      uintptr_t base = REL::Module::get().base();
      Logger::Debug("Attaching function hook to address 0x{:X} (offset from image base of 0x{:X} by 0x{:X}...",
                    reinterpret_cast<uintptr_t>(*target), base, reinterpret_cast<uintptr_t>(*target) - base);
      for (std::size_t i = 0; i < 3; ++i) {
          auto result = DetourTransactionBegin();
          if (result != NO_ERROR) {
              Logger::Fatal("Failed to start transaction for unknown reason (error code {}).", result);
          }
          Logger::Trace("Initiated transaction for function hook...");
          result = DetourUpdateThread(GetCurrentThread());
          switch (result) {
              case NO_ERROR:
                  Logger::Trace("Function hook transaction thread information written...");
                  break;
              case ERROR_NOT_ENOUGH_MEMORY:
                  DetourTransactionAbort();
                  Logger::Fatal(
                      "Not enough memory to obtain thread information for function hook transaction, "
                      "aborting operation.");
              default:
                  DetourTransactionAbort();
                  Logger::Fatal("Failed to obtain transaction thread information for unknown reason (error code {}).",
                                result);
          }
          result = DetourAttach(target, hook);
          switch (result) {
              case NO_ERROR:
                  Logger::Trace("Function hook attached successfully in transaction, committing transaction...");
                  break;
              case ERROR_INVALID_BLOCK:
                  DetourTransactionAbort();
                  Logger::Fatal<std::logic_error>(
                      ""
                      "The target function is too small to attach a function hook, aborting operation.");
              case ERROR_INVALID_HANDLE:
                  DetourTransactionAbort();
                  Logger::Fatal<std::invalid_argument>(
                      "The target function does refer to a valid address, aborting operation.");
              case ERROR_NOT_ENOUGH_MEMORY:
                  DetourTransactionAbort();
                  Logger::Fatal<std::runtime_error>("Not enough memory to attach function hook; aborting operation.");
              default:
                  DetourTransactionAbort();
                  Logger::Fatal("Failed to attach function hook for unknown reason (error code {}).", result);
          }
          result = DetourTransactionCommit();
          switch (result) {
              case NO_ERROR:
                  Logger::Debug("Function hook to address {} committed, function hook is now active.",
                                reinterpret_cast<uintptr_t>(*target));
                  return;
              case ERROR_INVALID_DATA:
                  Logger::Warn(
                      "Detected target of function hook was modified by another thread before committing "
                      "the hook. Retrying...");
                  continue;
              default:
                  DetourTransactionAbort();
                  Logger::Fatal("Failed to commit function hook for unknown reason (error code {}).", result);
          }
      }
      log::error("Unable to commit function hook after {} retries without "
               "another thread modifying the target function. Operation aborted.",
          MAX_RETRY);
  }

  void FunctionHook<void>::Detach(void** target, void* hook) {
      uintptr_t base = REL::Module::get().base();
      log::error("Detaching function hook from address 0x{:X} (offset from image base of 0x{:X} by 0x{:X}...",
                    reinterpret_cast<uintptr_t>(*target), base, reinterpret_cast<uintptr_t>(*target) - base);
      for (std::size_t i = 0; i < MAX_RETRY; ++i) {
          auto result = DetourTransactionBegin();
          if (result != NO_ERROR) {
              Logger::Fatal("Failed to start transaction for unknown reason (error code {}).", result);
          }
          log::trace("Initiated transaction for function hook...");
          result = DetourUpdateThread(GetCurrentThread());
          switch (result) {
              case NO_ERROR:
                  log::trace("Function hook transaction thread information written...");
                  break;
              case ERROR_NOT_ENOUGH_MEMORY:
                  DetourTransactionAbort();
                  log::error(
                      "Not enough memory to obtain thread information for function hook transaction, "
                      "aborting operation.");
              default:
                  DetourTransactionAbort();
                  log::error("Failed to obtain transaction thread information for unknown reason (error code {}).",
                                result);
          }
          result = DetourDetach(target, hook);
          if (result != NO_ERROR) {
              DetourTransactionAbort();
              log::error("Failed to detach function hook for unknown reason (error code {}).", result);
          }
          log::trace("Function hook detached successfully in transaction, committing transaction...");
          result = DetourTransactionCommit();
          switch (result) {
              case NO_ERROR:
                  Logger::Debug("Function hook detachment for address {} committed, function hook has been removed.",
                                reinterpret_cast<uintptr_t>(*target));
                  return;
              case ERROR_INVALID_DATA:
                  Logger::Warn(
                      "Detected target of function hook was modified by another thread before committing "
                      "the hook. Retrying...");
                  continue;
              default:
                  DetourTransactionAbort();
                  Logger::Fatal("Failed to commit function hook detachment for unknown reason (error code {}).", result);
          }
      }
      log::error("Unable to commit function hook detachment after {} retries "
               "without another thread modifying the target function. Operation aborted.",MAX_RETRY);
  }

}
