#pragma once

#include "unlocker/unlocker.hpp"

#define DLL_EXPORT(TYPE) extern "C" _declspec(dllexport) TYPE

using namespace unlocker;

enum class ProductType {
    App = 1,
    DLC = 2,
    Item = 4,
};

struct Product {
    uint32_t app_id;

    ProductType type;
    [[maybe_unused]] uint32_t mystery1;
    [[maybe_unused]] uint32_t always_3 = 3;
    [[maybe_unused]] uint32_t always_0 = 0;
    [[maybe_unused]] uint32_t mystery2;

    Product(uint32_t app_id, ProductType type) {
        this->app_id = app_id;
        this->type = type;
        this->mystery1 = type == ProductType::Item ? 4 : 1;
        this->mystery2 = type == ProductType::Item ? 1 : 3;
    }

    [[nodiscard]]
    String get_type_string() const;
};

struct ProductList {
    uint32_t length = 0;
    [[maybe_unused]] uint32_t padding = 0; // What is this? offset?
    Product** data = nullptr; // Array of pointers
};

typedef void (* UplayCallback)(unsigned long, void*);

struct CallbackContainer {
    void* context = nullptr;
    UplayCallback original_callback = nullptr;
    void* in_callback_data = nullptr;
    ProductList* legit_product_list = nullptr;
    ProductList** out_product_list = nullptr;
};

DLL_EXPORT(int) UPC_Init(unsigned int version, ProductID app_id);

DLL_EXPORT(const char*) UPC_InstallLanguageGet(void* context);

DLL_EXPORT(int) UPC_ProductListFree(void* context, ProductList* inProductList);

DLL_EXPORT(int) UPC_ProductListGet(
    void* context,
    const char* inOptUserIdUtf8,
    unsigned int inFilter,
    ProductList** outProductList,
    UplayCallback inCallback,
    void* inCallbackData
);