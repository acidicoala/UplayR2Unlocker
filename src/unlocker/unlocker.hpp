#pragma once

#include "koalabox/koalabox.hpp"

#define GET_ORIGINAL_FUNCTION(FUNC) \
    static const auto FUNC##_o = hook::get_original_function( \
        unlocker::is_hook_mode, unlocker::original_library, #FUNC, FUNC \
    );

namespace unlocker {
    using namespace koalabox;

    typedef uint32_t ProductID;

    struct Config {
        bool logging = false;
        String lang = "default";
        bool auto_fetch = true;
        Set<ProductID> dlcs;
        Set<ProductID> items;
        Set<ProductID> blacklist;

        NLOHMANN_DEFINE_TYPE_INTRUSIVE(Config, logging, lang, auto_fetch, dlcs, items, blacklist)
    };

    extern Config config;

    extern HMODULE original_library;

    extern ProductID global_app_id;

    extern bool is_hook_mode;

    void init(const HMODULE& loader_library);

    void post_init();

    void shutdown();

}
