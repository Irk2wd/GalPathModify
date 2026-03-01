#include <windows.h>
#include <cstring>
#include "proxy.h"

static FARPROC g_origFuncs[192];
static HMODULE g_realDll = NULL;

static const char* g_funcNames[] = {
    "mciExecute",
    "CloseDriver",
    "DefDriverProc",
    "DriverCallback",
    "DrvGetModuleHandle",
    "GetDriverModuleHandle",
    "NotifyCallbackData",
    "OpenDriver",
    "PlaySound",
    "PlaySoundA",
    "PlaySoundW",
    "SendDriverMessage",
    "WOW32DriverCallback",
    "WOW32ResolveMultiMediaHandle",
    "WOWAppExit",
    "aux32Message",
    "auxGetDevCapsA",
    "auxGetDevCapsW",
    "auxGetNumDevs",
    "auxGetVolume",
    "auxOutMessage",
    "auxSetVolume",
    "joy32Message",
    "joyConfigChanged",
    "joyGetDevCapsA",
    "joyGetDevCapsW",
    "joyGetNumDevs",
    "joyGetPos",
    "joyGetPosEx",
    "joyGetThreshold",
    "joyReleaseCapture",
    "joySetCapture",
    "joySetThreshold",
    "mci32Message",
    "mciDriverNotify",
    "mciDriverYield",
    "mciFreeCommandResource",
    "mciGetCreatorTask",
    "mciGetDeviceIDA",
    "mciGetDeviceIDFromElementIDA",
    "mciGetDeviceIDFromElementIDW",
    "mciGetDeviceIDW",
    "mciGetDriverData",
    "mciGetErrorStringA",
    "mciGetErrorStringW",
    "mciGetYieldProc",
    "mciLoadCommandResource",
    "mciSendCommandA",
    "mciSendCommandW",
    "mciSendStringA",
    "mciSendStringW",
    "mciSetDriverData",
    "mciSetYieldProc",
    "mid32Message",
    "midiConnect",
    "midiDisconnect",
    "midiInAddBuffer",
    "midiInClose",
    "midiInGetDevCapsA",
    "midiInGetDevCapsW",
    "midiInGetErrorTextA",
    "midiInGetErrorTextW",
    "midiInGetID",
    "midiInGetNumDevs",
    "midiInMessage",
    "midiInOpen",
    "midiInPrepareHeader",
    "midiInReset",
    "midiInStart",
    "midiInStop",
    "midiInUnprepareHeader",
    "midiOutCacheDrumPatches",
    "midiOutCachePatches",
    "midiOutClose",
    "midiOutGetDevCapsA",
    "midiOutGetDevCapsW",
    "midiOutGetErrorTextA",
    "midiOutGetErrorTextW",
    "midiOutGetID",
    "midiOutGetNumDevs",
    "midiOutGetVolume",
    "midiOutLongMsg",
    "midiOutMessage",
    "midiOutOpen",
    "midiOutPrepareHeader",
    "midiOutReset",
    "midiOutSetVolume",
    "midiOutShortMsg",
    "midiOutUnprepareHeader",
    "midiStreamClose",
    "midiStreamOpen",
    "midiStreamOut",
    "midiStreamPause",
    "midiStreamPosition",
    "midiStreamProperty",
    "midiStreamRestart",
    "midiStreamStop",
    "mixerClose",
    "mixerGetControlDetailsA",
    "mixerGetControlDetailsW",
    "mixerGetDevCapsA",
    "mixerGetDevCapsW",
    "mixerGetID",
    "mixerGetLineControlsA",
    "mixerGetLineControlsW",
    "mixerGetLineInfoA",
    "mixerGetLineInfoW",
    "mixerGetNumDevs",
    "mixerMessage",
    "mixerOpen",
    "mixerSetControlDetails",
    "mmDrvInstall",
    "mmGetCurrentTask",
    "mmTaskBlock",
    "mmTaskCreate",
    "mmTaskSignal",
    "mmTaskYield",
    "mmioAdvance",
    "mmioAscend",
    "mmioClose",
    "mmioCreateChunk",
    "mmioDescend",
    "mmioFlush",
    "mmioGetInfo",
    "mmioInstallIOProcA",
    "mmioInstallIOProcW",
    "mmioOpenA",
    "mmioOpenW",
    "mmioRead",
    "mmioRenameA",
    "mmioRenameW",
    "mmioSeek",
    "mmioSendMessage",
    "mmioSetBuffer",
    "mmioSetInfo",
    "mmioStringToFOURCCA",
    "mmioStringToFOURCCW",
    "mmioWrite",
    "mmsystemGetVersion",
    "mod32Message",
    "mxd32Message",
    "sndPlaySoundA",
    "sndPlaySoundW",
    "tid32Message",
    "timeBeginPeriod",
    "timeEndPeriod",
    "timeGetDevCaps",
    "timeGetSystemTime",
    "timeGetTime",
    "timeKillEvent",
    "timeSetEvent",
    "waveInAddBuffer",
    "waveInClose",
    "waveInGetDevCapsA",
    "waveInGetDevCapsW",
    "waveInGetErrorTextA",
    "waveInGetErrorTextW",
    "waveInGetID",
    "waveInGetNumDevs",
    "waveInGetPosition",
    "waveInMessage",
    "waveInOpen",
    "waveInPrepareHeader",
    "waveInReset",
    "waveInStart",
    "waveInStop",
    "waveInUnprepareHeader",
    "waveOutBreakLoop",
    "waveOutClose",
    "waveOutGetDevCapsA",
    "waveOutGetDevCapsW",
    "waveOutGetErrorTextA",
    "waveOutGetErrorTextW",
    "waveOutGetID",
    "waveOutGetNumDevs",
    "waveOutGetPitch",
    "waveOutGetPlaybackRate",
    "waveOutGetPosition",
    "waveOutGetVolume",
    "waveOutMessage",
    "waveOutOpen",
    "waveOutPause",
    "waveOutPrepareHeader",
    "waveOutReset",
    "waveOutRestart",
    "waveOutSetPitch",
    "waveOutSetPlaybackRate",
    "waveOutSetVolume",
    "waveOutUnprepareHeader",
    "waveOutWrite",
    "wid32Message",
    "wod32Message",
};

