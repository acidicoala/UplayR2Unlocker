#include "pch.h"
#include "upc.h"
#include "Logger.h"
#include "Config.h"

#define EXPORT extern "C" _declspec(dllexport)

#define GET_PROXY_FUNC(FUNC) \
	static auto proxyFunc = FnCast(GetProcAddress(originalDLL, #FUNC), FUNC );

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

EXPORT int UPC_Init(unsigned version, int appID)
{
	logger->info("{} -> version: {}, appid: {}", __func__, version, appID);

	products.push_back(Product(appID, 1));
	for(auto& dlc : config.dlcs)
	{
		products.push_back(Product(dlc, 2));
	}

	for(auto& item : config.items)
	{
		products.push_back(Product(item, 4));
	}

	GET_PROXY_FUNC(UPC_Init);

	return proxyFunc(version, appID);
}


void ProductListGetCallback(unsigned long arg1, void* data)
{
	logger->debug("{} -> arg1: {}, data: {}", arg1, data);

	auto callbackContainer = (CallbackContainer*) data;

	auto list = callbackContainer->legitProductList;
	for(uint32_t i = 0; i < list->length; i++)
	{
		auto product = *list->data[i];

		logger->debug(
			"App ID: {}, Type: {}, Mystery: {}, Always 0: {}, Always 1: {}, Always 3: {}",
			product.appid, product.type, product.mystery, product.always_0, product.always_1, product.always_3
		);
	}
	// TODO: delete product list. Tiny memory leak, no big deal.

	callbackContainer->originalCallback(arg1, callbackContainer->callbackData);

	delete callbackContainer;
}

EXPORT int UPC_ProductListGet(void* context, char* inOptUserIdUtf8, unsigned int inFilter, ProductList** outProductList, UplayCallback inCallback, void* inCallbackData) //CB: 1 argument, 0 val
{
	logger->debug("{} -> userID: {}", __func__, inOptUserIdUtf8);

	auto productList = new ProductList();
	productList->data = new Product * [products.size()];
	for(int i = 0; i < products.size(); i++)
	{
		productList->data[i] = new Product(products.at(i));
	}

	productList->length = products.size();
	*outProductList = productList;

	auto callbackContainer = new CallbackContainer{
		inCallback,
		inCallbackData
	};

	GET_PROXY_FUNC(UPC_ProductListGet);

	return proxyFunc(context, inOptUserIdUtf8, inFilter, &callbackContainer->legitProductList, ProductListGetCallback, callbackContainer);
	return 1 << 4;
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
		return proxyFunc(context);
	}
	else
	{
		return config.lang.c_str();
	}
}
