#include "upc.hpp"

#include "koalabox/util/util.hpp"

String Product::get_type_string() const {
    switch (type) {
        case ProductType::App:
            return "App";
        case ProductType::DLC:
            return "DLC";
        case ProductType::Item:
            return "Item";
        default:
            return "Unexpected Type";
    }
}

void add_config_products(Map<ProductID, Product>& products) {
    products.insert({ global_app_id, Product(global_app_id, ProductType::App) });

    for (const auto& dlc_id: config.dlcs) {
        products.insert({ dlc_id, Product(dlc_id, ProductType::DLC) });
    }

    for (const auto& item_id: config.items) {
        products.insert({ item_id, Product(item_id, ProductType::Item) });
    }
}

void add_fetched_products(Map<ProductID, Product>& products) {
    if (not config.auto_fetch) {
        return;
    }

    const auto res = cpr::Get(cpr::Url{
        "https://raw.githubusercontent.com/acidicoala/public-entitlements/main/ubisoft/v1/products.jsonc"
    });

    if (res.status_code == cpr::status::HTTP_OK) {
        try {
            const auto json = nlohmann::json::parse(res.text, nullptr, true, true);

            // iterate over keys to find the one that matches the game

            for (const auto&[key, game]: json.items()) {
                // Regex matching enables us to specify multiple IDs for the same game
                if (std::regex_match(std::to_string(global_app_id), std::regex(key))) {
                    const String name(game["name"]);

                    logger->info("Fetched products for game: '{}'", name);

                    for (const ProductID dlc: game["dlcs"]) {
                        logger->debug("  ID: {}, Type: DLC", dlc);

                        products.insert({ dlc, Product(dlc, ProductType::DLC) });
                    }

                    for (const ProductID item: game["items"]) {
                        logger->debug("  ID: {}, Type: Item", item);

                        products.insert({ item, Product(item, ProductType::Item) });
                    }

                    break;
                }
            }
        } catch (const std::exception& ex) {
            logger->error("Failed to parse fetched products: {}", ex.what());
        }
    } else {
        logger->error(
            "Failed to fetch product IDs. "
            "Status code: {}. Text: {}", res.status_code, res.text
        );
    }
}

void add_legit_products(Map<ProductID, Product>& products, const ProductList* legit_product_list) {
    logger->debug("Original product list contains {} elements:", legit_product_list->length);

    Vector<Product*> missing_products;

    // Iterate over original product list to find missing items

    for (uint32_t i = 0; i < legit_product_list->length; i++) {
        const auto product = legit_product_list->data[i];

        logger->debug(
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
        logger->warn("Some of the legitimately owned products are missing from the config:");

        for (const auto missing_product: missing_products) {
            logger->warn(
                "  ID: {}, Type: {}",
                missing_product->app_id,
                missing_product->get_type_string()
            );
        }
    }
}

Vector<Product> get_filtered_products(Map<ProductID, Product>& products) {
    Vector<Product> filtered_products;

    // Construct a filtered list for game that excludes blacklisted ids

    for (const auto&[id, product]: products) {
        const auto included = not config.blacklist.contains(id);

        if (included) {
            filtered_products.push_back(product);
        }

        logger->info(
            "  {} ID: {}, Type: {}",
            included ? "✅" : "❌", id, product.get_type_string()
        );
    }

    return filtered_products;
}

ProductList* create_new_product_list(const Vector<Product>& filtered_products) {
    const auto product_list = new ProductList{
        .length = static_cast<uint32_t>(filtered_products.size()),
        .data = new Product* [filtered_products.size()]
    };

    for (size_t i = 0; i < filtered_products.size(); i++) {
        product_list->data[i] = new Product(filtered_products.at(i));
    }

    return product_list;
}

DLL_EXPORT(int) UPC_Init(unsigned int version, ProductID app_id) {
    logger->info("⚡ {} -> version: {}, app_id: {}", __func__, version, app_id);

    global_app_id = app_id;

    GET_ORIGINAL_FUNCTION(UPC_Init)

    const auto result = UPC_Init_o(version, app_id);

    logger->debug("{} result: {}", __func__, result);

    // Store R2 DLL is loaded at this point, so we can hook its functions now
    if (is_hook_mode) {
        post_init();
    }

    return result;
}

DLL_EXPORT(int) UPC_ProductListFree(void*, ProductList* inProductList) {
    logger->debug(__func__);

    if (inProductList) {
        for (uint32_t i = 0; i < inProductList->length; i++) {
            delete inProductList->data[i];
        }

        delete[] inProductList->data;
    }

    delete inProductList;

    return 0;
}

DLL_EXPORT(int) UPC_ProductListGet(
    void* context,
    const char* inOptUserIdUtf8,
    unsigned int inFilter,
    ProductList** outProductList,
    UplayCallback inCallback,
    void* inCallbackData
) {
    logger->debug(__func__);

    const auto callbackContainer = new CallbackContainer{
        .context = context,
        .original_callback = inCallback,
        .in_callback_data = inCallbackData,
        .out_product_list = outProductList,
    };

    static const auto callback = [](unsigned long arg1, void* data) {
        logger->debug("UPC_ProductListGet callback -> arg1: {}", arg1);

        const auto container = static_cast<CallbackContainer*>(data);

        Map<ProductID, Product> products;

        add_config_products(products);
        add_fetched_products(products);
        add_legit_products(products, container->legit_product_list);

        logger->info("🍀 Unlocker prepared {} products:", products.size());

        const auto filtered_products = get_filtered_products(products);

        const auto product_list = create_new_product_list(filtered_products);

        // Save the product list address in the pointer provided by game earlier
        *(container->out_product_list) = product_list;

        // Let the game know that the product list is ready
        container->original_callback(arg1, container->in_callback_data);

        // Free the legit product list
        GET_ORIGINAL_FUNCTION(UPC_ProductListFree)
        UPC_ProductListFree_o(container->context, container->legit_product_list);

        delete container;
    };

    GET_ORIGINAL_FUNCTION(UPC_ProductListGet)

    return UPC_ProductListGet_o(
        context,
        inOptUserIdUtf8,
        inFilter,
        &callbackContainer->legit_product_list,
        callback,
        callbackContainer
    );
}

DLL_EXPORT(const char*) UPC_InstallLanguageGet(void* context) {
    logger->debug(__func__);

    if (config.lang == "default") {
        GET_ORIGINAL_FUNCTION(UPC_InstallLanguageGet)
        const auto result = UPC_InstallLanguageGet_o(context);

        logger->info("🔤 Responding with original language -> '{}'", result);

        return result;
    } else {
        logger->info("🔤 Responding with configured language -> '{}'", config.lang);

        return config.lang.c_str();
    }
}
