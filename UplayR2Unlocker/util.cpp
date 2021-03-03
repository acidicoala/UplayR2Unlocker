#include "pch.h"
#include "util.h"

wstring getModulePath(HMODULE hModule)
{
	WCHAR buffer[MAX_PATH];
	GetModuleFileName(hModule, buffer, MAX_PATH);
	return wstring(buffer);
}

bool vectorContains(vector<int> numbers, uint32_t number)
{
	for(const auto& num : numbers)
	{
		if(num == number)
			return true;
	}
	return false;
}