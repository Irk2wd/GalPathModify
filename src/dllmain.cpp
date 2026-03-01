#include "proxy.h"
#include "hook.h"
#include <windows.h>

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        DisableThreadLibraryCalls(hModule);
        Proxy_Init();
        Hook_Init();
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        Hook_Free();
        Proxy_Free();
    }
    return TRUE;
}