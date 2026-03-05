#include "proxy.h"
#include "hook.h"
#include <windows.h>
#include <cstring>
#include <cstdio>

#ifndef GALPATHMODIFY_ENABLE_LOG
#define GALPATHMODIFY_ENABLE_LOG 0
#endif

static HANDLE g_hookThread = NULL;
static volatile LONG g_stopHookThread = 0;

static void LogDllEvent(const char *msg)
{
#if GALPATHMODIFY_ENABLE_LOG
    char exePath[MAX_PATH] = {0};
    char logPath[MAX_PATH] = {0};
    DWORD len = GetModuleFileNameA(NULL, exePath, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
        return;

    char *slash = strrchr(exePath, '\\');
    if (!slash)
        return;
    *(slash + 1) = '\0';
    lstrcpynA(logPath, exePath, MAX_PATH);
    if (lstrlenA(logPath) + 16 >= MAX_PATH)
        return;
    lstrcatA(logPath, "galpathmodify.log");

    FILE *fp = fopen(logPath, "a");
    if (!fp)
        return;
    fprintf(fp, "%s\n", msg);
    fclose(fp);
#else
    (void)msg;
#endif
}

static DWORD WINAPI HookInitThreadProc(LPVOID)
{
    for (int i = 0; i < 200 && InterlockedCompareExchange(&g_stopHookThread, 0, 0) == 0; ++i)
    {
        if (Hook_Init())
        {
            LogDllEvent("HookInitThreadProc: Hook_Init succeeded.");
            return 0;
        }
        Sleep(50);
    }
    LogDllEvent("HookInitThreadProc: Hook_Init timed out.");
    return 0;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD reason, LPVOID reserved)
{
    if (reason == DLL_PROCESS_ATTACH)
    {
        LogDllEvent("DLL_PROCESS_ATTACH");
        DisableThreadLibraryCalls(hModule);
        InterlockedExchange(&g_stopHookThread, 0);
        Proxy_Init();
        g_hookThread = CreateThread(NULL, 0, HookInitThreadProc, NULL, 0, NULL);
        if (g_hookThread)
        {
            CloseHandle(g_hookThread);
            g_hookThread = NULL;
        }
    }
    else if (reason == DLL_PROCESS_DETACH)
    {
        LogDllEvent("DLL_PROCESS_DETACH");
        InterlockedExchange(&g_stopHookThread, 1);
        Hook_Free();
        Proxy_Free();
    }
    return TRUE;
}
