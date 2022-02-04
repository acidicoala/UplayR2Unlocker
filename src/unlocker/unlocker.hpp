#pragma once

#include "config/config.hpp"
#include "upc/upc.hpp"

namespace unlocker{
    using namespace config;
    using namespace upc;

    extern HMODULE original_module;

    extern config::Config config;

    extern config::ProductID app_id;

    void init(HMODULE module);

    void shutdown();

    void add_config_products(Map<ProductID, Product>& products);

    void add_fetched_products(Map<ProductID, Product>& products);

    void add_legit_products(Map<ProductID, Product>& products, const ProductList* legit_product_list);

    Vector<Product> get_filtered_products(Map<ProductID, Product>& products);

}