static const int FUNC_COUNT = 192;

bool Proxy_Init() {
    char path[MAX_PATH];
    GetSystemDirectoryA(path, MAX_PATH);
    strcat(path, "\\winmm.dll");
    g_realDll = LoadLibraryA(path);
    if (!g_realDll) return false;
    for (int i = 0; i < FUNC_COUNT; i++)
        g_origFuncs[i] = GetProcAddress(g_realDll, g_funcNames[i]);
    return true;
}

void Proxy_Free() {
    if (g_realDll) { FreeLibrary(g_realDll); g_realDll = NULL; }
}

extern "C" __attribute__((naked)) void proxy_mciExecute() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[0]));
}

extern "C" __attribute__((naked)) void proxy_CloseDriver() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[1]));
}

extern "C" __attribute__((naked)) void proxy_DefDriverProc() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[2]));
}

extern "C" __attribute__((naked)) void proxy_DriverCallback() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[3]));
}

extern "C" __attribute__((naked)) void proxy_DrvGetModuleHandle() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[4]));
}

extern "C" __attribute__((naked)) void proxy_GetDriverModuleHandle() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[5]));
}

extern "C" __attribute__((naked)) void proxy_NotifyCallbackData() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[6]));
}

extern "C" __attribute__((naked)) void proxy_OpenDriver() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[7]));
}

extern "C" __attribute__((naked)) void proxy_PlaySound() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[8]));
}

extern "C" __attribute__((naked)) void proxy_PlaySoundA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[9]));
}

extern "C" __attribute__((naked)) void proxy_PlaySoundW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[10]));
}

extern "C" __attribute__((naked)) void proxy_SendDriverMessage() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[11]));
}

extern "C" __attribute__((naked)) void proxy_WOW32DriverCallback() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[12]));
}

extern "C" __attribute__((naked)) void proxy_WOW32ResolveMultiMediaHandle() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[13]));
}

extern "C" __attribute__((naked)) void proxy_WOWAppExit() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[14]));
}

