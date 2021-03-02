#pragma once

namespace UPC
{

struct Product
{
	Product(uint32_t appid, uint32_t type)
	{
		this->appid = appid;
		this->type = type;
		this->mystery = type == 4 ? 4 : 1;
	}

	uint32_t appid;
	uint32_t type; //1 = app, 2 = dlc, 4 = ???
	uint32_t mystery;
	uint32_t always_3 = 3; // always 3
	uint32_t always_0 = 0; // always zero
	uint32_t always_1 = 1; // always 1
};

struct ProductList
{
	uint32_t length = 0;
	uint32_t padding = 0; // What is this?
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
