#pragma once

namespace UPC
{


enum class ProductType
{
	App = 1,
	DLC = 2,
	Item = 4
};

struct Product
{
	Product(uint32_t appid, ProductType type)
	{
		this->appid = appid;
		this->type = type;
		this->mystery1 = type == ProductType::Item ? 4 : 1;
		this->mystery2 = type == ProductType::Item ? 1 : 3;
	}

	uint32_t appid;
	ProductType type;
	uint32_t mystery1;
	uint32_t always_3 = 3; // always 3
	uint32_t always_0 = 0; // always zero
	uint32_t mystery2;
};

struct ProductList
{
	uint32_t length = 0;
	uint32_t padding = 0; // What is this? offset?
	Product** data = NULL; // Array of pointers
};

typedef void (*UplayCallback)(unsigned long, void*);


struct CallbackContainer
{
	UplayCallback originalCallback = NULL;
	void* callbackData = NULL;
	ProductList* legitProductList = NULL;
};

void init();
void shutdown();

}
