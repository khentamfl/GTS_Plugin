#pragma once

#include <RE/Skyrim.h>

namespace Gts {
    bool RegisterHitCounter(RE::BSScript::IVirtualMachine* vm);

    void InitializeHook(SKSE::Trampoline& trampoline);
}
