#include <windows.h>
#include <shlobj.h>
#include <cstring>
#include <cstdio>
#include <cstdarg>
#include <MinHook.h>
#include "hook.h"

#ifndef GALPATHMODIFY_ENABLE_LOG
#define GALPATHMODIFY_ENABLE_LOG 0
#endif

static wchar_t g_savePathW[MAX_PATH] = {0};
static char g_savePathA[MAX_PATH] = {0};
static char g_logPathA[MAX_PATH] = {0};
static LONG g_logCallCount = 0;

typedef BOOL(WINAPI *SHGetPathFromIDListA_t)(LPCITEMIDLIST, LPSTR);
typedef BOOL(WINAPI *SHGetPathFromIDListW_t)(LPCITEMIDLIST, LPWSTR);
static SHGetPathFromIDListA_t pOrigGetPathA = nullptr;
static SHGetPathFromIDListW_t pOrigGetPathW = nullptr;

static void Log(const char *fmt, ...)
{
#if GALPATHMODIFY_ENABLE_LOG
    if (!g_logPathA[0])
        return;

    FILE *fp = fopen(g_logPathA, "a");
    if (!fp)
        return;

    va_list args;
    va_start(args, fmt);
    vfprintf(fp, fmt, args);
    va_end(args);
    fputc('\n', fp);
    fclose(fp);
#else
    (void)fmt;
#endif
}

BOOL WINAPI HookedGetPathFromIDListA(LPCITEMIDLIST pidl, LPSTR pszPath)
{
    BOOL ret = pOrigGetPathA ? pOrigGetPathA(pidl, pszPath) : FALSE;
    if (ret && pszPath)
    {
        lstrcpynA(pszPath, g_savePathA, MAX_PATH);
        const bool redirected = (lstrcmpiA(pszPath, g_savePathA) == 0);
        LONG n = InterlockedIncrement(&g_logCallCount);
        if (n <= 20)
            Log("[A] redirected=%d path=%s", redirected ? 1 : 0, pszPath);
    }
    return ret;
}

BOOL WINAPI HookedGetPathFromIDListW(LPCITEMIDLIST pidl, LPWSTR pszPath)
{
    BOOL ret = pOrigGetPathW ? pOrigGetPathW(pidl, pszPath) : FALSE;
    if (ret && pszPath)
    {
        lstrcpynW(pszPath, g_savePathW, MAX_PATH);
        const bool redirected = (lstrcmpiW(pszPath, g_savePathW) == 0);
        LONG n = InterlockedIncrement(&g_logCallCount);
        if (n <= 20)
            Log("[W] redirected=%d", redirected ? 1 : 0);
    }
    return ret;
}

static bool BuildPaths()
{
    DWORD len = GetModuleFileNameW(NULL, g_savePathW, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
        return false;

    wchar_t *lastSlash = wcsrchr(g_savePathW, L'\\');
    if (!lastSlash)
        return false;
    *(lastSlash + 1) = L'\0';

    if (lstrlenW(g_savePathW) + 4 >= MAX_PATH)
        return false;
    lstrcatW(g_savePathW, L"save");
    CreateDirectoryW(g_savePathW, NULL);

    if (!WideCharToMultiByte(CP_ACP, 0, g_savePathW, -1, g_savePathA, MAX_PATH, NULL, NULL))
        return false;

    len = GetModuleFileNameA(NULL, g_logPathA, MAX_PATH);
    if (len == 0 || len >= MAX_PATH)
        return false;

    char *slash = strrchr(g_logPathA, '\\');
    if (!slash)
        return false;
    *(slash + 1) = '\0';
    if (lstrlenA(g_logPathA) + 16 >= MAX_PATH)
        return false;
    lstrcatA(g_logPathA, "galpathmodify.log");

    return true;
}

bool Hook_Init()
{
    if (!BuildPaths())
        return false;

    Log("Hook_Init start. savePath=%s", g_savePathA);
    Log("Redirect target confirmed: %s", g_savePathA);

    MH_STATUS st = MH_Initialize();
    const bool initializedNow = (st == MH_OK);
    if (st != MH_OK && st != MH_ERROR_ALREADY_INITIALIZED)
    {
        Log("MH_Initialize failed: %d", (int)st);
        return false;
    }

    st = MH_CreateHookApi(L"shell32", "SHGetPathFromIDListA",
                          (void *)HookedGetPathFromIDListA, (void **)&pOrigGetPathA);
    if (st != MH_OK && st != MH_ERROR_ALREADY_CREATED)
    {
        Log("MH_CreateHookApi A failed: %d", (int)st);
        if (initializedNow)
            MH_Uninitialize();
        return false;
    }
    Log("MH_CreateHookApi A ok: %d", (int)st);

    st = MH_CreateHookApi(L"shell32", "SHGetPathFromIDListW",
                          (void *)HookedGetPathFromIDListW, (void **)&pOrigGetPathW);
    if (st != MH_OK && st != MH_ERROR_ALREADY_CREATED)
    {
        Log("MH_CreateHookApi W failed: %d", (int)st);
        if (initializedNow)
            MH_Uninitialize();
        return false;
    }
    Log("MH_CreateHookApi W ok: %d", (int)st);

    st = MH_EnableHook(MH_ALL_HOOKS);
    if (st != MH_OK && st != MH_ERROR_ENABLED)
    {
        Log("MH_EnableHook failed: %d", (int)st);
        if (initializedNow)
            MH_Uninitialize();
        return false;
    }
    Log("MH_EnableHook ok: %d", (int)st);

    return true;
}

void Hook_Free()
{
    MH_STATUS st = MH_DisableHook(MH_ALL_HOOKS);
    Log("MH_DisableHook: %d", (int)st);
    st = MH_Uninitialize();
    Log("MH_Uninitialize: %d", (int)st);
}