extern "C" __attribute__((naked)) void proxy_aux32Message() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[15]));
}

extern "C" __attribute__((naked)) void proxy_auxGetDevCapsA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[16]));
}

extern "C" __attribute__((naked)) void proxy_auxGetDevCapsW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[17]));
}

extern "C" __attribute__((naked)) void proxy_auxGetNumDevs() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[18]));
}

extern "C" __attribute__((naked)) void proxy_auxGetVolume() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[19]));
}

extern "C" __attribute__((naked)) void proxy_auxOutMessage() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[20]));
}

extern "C" __attribute__((naked)) void proxy_auxSetVolume() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[21]));
}

extern "C" __attribute__((naked)) void proxy_joy32Message() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[22]));
}

extern "C" __attribute__((naked)) void proxy_joyConfigChanged() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[23]));
}

extern "C" __attribute__((naked)) void proxy_joyGetDevCapsA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[24]));
}

extern "C" __attribute__((naked)) void proxy_joyGetDevCapsW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[25]));
}

extern "C" __attribute__((naked)) void proxy_joyGetNumDevs() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[26]));
}

extern "C" __attribute__((naked)) void proxy_joyGetPos() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[27]));
}

extern "C" __attribute__((naked)) void proxy_joyGetPosEx() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[28]));
}

extern "C" __attribute__((naked)) void proxy_joyGetThreshold() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[29]));
}

extern "C" __attribute__((naked)) void proxy_joyReleaseCapture() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[30]));
}

extern "C" __attribute__((naked)) void proxy_joySetCapture() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[31]));
}

extern "C" __attribute__((naked)) void proxy_joySetThreshold() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[32]));
}

extern "C" __attribute__((naked)) void proxy_mci32Message() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[33]));
}

extern "C" __attribute__((naked)) void proxy_mciDriverNotify() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[34]));
}

extern "C" __attribute__((naked)) void proxy_mciDriverYield() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[35]));
}

extern "C" __attribute__((naked)) void proxy_mciFreeCommandResource() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[36]));
}

extern "C" __attribute__((naked)) void proxy_mciGetCreatorTask() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[37]));
}

extern "C" __attribute__((naked)) void proxy_mciGetDeviceIDA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[38]));
}

extern "C" __attribute__((naked)) void proxy_mciGetDeviceIDFromElementIDA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[39]));
}

extern "C" __attribute__((naked)) void proxy_mciGetDeviceIDFromElementIDW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[40]));
}

extern "C" __attribute__((naked)) void proxy_mciGetDeviceIDW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[41]));
}

extern "C" __attribute__((naked)) void proxy_mciGetDriverData() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[42]));
}

extern "C" __attribute__((naked)) void proxy_mciGetErrorStringA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[43]));
}

extern "C" __attribute__((naked)) void proxy_mciGetErrorStringW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[44]));
}

extern "C" __attribute__((naked)) void proxy_mciGetYieldProc() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[45]));
}

extern "C" __attribute__((naked)) void proxy_mciLoadCommandResource() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[46]));
}

extern "C" __attribute__((naked)) void proxy_mciSendCommandA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[47]));
}

extern "C" __attribute__((naked)) void proxy_mciSendCommandW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[48]));
}

extern "C" __attribute__((naked)) void proxy_mciSendStringA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[49]));
}

extern "C" __attribute__((naked)) void proxy_mciSendStringW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[50]));
}

extern "C" __attribute__((naked)) void proxy_mciSetDriverData() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[51]));
}

extern "C" __attribute__((naked)) void proxy_mciSetYieldProc() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[52]));
}

extern "C" __attribute__((naked)) void proxy_mid32Message() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[53]));
}

extern "C" __attribute__((naked)) void proxy_midiConnect() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[54]));
}

extern "C" __attribute__((naked)) void proxy_midiDisconnect() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[55]));
}

extern "C" __attribute__((naked)) void proxy_midiInAddBuffer() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[56]));
}

extern "C" __attribute__((naked)) void proxy_midiInClose() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[57]));
}

extern "C" __attribute__((naked)) void proxy_midiInGetDevCapsA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[58]));
}

