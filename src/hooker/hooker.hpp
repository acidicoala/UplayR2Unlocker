#pragma once

#include <cstdint>

namespace hooker {

    struct AddressBook {
        uint64_t UPC_Init;
        uint64_t UPC_InstallLanguageGet;
        uint64_t UPC_ProductListFree;
        uint64_t UPC_ProductListGet;
    };

    extern AddressBook address_book;

    void init();

    void post_init();

    void shutdown();
}
