#include "hooker.hpp"
#include "unlocker/unlocker.hpp"
#include "upc/upc.hpp"
#include "build_config.h"
#include <koalabox/logger/logger.hpp>
#include <koalabox/util/util.hpp>
#include <koalabox/win_util/win_util.hpp>

#include <polyhook2/CapstoneDisassembler.hpp>
#include <polyhook2/Detour/x86Detour.hpp>
#include <polyhook2/Detour/x64Detour.hpp>


#ifdef _WIN64
typedef PLH::x64Detour Detour;
const PLH::Mode DisassemblerMode = PLH::Mode::x64;
#else
typedef PLH::x86Detour Detour;
const PLH::Mode DisassemblerMode = PLH::Mode::x64;
#endif
PLH::CapstoneDisassembler disassembler(DisassemblerMode); // NOLINT(cert-err58-cpp)

using namespace koalabox;
using namespace hooker;

class PolyhookLogger : public PLH::Logger {
    void log(String msg, PLH::ErrorLevel level) override {
        if (level == PLH::ErrorLevel::WARN) {
            logger::warn("[Polyhook] {}", msg);
        } else if (level == PLH::ErrorLevel::SEV) {
            logger::error("[Polyhook] {}", msg);
        }
    }
};

AddressBook hooker::address_book = {}; // NOLINT(cert-err58-cpp)

Vector<Detour*> hooks; // NOLINT(cert-err58-cpp)

void hook(
    HMODULE module, const char* function_name, char* callback_function, uint64_t* trampoline_address
) {
    logger::info("Hooking '{}'", function_name);

    const auto address = win_util::get_proc_address(module, function_name);

    auto* detour = new Detour(
        (char*) address,
        callback_function,
        trampoline_address,
        disassembler
    );

    if (detour->hook()) {
        hooks.push_back(detour);
    } else {
        util::panic(__func__, "Failed to hook function: {}", function_name);
    }
}

#define HOOK(MODULE, FUNC) hook(MODULE, #FUNC, (char*) &upc::FUNC, &address_book.FUNC)

void hooker::init() {
    logger::info("Hooker initialization");

    // Initialize polyhook logger
    auto polyhook_logger = std::make_shared<PolyhookLogger>();
    PLH::Log::registerLogger(polyhook_logger);

    HOOK(unlocker::original_module, UPC_Init);
}

/**
 * Functions other than UPC_Init cannot be hooked in the loader dll,
 * hence we hook them in the store dll
 */
void hooker::post_init() {
    logger::info("Hooker post-initialization");

    // First try loading uplay_r2
    auto store_dll_name = String("uplay_r2") + (util::is_64_bit() ? "64" : "");
    auto store_dll_handle = GetModuleHandleA(store_dll_name.c_str());

    // If it fails, try upc_r2 (attempt at forward-compatibility)
    if (store_dll_handle == nullptr) {
        store_dll_name = String("upc_r2") + (util::is_64_bit() ? "64" : "");
        store_dll_handle = GetModuleHandleA(store_dll_name.c_str());
    }

    // If both fail, then we fail too
    if (store_dll_handle == nullptr) {
        util::panic("hooker::post_init", "Failed to obtain handle of *r2.dll");
    }

    HOOK(store_dll_handle, UPC_InstallLanguageGet);
    HOOK(store_dll_handle, UPC_ProductListFree);
    HOOK(store_dll_handle, UPC_ProductListGet);

    logger::info("Hooker initialization complete");
}

void hooker::shutdown() {
    logger::info("Hooker shutdown");

    for (const auto hook: hooks) {
        hook->unHook();
    }
}
