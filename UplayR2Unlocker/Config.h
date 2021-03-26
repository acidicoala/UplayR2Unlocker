#pragma once
#include "util.h"

class Config
{
protected:
	Config(HMODULE hModule);
public:
	string log_level;
	string lang;
	vector<int> dlcs;
	vector<int> items;

	static void init(HMODULE hModule);

};

extern Config* config;
