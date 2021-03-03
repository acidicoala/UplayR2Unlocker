#include "pch.h"
#include "Config.h"

using nlohmann::json;

// Source: https://stackoverflow.com/a/54394658/3805929
#define GET(j, key) this->key = j[#key].get<decltype(key)>()

constexpr auto CONFIG_PATH = "UplayR2Unlocker.json";

Config::Config()
{
	std::ifstream ifs(CONFIG_PATH, std::ifstream::in);

	if(!ifs.good())
	{
		auto path = std::filesystem::relative(CONFIG_PATH).string();
		MessageBoxA(NULL, path.c_str(), "Config not found at: ", MB_ICONERROR | MB_ICONERROR);
		exit(1);
	}

	try
	{
		auto j = json::parse(ifs, nullptr, true, true);

		GET(j, log_level);
		GET(j, lang);
		GET(j, dlcs);
		GET(j, items);
	} catch(json::exception e)
	{
		MessageBoxA(NULL, e.what(), "Error parsing config file", MB_ICONERROR | MB_ICONERROR);
		exit(1);
	}
}

Config config;
