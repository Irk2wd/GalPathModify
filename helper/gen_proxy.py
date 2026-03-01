import re, sys

functions = []

with open("helper/winmm.def", "r") as f:
    for line in f:
        line = line.strip()
        # 跳过注释、空行、LIBRARY、EXPORTS
        if (
            not line
            or line.startswith(";")
            or line.startswith("LIBRARY")
            or line == "EXPORTS"
        ):
            continue
        # 跳过 NONAME（无名导出，按序号导出的，极少被调用）
        if "NONAME" in line:
            continue
        # 提取函数名：取第一个 token，去掉 @N 后缀和 ; Check 注释
        token = line.split()[0]  # "mciExecute@4" 或 "CloseDriver"
        name = token.split("@")[0]  # "mciExecute" 或 "CloseDriver"
        if name and name.isidentifier():
            functions.append(name)

# 去重（有些函数可能重复出现）
functions = list(dict.fromkeys(functions))

print(f"找到 {len(functions)} 个函数")
for name in functions:
    print(f"  {name}")

# 生成 exports.def
with open("src/exports.def", "w") as f:
    f.write("LIBRARY winmm\n")
    f.write("EXPORTS\n")
    for i, name in enumerate(functions):
        f.write(f"    {name}=proxy_{name} @{i+1}\n")

# 生成 proxy.h
with open("src/proxy.h", "w") as f:
    f.write("#pragma once\n")
    f.write("#include <windows.h>\n\n")
    f.write("bool Proxy_Init();\n")
    f.write("void Proxy_Free();\n")

# 生成 proxy.cpp
with open("src/proxy.cpp", "w") as f:
    f.write("#include <windows.h>\n")
    f.write("#include <cstring>\n")
    f.write('#include "proxy.h"\n\n')

    f.write(f"static FARPROC g_origFuncs[{len(functions)}];\n")
    f.write("static HMODULE g_realDll = NULL;\n\n")

    f.write("static const char* g_funcNames[] = {\n")
    for name in functions:
        f.write(f'    "{name}",\n')
    f.write("};\n\n")

    f.write(f"static const int FUNC_COUNT = {len(functions)};\n\n")

    # Init
    f.write("bool Proxy_Init() {\n")
    f.write("    char path[MAX_PATH];\n")
    f.write("    GetSystemDirectoryA(path, MAX_PATH);\n")
    f.write('    strcat(path, "\\\\winmm.dll");\n')
    f.write("    g_realDll = LoadLibraryA(path);\n")
    f.write("    if (!g_realDll) return false;\n")
    f.write("    for (int i = 0; i < FUNC_COUNT; i++)\n")
    f.write("        g_origFuncs[i] = GetProcAddress(g_realDll, g_funcNames[i]);\n")
    f.write("    return true;\n")
    f.write("}\n\n")

    # Free
    f.write("void Proxy_Free() {\n")
    f.write("    if (g_realDll) { FreeLibrary(g_realDll); g_realDll = NULL; }\n")
    f.write("}\n\n")

    # 转发函数（GCC naked + 内联汇编）
    for i, name in enumerate(functions):
        f.write(f'extern "C" __attribute__((naked)) void proxy_{name}() {{\n')
        f.write(f'    __asm__ __volatile__("jmp *%0" :: "m"(g_origFuncs[{i}]));\n')
        f.write(f"}}\n\n")

print(f"\n生成完毕:")
print(f"  src/exports.def  ({len(functions)} 个导出)")
print(f"  src/proxy.h")
print(f"  src/proxy.cpp    ({len(functions)} 个转发函数)")
