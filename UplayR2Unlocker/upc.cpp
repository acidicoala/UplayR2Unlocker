#include "pch.h"
#include "upc.h"
#include "Logger.h"
#include "Config.h"

#define EXPORT extern "C" _declspec(dllexport)

#define GET_PROXY_FUNC(FUNC) \
	static auto proxyFunc = FnCast(GetProcAddress(originalDLL, #FUNC), FUNC);

using namespace UPC;

constexpr auto ORIG_DLL = L"uplay_r2_loader64_o.dll";

HMODULE originalDLL = nullptr;
vector<Product> products;

void UPC::init()
{
	Logger::init();
	logger->info("Uplay R2 Unlocker v{}", VERSION);

	originalDLL = LoadLibrary(ORIG_DLL);
	if(originalDLL)
	{
		logger->info(L"Successfully loaded original DLL: {}", ORIG_DLL);
	}
	else
	{
		logger->error(L"Failed to load original DLL: {}. Error code: {}", ORIG_DLL, GetLastError());
		exit(1);
	}
}

void UPC::shutdown()
{
	logger->info("Shutting down");
	FreeLibrary(originalDLL);
}

string productTypeToString(ProductType type)
{
	switch(type)
	{
		case UPC::ProductType::App:
			return "App";
		case UPC::ProductType::DLC:
			return "DLC";
		case UPC::ProductType::Item:
			return "Item";
		default:
			return "Unexpected Type";
	}
}

EXPORT int UPC_Init(unsigned version, int appID)
{
	logger->info("{} -> version: {}, appid: {}", __func__, version, appID);

	products.push_back(Product(appID, ProductType::App));
	for(auto& dlc : config.dlcs)
	{
		products.push_back(Product(dlc, ProductType::DLC));
	}

	for(auto& item : config.items)
	{
		products.push_back(Product(item, ProductType::Item));
	}

	GET_PROXY_FUNC(UPC_Init);

	return proxyFunc(version, appID);
}

void ProductListGetCallback(unsigned long arg1, void* data)
{
	logger->debug("{} -> arg1: {}, data: {}", arg1, data);

	auto callbackContainer = (CallbackContainer*) data;

	logger->debug("Legit product list:");

	vector<Product*> missingProducts;
	auto list = callbackContainer->legitProductList;
	for(uint32_t i = 0; i < list->length; i++)
	{
		auto product = list->data[i];

		logger->debug(
			"\tApp ID: {}, Type: {}, Myster 1: {}, Mystery 2: {}, Always 0: {}, Always 3: {}",
			product->appid, product->type, product->mystery1, product->mystery2, product->always_0, product->always_3
		);

		if(!(vectorContains(config.dlcs, product->appid) || vectorContains(config.items, product->appid)))
			if(product->type != ProductType::App)
				missingProducts.push_back(product);
	}

	if(missingProducts.size() != 0)
		logger->warn("Some of the legitimately owned products are missing from the config: ");

	for(const auto& missingProduct : missingProducts)
	{
		logger->warn("\tApp ID: {}, Type: {}", missingProduct->appid, productTypeToString(missingProduct->type));
	}

	// TODO: delete product list. Tiny memory leak, no big deal.

	callbackContainer->originalCallback(arg1, callbackContainer->callbackData);

	delete callbackContainer;
}

EXPORT int UPC_ProductListGet(void* context, char* inOptUserIdUtf8, unsigned int inFilter, ProductList** outProductList, UplayCallback inCallback, void* inCallbackData) //CB: 1 argument, 0 val
{
	logger->debug("{}", __func__);
	logger->debug(L"\tuserID: {}", wstring((LPWCH) inOptUserIdUtf8));

	auto productList = new ProductList();
	productList->data = new Product * [products.size()];
	for(int i = 0; i < products.size(); i++)
	{
		productList->data[i] = new Product(products.at(i));
	}

	productList->length = (uint32_t) products.size();
	*outProductList = productList;

	auto callbackContainer = new CallbackContainer{
		inCallback,
		inCallbackData
	};

	GET_PROXY_FUNC(UPC_ProductListGet);
	return proxyFunc(context, inOptUserIdUtf8, inFilter, &callbackContainer->legitProductList, ProductListGetCallback, callbackContainer);
	//return 1 << 4;
}

EXPORT int UPC_ProductListFree(void* context, ProductList* inProductList)
{
	logger->debug(__func__);
	if(inProductList)
	{
		for(unsigned i = 0; i < inProductList->length; ++i)
		{
			delete inProductList->data[i];
		}

		delete[] inProductList->data;
	}

	delete inProductList;
	return 0;
}

EXPORT const char* UPC_InstallLanguageGet(void* context)
{
	if(config.lang == "default")
	{
		GET_PROXY_FUNC(UPC_InstallLanguageGet);
		auto result = proxyFunc(context);
		logger->debug("UPC_InstallLanguageGet -> {}", result);
		return result;
	}
	else
	{
		return config.lang.c_str();
	}
}