extern "C" __attribute__((naked)) void proxy_midiInGetDevCapsW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[59]));
}

extern "C" __attribute__((naked)) void proxy_midiInGetErrorTextA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[60]));
}

extern "C" __attribute__((naked)) void proxy_midiInGetErrorTextW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[61]));
}

extern "C" __attribute__((naked)) void proxy_midiInGetID() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[62]));
}

extern "C" __attribute__((naked)) void proxy_midiInGetNumDevs() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[63]));
}

extern "C" __attribute__((naked)) void proxy_midiInMessage() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[64]));
}

extern "C" __attribute__((naked)) void proxy_midiInOpen() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[65]));
}

extern "C" __attribute__((naked)) void proxy_midiInPrepareHeader() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[66]));
}

extern "C" __attribute__((naked)) void proxy_midiInReset() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[67]));
}

extern "C" __attribute__((naked)) void proxy_midiInStart() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[68]));
}

extern "C" __attribute__((naked)) void proxy_midiInStop() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[69]));
}

extern "C" __attribute__((naked)) void proxy_midiInUnprepareHeader() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[70]));
}

extern "C" __attribute__((naked)) void proxy_midiOutCacheDrumPatches() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[71]));
}

extern "C" __attribute__((naked)) void proxy_midiOutCachePatches() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[72]));
}

extern "C" __attribute__((naked)) void proxy_midiOutClose() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[73]));
}

extern "C" __attribute__((naked)) void proxy_midiOutGetDevCapsA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[74]));
}

extern "C" __attribute__((naked)) void proxy_midiOutGetDevCapsW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[75]));
}

extern "C" __attribute__((naked)) void proxy_midiOutGetErrorTextA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[76]));
}

extern "C" __attribute__((naked)) void proxy_midiOutGetErrorTextW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[77]));
}

extern "C" __attribute__((naked)) void proxy_midiOutGetID() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[78]));
}

extern "C" __attribute__((naked)) void proxy_midiOutGetNumDevs() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[79]));
}

extern "C" __attribute__((naked)) void proxy_midiOutGetVolume() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[80]));
}

extern "C" __attribute__((naked)) void proxy_midiOutLongMsg() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[81]));
}

extern "C" __attribute__((naked)) void proxy_midiOutMessage() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[82]));
}

extern "C" __attribute__((naked)) void proxy_midiOutOpen() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[83]));
}

extern "C" __attribute__((naked)) void proxy_midiOutPrepareHeader() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[84]));
}

extern "C" __attribute__((naked)) void proxy_midiOutReset() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[85]));
}

extern "C" __attribute__((naked)) void proxy_midiOutSetVolume() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[86]));
}

extern "C" __attribute__((naked)) void proxy_midiOutShortMsg() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[87]));
}

extern "C" __attribute__((naked)) void proxy_midiOutUnprepareHeader() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[88]));
}

extern "C" __attribute__((naked)) void proxy_midiStreamClose() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[89]));
}

extern "C" __attribute__((naked)) void proxy_midiStreamOpen() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[90]));
}

extern "C" __attribute__((naked)) void proxy_midiStreamOut() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[91]));
}

extern "C" __attribute__((naked)) void proxy_midiStreamPause() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[92]));
}

extern "C" __attribute__((naked)) void proxy_midiStreamPosition() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[93]));
}

extern "C" __attribute__((naked)) void proxy_midiStreamProperty() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[94]));
}

extern "C" __attribute__((naked)) void proxy_midiStreamRestart() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[95]));
}

extern "C" __attribute__((naked)) void proxy_midiStreamStop() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[96]));
}

extern "C" __attribute__((naked)) void proxy_mixerClose() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[97]));
}

extern "C" __attribute__((naked)) void proxy_mixerGetControlDetailsA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[98]));
}

extern "C" __attribute__((naked)) void proxy_mixerGetControlDetailsW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[99]));
}

extern "C" __attribute__((naked)) void proxy_mixerGetDevCapsA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[100]));
}

extern "C" __attribute__((naked)) void proxy_mixerGetDevCapsW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[101]));
}

extern "C" __attribute__((naked)) void proxy_mixerGetID() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[102]));
}

