#include "unlocker.hpp"
#include "upc/upc.hpp"

#include "koalabox/config_parser/config_parser.hpp"
#include "koalabox/loader/loader.hpp"
#include "koalabox/util/util.hpp"
#include "koalabox/win_util/win_util.hpp"

#include <build_config.h>

#define DETOUR(MODULE, FUNC) \
    hook::detour_or_throw( \
        MODULE, \
        loader::get_undecorated_function(original_library, #FUNC), \
        reinterpret_cast<FunctionPointer>(FUNC) \
    );

namespace unlocker {
    Config config = {}; // NOLINT(cert-err58-cpp)

    ProductID global_app_id = 0;

    HMODULE original_library = nullptr;

    bool is_hook_mode = false;

    void init(const HMODULE& self_module) {
        DisableThreadLibraryCalls(self_module);

        const auto self_directory = loader::get_module_dir(self_module);

        config = config_parser::parse<Config>(self_directory / PROJECT_NAME".jsonc", true);

        if (config.logging) {
            logger = file_logger::create(self_directory / PROJECT_NAME".log");
        }

        logger->info("🐨 {} 🔓 v{}", PROJECT_NAME, PROJECT_VERSION);

        const auto module_path = win_util::get_module_file_name(self_module);

        is_hook_mode = hook::is_hook_mode(self_module, ORIGINAL_DLL);

        if (is_hook_mode) {
            logger->info("🪝 Detected hook mode"); // New hook emoji:

            dll_monitor::init(
                Vector<String>{ ORIGINAL_DLL, LEGACY_ORIGINAL_DLL },
                [](const HMODULE& loader_library, const String& library_name) {
                    original_library = loader_library;

                    hook::init();

                    DETOUR(loader_library, UPC_Init)

                    dll_monitor::shutdown();
                }
            );
        } else {
            logger->info("🔀 Detected proxy mode");

            original_library = loader::load_original_library(self_directory, ORIGINAL_DLL);
        }

        logger->info("🚀 Initialization complete");
    }

    void post_init() {
        logger->debug("Unlocker post-initialization");

        dll_monitor::init(
            Vector<String>{ STORE_DLL, LEGACY_STORE_DLL },
            [](const HMODULE& store_library, const String& library_name) {
                DETOUR(store_library, UPC_InstallLanguageGet)
                DETOUR(store_library, UPC_ProductListFree)
                DETOUR(store_library, UPC_ProductListGet)

                dll_monitor::shutdown();

                logger->info("Hooking complete");
            }
        );
    }

    void shutdown() {
        if (not is_hook_mode) {
            win_util::free_library(original_library);
        }

        logger->info("💀 Shutdown complete");
    }

}

