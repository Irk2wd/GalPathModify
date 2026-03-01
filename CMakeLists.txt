cmake_minimum_required(VERSION 3.15)
project(winmm_proxy C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_STANDARD 11)

# ---- MinHook sources ----
set(MINHOOK_DIR ${CMAKE_SOURCE_DIR}/minhook)
set(MINHOOK_SRC
    ${MINHOOK_DIR}/src/buffer.c
    ${MINHOOK_DIR}/src/hook.c
    ${MINHOOK_DIR}/src/trampoline.c
    ${MINHOOK_DIR}/src/hde/hde32.c
)

# ---- Proxy DLL ----
add_library(winmm SHARED
    src/proxy.cpp
    src/dllmain.cpp
    src/hook.cpp
    src/exports.def
    ${MINHOOK_SRC}
)

target_include_directories(winmm PRIVATE
    ${CMAKE_SOURCE_DIR}/src
    ${MINHOOK_DIR}/include
)

target_compile_options(winmm PRIVATE
    -O2
)

target_link_options(winmm PRIVATE
    -static-libgcc
    -static-libstdc++
    -s
)

set_target_properties(winmm PROPERTIES
    PREFIX ""
    OUTPUT_NAME "winmm"
    SUFFIX ".dll"
)