extern "C" __attribute__((naked)) void proxy_mixerGetLineControlsA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[103]));
}

extern "C" __attribute__((naked)) void proxy_mixerGetLineControlsW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[104]));
}

extern "C" __attribute__((naked)) void proxy_mixerGetLineInfoA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[105]));
}

extern "C" __attribute__((naked)) void proxy_mixerGetLineInfoW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[106]));
}

extern "C" __attribute__((naked)) void proxy_mixerGetNumDevs() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[107]));
}

extern "C" __attribute__((naked)) void proxy_mixerMessage() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[108]));
}

extern "C" __attribute__((naked)) void proxy_mixerOpen() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[109]));
}

extern "C" __attribute__((naked)) void proxy_mixerSetControlDetails() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[110]));
}

extern "C" __attribute__((naked)) void proxy_mmDrvInstall() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[111]));
}

extern "C" __attribute__((naked)) void proxy_mmGetCurrentTask() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[112]));
}

extern "C" __attribute__((naked)) void proxy_mmTaskBlock() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[113]));
}

extern "C" __attribute__((naked)) void proxy_mmTaskCreate() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[114]));
}

extern "C" __attribute__((naked)) void proxy_mmTaskSignal() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[115]));
}

extern "C" __attribute__((naked)) void proxy_mmTaskYield() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[116]));
}

extern "C" __attribute__((naked)) void proxy_mmioAdvance() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[117]));
}

extern "C" __attribute__((naked)) void proxy_mmioAscend() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[118]));
}

extern "C" __attribute__((naked)) void proxy_mmioClose() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[119]));
}

extern "C" __attribute__((naked)) void proxy_mmioCreateChunk() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[120]));
}

extern "C" __attribute__((naked)) void proxy_mmioDescend() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[121]));
}

extern "C" __attribute__((naked)) void proxy_mmioFlush() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[122]));
}

extern "C" __attribute__((naked)) void proxy_mmioGetInfo() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[123]));
}

extern "C" __attribute__((naked)) void proxy_mmioInstallIOProcA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[124]));
}

extern "C" __attribute__((naked)) void proxy_mmioInstallIOProcW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[125]));
}

extern "C" __attribute__((naked)) void proxy_mmioOpenA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[126]));
}

extern "C" __attribute__((naked)) void proxy_mmioOpenW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[127]));
}

extern "C" __attribute__((naked)) void proxy_mmioRead() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[128]));
}

extern "C" __attribute__((naked)) void proxy_mmioRenameA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[129]));
}

extern "C" __attribute__((naked)) void proxy_mmioRenameW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[130]));
}

extern "C" __attribute__((naked)) void proxy_mmioSeek() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[131]));
}

extern "C" __attribute__((naked)) void proxy_mmioSendMessage() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[132]));
}

extern "C" __attribute__((naked)) void proxy_mmioSetBuffer() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[133]));
}

extern "C" __attribute__((naked)) void proxy_mmioSetInfo() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[134]));
}

extern "C" __attribute__((naked)) void proxy_mmioStringToFOURCCA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[135]));
}

extern "C" __attribute__((naked)) void proxy_mmioStringToFOURCCW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[136]));
}

extern "C" __attribute__((naked)) void proxy_mmioWrite() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[137]));
}

extern "C" __attribute__((naked)) void proxy_mmsystemGetVersion() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[138]));
}

extern "C" __attribute__((naked)) void proxy_mod32Message() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[139]));
}

extern "C" __attribute__((naked)) void proxy_mxd32Message() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[140]));
}

extern "C" __attribute__((naked)) void proxy_sndPlaySoundA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[141]));
}

extern "C" __attribute__((naked)) void proxy_sndPlaySoundW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[142]));
}

extern "C" __attribute__((naked)) void proxy_tid32Message() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[143]));
}

extern "C" __attribute__((naked)) void proxy_timeBeginPeriod() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[144]));
}

extern "C" __attribute__((naked)) void proxy_timeEndPeriod() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[145]));
}

extern "C" __attribute__((naked)) void proxy_timeGetDevCaps() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[146]));
}

extern "C" __attribute__((naked)) void proxy_timeGetSystemTime() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[147]));
}

