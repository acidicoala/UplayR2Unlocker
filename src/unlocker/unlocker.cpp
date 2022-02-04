#include "unlocker.hpp"
#include "upc/upc.hpp"
#include "config/config.hpp"
#include "build_config.h"

#include "koalabox/logger/logger.hpp"
#include "koalabox/util/util.hpp"
#include "koalabox/win_util/win_util.hpp"

#include <cpr/cpr.h>

using namespace koalabox;
using namespace unlocker;

config::Config unlocker::config = {}; // NOLINT(cert-err58-cpp)

HMODULE unlocker::original_module = nullptr;

ProductID unlocker::app_id = 0;

void unlocker::init(HMODULE self_module) {
    DisableThreadLibraryCalls(self_module);

    koalabox::project_name = PROJECT_NAME;

    const auto self_directory = util::get_module_dir(self_module);

    unlocker::config = config::read(self_directory / PROJECT_NAME".jsonc");

    if (unlocker::config.logging) {
        logger::init(self_directory / PROJECT_NAME".log");
    }

    logger::info("🐨 {} 🔓 v{}", PROJECT_NAME, PROJECT_VERSION);

    const auto original_module_path = util::get_module_dir(self_module) / ORIG_DLL".dll";
    original_module = win_util::load_library(original_module_path);
    logger::info("📚 Loaded original library from: '{}'", original_module_path.string());

    logger::info("🚀 Initialization complete");
}

void unlocker::shutdown() {
    win_util::free_library(original_module);

    logger::info("💀 Shutdown complete");
}

void unlocker::add_config_products(Map<ProductID, Product>& products) {
    products.insert({ app_id, Product(app_id, ProductType::App) });

    for (const auto& dlc_id: unlocker::config.dlcs) {
        products.insert({ dlc_id, Product(dlc_id, ProductType::DLC) });
    }

    for (const auto& item_id: unlocker::config.items) {
        products.insert({ item_id, Product(item_id, ProductType::Item) });
    }
}

void unlocker::add_fetched_products(Map<ProductID, Product>& products) {
    if (not unlocker::config.auto_fetch) {
        return;
    }

    const auto res = cpr::Get(cpr::Url{
        "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/ubisoft/v1/products.jsonc"
    });

    if (res.status_code == cpr::status::HTTP_OK) {
        try {
            const auto json = nlohmann::json::parse(res.text, nullptr, true, true);

            const auto game = json[std::to_string(app_id)];

            const String name(game["name"]);

            logger::info("Fetched products for game: '{}'", name);

            for (const ProductID dlc: game["dlcs"]) {
                logger::debug("  ID: {}, Type: DLC", dlc);

                products.insert({ dlc, Product(dlc, ProductType::DLC) });
            }

            for (const ProductID item: game["items"]) {
                logger::debug("  ID: {}, Type: Item", item);

                products.insert({ item, Product(item, ProductType::Item) });
            }
        } catch (const std::exception& ex) {
            logger::error("Failed to parse fetched products: {}", ex.what());
        }
    } else {
        logger::error(
            "Failed to fetch product IDs. "
            "Status code: {}. Text: {}", res.status_code, res.text
        );
    }
}

void unlocker::add_legit_products(Map<ProductID, Product>& products, const ProductList* legit_product_list) {
    logger::debug("Original product list contains {} elements:", legit_product_list->length);

    Vector<Product*> missing_products;

    // Iterate over original product list to find missing items

    for (uint32_t i = 0; i < legit_product_list->length; i++) {
        const auto product = legit_product_list->data[i];

        logger::debug(
            "  ID: {}, Type: {}, Mystery1: {}, Mystery2: {}, Always0: {}, Always3: {}",
            product->app_id, (int) product->type, product->mystery1,
            product->mystery2, product->always_0, product->always_3
        );

        if (product->type == ProductType::App) {
            continue;
        }

        // Insert missing products into the global product list
        if (not products.contains(product->app_id)) {
            products.insert({ product->app_id, Product(*product) });

            missing_products.push_back(product);
        }
    }

    // Let the user know which legitimately owned products are missing
    if (not missing_products.empty()) {
        logger::warn("Some of the legitimately owned products are missing from the config:");

        for (const auto missing_product: missing_products) {
            logger::warn(
                "  ID: {}, Type: {}",
                missing_product->app_id,
                missing_product->get_type_string()
            );
        }
    }
}

Vector<Product> unlocker::get_filtered_products(Map<ProductID, Product>& products) {
    Vector<Product> filtered_products;

    // Construct a filtered list for game that excludes blacklisted ids

    for (const auto&[id, product]: products) {
        const auto included = not unlocker::config.blacklist.contains(id);

        if (included) {
            filtered_products.push_back(product);
        }

        logger::info(
            "  {} ID: {}, Type: {}",
            included ? "✔" : "❌", id, product.get_type_string()
        );
    }

    return filtered_products;
}
