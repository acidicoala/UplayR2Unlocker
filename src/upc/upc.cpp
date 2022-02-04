#include "upc.hpp"
#include "koalabox/logger/logger.hpp"
#include "koalabox/util/util.hpp"
#include "unlocker/unlocker.hpp"

#define DLL_EXPORT(TYPE) extern "C" _declspec(dllexport) TYPE

#define GET_ORIG_FUNC(FUNC) koalabox::util::get_procedure(unlocker::original_module, #FUNC, FUNC)

using namespace upc;
using namespace koalabox;
using namespace config;

DLL_EXPORT(int) UPC_Init(unsigned int version, ProductID app_id) {
    logger::info("⚡ {} -> version: {}, app_id: {}", __func__, version, app_id);

    unlocker::app_id = app_id;

    static const auto UPC_Init_o = GET_ORIG_FUNC(UPC_Init);

    return UPC_Init_o(version, app_id);
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

DLL_EXPORT(int) UPC_ProductListFree([[maybe_unused]] void* context, ProductList* inProductList) {
    logger::debug(__func__);

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
    logger::debug(__func__);

    const auto callbackContainer = new CallbackContainer{
        .context = context,
        .original_callback = inCallback,
        .in_callback_data = inCallbackData,
        .out_product_list = outProductList,
    };

    static const auto callback = [](unsigned long arg1, void* data) {
        logger::debug("UPC_ProductListGet callback -> arg1: {}", arg1);

        const auto container = static_cast<CallbackContainer*>(data);

        Map<ProductID, Product> products;

        unlocker::add_config_products(products);
        unlocker::add_fetched_products(products);
        unlocker::add_legit_products(products, container->legit_product_list);

        logger::info("🍀 Unlocker prepared {} products:", products.size());

        const auto filtered_products = unlocker::get_filtered_products(products);

        const auto product_list = create_new_product_list(filtered_products);

        // Save the product list address in the pointer provided by game earlier
        *(container->out_product_list) = product_list;

        // Let the game know that the product list is ready
        container->original_callback(arg1, container->in_callback_data);

        // Free the legit product list
        static const auto UPC_ProductListFree_o = GET_ORIG_FUNC(UPC_ProductListFree);
        UPC_ProductListFree_o(container->context, container->legit_product_list);

        delete container;
    };

    static const auto UPC_ProductListGet_o = GET_ORIG_FUNC(UPC_ProductListGet);

    return UPC_ProductListGet_o(
        context,
        inOptUserIdUtf8,
        inFilter,
        &callbackContainer->legit_product_list,
        callback,
        callbackContainer
    );
}

DLL_EXPORT(LPCSTR) UPC_InstallLanguageGet(void* context) {
    logger::debug(__func__);

    if (unlocker::config.lang == "default") {
        static const auto UPC_InstallLanguageGet_o = GET_ORIG_FUNC(UPC_InstallLanguageGet);
        const auto result = UPC_InstallLanguageGet_o(context);

        logger::info("🔤 Responding with original language -> '{}'", result);

        return result;
    } else {
        logger::info("🔤 Responding with configured language -> '{}'", unlocker::config.lang);

        return unlocker::config.lang.c_str();
    }
}

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