extern "C" __attribute__((naked)) void proxy_timeGetTime() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[148]));
}

extern "C" __attribute__((naked)) void proxy_timeKillEvent() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[149]));
}

extern "C" __attribute__((naked)) void proxy_timeSetEvent() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[150]));
}

extern "C" __attribute__((naked)) void proxy_waveInAddBuffer() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[151]));
}

extern "C" __attribute__((naked)) void proxy_waveInClose() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[152]));
}

extern "C" __attribute__((naked)) void proxy_waveInGetDevCapsA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[153]));
}

extern "C" __attribute__((naked)) void proxy_waveInGetDevCapsW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[154]));
}

extern "C" __attribute__((naked)) void proxy_waveInGetErrorTextA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[155]));
}

extern "C" __attribute__((naked)) void proxy_waveInGetErrorTextW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[156]));
}

extern "C" __attribute__((naked)) void proxy_waveInGetID() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[157]));
}

extern "C" __attribute__((naked)) void proxy_waveInGetNumDevs() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[158]));
}

extern "C" __attribute__((naked)) void proxy_waveInGetPosition() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[159]));
}

extern "C" __attribute__((naked)) void proxy_waveInMessage() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[160]));
}

extern "C" __attribute__((naked)) void proxy_waveInOpen() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[161]));
}

extern "C" __attribute__((naked)) void proxy_waveInPrepareHeader() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[162]));
}

extern "C" __attribute__((naked)) void proxy_waveInReset() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[163]));
}

extern "C" __attribute__((naked)) void proxy_waveInStart() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[164]));
}

extern "C" __attribute__((naked)) void proxy_waveInStop() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[165]));
}

extern "C" __attribute__((naked)) void proxy_waveInUnprepareHeader() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[166]));
}

extern "C" __attribute__((naked)) void proxy_waveOutBreakLoop() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[167]));
}

extern "C" __attribute__((naked)) void proxy_waveOutClose() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[168]));
}

extern "C" __attribute__((naked)) void proxy_waveOutGetDevCapsA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[169]));
}

extern "C" __attribute__((naked)) void proxy_waveOutGetDevCapsW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[170]));
}

extern "C" __attribute__((naked)) void proxy_waveOutGetErrorTextA() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[171]));
}

extern "C" __attribute__((naked)) void proxy_waveOutGetErrorTextW() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[172]));
}

extern "C" __attribute__((naked)) void proxy_waveOutGetID() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[173]));
}

extern "C" __attribute__((naked)) void proxy_waveOutGetNumDevs() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[174]));
}

extern "C" __attribute__((naked)) void proxy_waveOutGetPitch() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[175]));
}

extern "C" __attribute__((naked)) void proxy_waveOutGetPlaybackRate() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[176]));
}

extern "C" __attribute__((naked)) void proxy_waveOutGetPosition() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[177]));
}

extern "C" __attribute__((naked)) void proxy_waveOutGetVolume() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[178]));
}

extern "C" __attribute__((naked)) void proxy_waveOutMessage() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[179]));
}

extern "C" __attribute__((naked)) void proxy_waveOutOpen() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[180]));
}

extern "C" __attribute__((naked)) void proxy_waveOutPause() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[181]));
}

extern "C" __attribute__((naked)) void proxy_waveOutPrepareHeader() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[182]));
}

extern "C" __attribute__((naked)) void proxy_waveOutReset() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[183]));
}

extern "C" __attribute__((naked)) void proxy_waveOutRestart() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[184]));
}

extern "C" __attribute__((naked)) void proxy_waveOutSetPitch() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[185]));
}

extern "C" __attribute__((naked)) void proxy_waveOutSetPlaybackRate() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[186]));
}

extern "C" __attribute__((naked)) void proxy_waveOutSetVolume() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[187]));
}

extern "C" __attribute__((naked)) void proxy_waveOutUnprepareHeader() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[188]));
}

extern "C" __attribute__((naked)) void proxy_waveOutWrite() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[189]));
}

extern "C" __attribute__((naked)) void proxy_wid32Message() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[190]));
}

extern "C" __attribute__((naked)) void proxy_wod32Message() {
    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[191]));
}

