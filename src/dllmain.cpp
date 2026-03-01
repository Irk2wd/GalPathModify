#include <Windows.h>

// 前面定义的函数
extern bool Proxy_Init();
extern void Proxy_Free();
extern bool Hook_Init();
extern void Hook_Free();

static DWORD WINAPI HookThread(LPVOID)
{
    // 等待 shell32.dll 被加载
    for (int i = 0; i < 50; i++)
    {
        if (GetModuleHandleW(L"shell32.dll"))
            break;
        Sleep(100);
    }
    Hook_Init();
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
    switch (reason)
    {
    case DLL_PROCESS_ATTACH:
        DisableThreadLibraryCalls(hModule);

        if (!Proxy_Init())
            return FALSE;

        CreateThread(NULL, 0, HookThread, NULL, 0, NULL);
        break;

    case DLL_PROCESS_DETACH:
        Hook_Free();
        Proxy_Free();
        break;
    }
    return TRUE;
}