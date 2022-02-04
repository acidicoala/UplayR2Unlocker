#pragma once

#include "koalabox/koalabox.hpp"

#include <nlohmann/json.hpp>

namespace config {

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

    Config read(Path path);

}
