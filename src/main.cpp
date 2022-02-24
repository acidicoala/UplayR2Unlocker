#include "unlocker/unlocker.hpp"

// This header will be populated at build time
#include <linker_exports.h>

EXTERN_C BOOL WINAPI DllMain(HMODULE module, DWORD reason, LPVOID) {
    if (reason == DLL_PROCESS_ATTACH) {
        unlocker::init(module);
    } else if (reason == DLL_PROCESS_DETACH) {
        unlocker::shutdown();
    }

    return TRUE;
}
