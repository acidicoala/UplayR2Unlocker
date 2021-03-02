#pragma once
#include "util.h"

class Config
{
public:
	Config();

	string log_level;
	string lang;
	vector<int> dlcs;
	vector<int> items;
};

extern Config config;
