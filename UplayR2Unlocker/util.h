#pragma once
#include "framework.h"

using std::string;
using std::wstring;
using std::vector;
using std::shared_ptr;
using std::filesystem::absolute;
using std::filesystem::path;

constexpr auto VERSION = "1.1.0";

// Source: Polyhook 2
template<typename T>
T FnCast(FARPROC fnToCast, T pFnCastTo)
{
	return (T) fnToCast;
}

bool vectorContains(vector<int> numbers, uint32_t number);

path getDllDir(HMODULE hModule);
