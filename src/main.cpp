#include "unlocker/unlocker.hpp"

#include <Windows.h>

// This header will be populated at build time
#include <linker_exports.h>

[[maybe_unused]]
BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        unlocker::init(module);
    } else if (reason == DLL_PROCESS_DETACH) {
        unlocker::shutdown();
    }

    return TRUE;
}
