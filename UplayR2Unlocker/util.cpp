#include "pch.h"
#include "util.h"
#include "Logger.h"

bool vectorContains(vector<int> numbers, uint32_t number)
{
	for(const auto& num : numbers)
	{
		if(num == number)
			return true;
	}
	return false;
}

path getDllDir(HMODULE hModule)
{
	TCHAR name[MAX_PATH];
	auto result = GetModuleFileName(hModule, name, MAX_PATH);

	if(result == NULL)
		logger->error("Failed to get dll path. Error code: {}", GetLastError());

	return path(name).parent_path();
}
