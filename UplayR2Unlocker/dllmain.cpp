#include "pch.h"
#include "upc.h"

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	if(ul_reason_for_call == DLL_PROCESS_ATTACH)
		UPC::init();
	else if(ul_reason_for_call == DLL_PROCESS_DETACH)
		UPC::shutdown();

	return TRUE;
}
