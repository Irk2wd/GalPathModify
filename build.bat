@echo off
set GCC32=D:\3rdparty\mingw32\bin\g++.exe

%GCC32% -O2 -shared -static -m32 ^
    src\dllmain.cpp ^
    src\proxy.cpp ^
    src\hook.cpp ^
    minhook\src\hook.c ^
    minhook\src\trampoline.c ^
    minhook\src\buffer.c ^
    minhook\src\hde\hde32.c ^
    -I . ^
    -I src ^
    -I minhook\include ^
    -o winmm.dll ^
    -Wl,--enable-stdcall-fixup ^
    -Wl,src\exports.def ^
    -lshlwapi -lole32

if %errorlevel%==0 (
    echo [OK] winmm.dll
) else (
    echo [FAIL]
)
pause