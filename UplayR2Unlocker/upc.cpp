#include "pch.h"
#include "upc.h"
#include "Logger.h"
#include "Config.h"

#define EXPORT extern "C" _declspec(dllexport)

#define GET_PROXY_FUNC(FUNC) \
	static auto proxyFunc = FnCast(GetProcAddress(originalDLL, #FUNC), FUNC)

using namespace UPC;

#ifdef _WIN64
constexpr auto ORIG_DLL = L"upc_r2_loader64_o.dll";
#else
constexpr auto ORIG_DLL = L"upc_r2_loader_o.dll";
#endif

HMODULE originalDLL = nullptr;
vector<Product> products;

void UPC::init(HMODULE hModule) {
	Config::init(hModule);
	Logger::init(hModule);
	logger->info("Uplay R2 Unlocker v{}", VERSION);

	originalDLL = LoadLibrary(ORIG_DLL);
	if (originalDLL) {
		logger->info(L"Successfully loaded original DLL: {}", ORIG_DLL);
	} else {
		logger->error(L"Failed to load original DLL: {}. Error code: {}", ORIG_DLL, GetLastError());
		exit(1);
	}
}

void UPC::shutdown() {
	logger->info("Shutting down");
	FreeLibrary(originalDLL);
}

string productTypeToString(ProductType type) {
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

EXPORT int UPC_Init(unsigned version, int appID) {
	logger->info("{} -> version: {}, appid: {}", __func__, version, appID);

	products.emplace_back(Product(appID, ProductType::App));
	for (const auto& dlc : config->dlcs) {
		products.emplace_back(Product(dlc, ProductType::DLC));
	}

	for (const auto& item : config->items) {
		products.emplace_back(Product(item, ProductType::Item));
	}

	GET_PROXY_FUNC(UPC_Init);

	return proxyFunc(version, appID);
}

EXPORT int UPC_ProductListFree(void* context, ProductList* inProductList) {
	logger->debug(__func__);
	if (inProductList) {
		for (unsigned i = 0; i < inProductList->length; ++i) {
			delete inProductList->data[i];
		}

		delete[] inProductList->data;
	}

	delete inProductList;
	return 0;
}

void ProductListGetCallback(unsigned long arg1, void* data) {
	logger->debug("{} -> arg1: {}, data: {}", arg1, data);

	const auto callbackContainer = (CallbackContainer*)data;

	logger->debug("Legit product list:");

	vector<Product*> missingProducts;
	const auto list = callbackContainer->legitProductList;
	for (uint32_t i = 0; i < list->length; i++) {
		auto product = list->data[i];

		logger->debug(
			"\tApp ID: {}, Type: {}, Mystery1: {}, Mystery2: {}, Always0: {}, Always3: {}",
			product->appid, product->type, product->mystery1, product->mystery2, product->always_0, product->always_3
		);

		if (!(vectorContains(config->dlcs, product->appid) || vectorContains(config->items, product->appid)))
			if (product->type != ProductType::App)
				missingProducts.push_back(product);
	}

	if (!missingProducts.empty())
		logger->warn("Some of the legitimately owned products are missing from the config: ");

	for (const auto& missingProduct : missingProducts) {
		logger->warn("\tApp ID: {}, Type: {}", missingProduct->appid, productTypeToString(missingProduct->type));
	}

	// Free the legit product list
	GET_PROXY_FUNC(UPC_ProductListFree);
	proxyFunc(callbackContainer->context, callbackContainer->legitProductList);

	callbackContainer->originalCallback(arg1, callbackContainer->callbackData);

	delete callbackContainer;
}

EXPORT int UPC_ProductListGet(void* context, char* inOptUserIdUtf8, unsigned int inFilter, ProductList** outProductList, UplayCallback inCallback, void* inCallbackData) //CB: 1 argument, 0 val
{
	logger->debug("{}", __func__);

	const auto productList = new ProductList();
	productList->data = new Product*[products.size()];
	for (size_t i = 0; i < products.size(); i++) {
		productList->data[i] = new Product(products.at(i));
	}

	productList->length = (uint32_t)products.size();
	*outProductList = productList;

	const auto callbackContainer = new CallbackContainer{
		context,
		inCallback,
		inCallbackData
	};

	GET_PROXY_FUNC(UPC_ProductListGet);
	return proxyFunc(context, inOptUserIdUtf8, inFilter, &callbackContainer->legitProductList, ProductListGetCallback, callbackContainer);
}

EXPORT const char* UPC_InstallLanguageGet(void* context) {
	if (config->lang == "default") {
		GET_PROXY_FUNC(UPC_InstallLanguageGet);
		const auto result = proxyFunc(context);
		logger->debug("UPC_InstallLanguageGet -> {}", result);
		return result;
	} else {
		return config->lang.c_str();
	}
}
