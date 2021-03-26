#include "pch.h"
#include "Config.h"

using nlohmann::json;

// Source: https://stackoverflow.com/a/54394658/3805929
#define GET(j, key) this->key = j[#key].get<decltype(key)>()

constexpr auto LEGACY_CONFIG_NAME = "UplayR2Unlocker.json";
constexpr auto CONFIG_NAME = "UplayR2Unlocker.jsonc";

Config::Config(HMODULE hModule)
{
	auto path = getDllDir(hModule) / CONFIG_NAME;
	std::ifstream ifs(path, std::ifstream::in);

	if(!ifs.good())
	{
		// Try the legacy config
		auto legacyPath = getDllDir(hModule) / LEGACY_CONFIG_NAME;
		ifs = std::ifstream(legacyPath, std::ifstream::in);
		if(!ifs.good())
		{ // No config found, therefore exit
			MessageBox(NULL, path.c_str(), L"Config not found at: ", MB_ICONERROR);
			exit(1);
		}
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
		MessageBoxA(NULL, e.what(), "Error parsing config file", MB_ICONERROR);
		exit(1);
	}
}


void Config::init(HMODULE hModule)
{
	if(config != nullptr)
		return;

	config = new Config(hModule);
}

Config* config = nullptr;
