#pragma once

#include "config/config.hpp"
#include "upc/upc.hpp"

#include <Windows.h>

namespace unlocker {
    using namespace koalabox;

    extern HMODULE original_module;

    extern config::ProductID app_id;

    extern bool is_hooker_mode;

    void init(HMODULE module);

    void shutdown();

    void add_config_products(Map<config::ProductID, upc::Product>& products);

    void add_fetched_products(Map<config::ProductID, upc::Product>& products);

    void add_legit_products(
        Map<config::ProductID, upc::Product>& products,
        const upc::ProductList* legit_product_list
    );

    Vector<upc::Product> get_filtered_products(Map<config::ProductID, upc::Product>& products);

}
