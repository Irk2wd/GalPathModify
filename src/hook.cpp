#include <windows.h>
#include <shlobj.h>
#include <cstring>
#include <MinHook.h>
#include "hook.h"

static wchar_t g_savePath[MAX_PATH] = {0};
static char g_savePathA[MAX_PATH] = {0};

typedef BOOL(WINAPI *SHGetPathFromIDListA_t)(LPCITEMIDLIST, LPSTR);
static SHGetPathFromIDListA_t pOrigGetPathA = nullptr;

BOOL WINAPI HookedGetPathFromIDListA(LPCITEMIDLIST pidl, LPSTR pszPath)
{
    BOOL ret = pOrigGetPathA(pidl, pszPath);
    if (ret)
    {
        // 原始返回的是 "C:\Users\xxx\Documents"
        // 只替换 Documents 路径，不影响其他调用
        if (strstr(pszPath, "Documents"))
        {
            strcpy(pszPath, g_savePathA);
        }
    }
    return ret;
}

static bool BuildSavePath()
{
    DWORD len = GetModuleFileNameW(NULL, g_savePath, MAX_PATH);
    if (len == 0)
        return false;

    wchar_t *lastSlash = wcsrchr(g_savePath, L'\\');
    if (!lastSlash)
        return false;
    *(lastSlash + 1) = L'\0';
    wcscat(g_savePath, L"save");
    CreateDirectoryW(g_savePath, NULL);

    WideCharToMultiByte(CP_ACP, 0, g_savePath, -1, g_savePathA, MAX_PATH, NULL, NULL);

    return true;
}

bool Hook_Init()
{
    if (!BuildSavePath())
        return false;

    if (MH_Initialize() != MH_OK)
        return false;

    MH_CreateHookApi(L"shell32", "SHGetPathFromIDListA",
                     (void *)HookedGetPathFromIDListA, (void **)&pOrigGetPathA);

    MH_EnableHook(MH_ALL_HOOKS);

    return true;
}

void Hook_Free()
{
    MH_DisableHook(MH_ALL_HOOKS);
    MH_Uninitialize();
}