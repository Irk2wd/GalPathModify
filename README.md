# White Album 2 存档路径重定向

## 动机

White Album 2 的存档固定写入 `C:\\Users\\xxx\\Documents`，且不提供手动指定路径的选项。本项目通过 DLL 代理注入 + API Hook，将存档重定向到游戏本体目录下的 `save` 文件夹。

## 技术路线

### 原理分析

游戏通过 Windows 早期（Windows 95 时代）的 COM PIDL 方式获取"我的文档"路径：

```cpp
// 第一步：获取 PIDL
LPITEMIDLIST pidl;
SHGetSpecialFolderLocation(NULL, CSIDL_PERSONAL, &pidl);

// 第二步：PIDL → 路径字符串
SHGetPathFromIDListA(pidl, path);

// 第三步：手动释放 PIDL
IMalloc* pMalloc;
SHGetMalloc(&pMalloc);
pMalloc->Free(pidl);
pMalloc->Release();
```

> 💡 EXE 的导入导出表可使用 [Dependencies](https://github.com/lucasg/Dependencies) 工具查看。

### 实现方案

1. **DLL 代理注入** — 以 `winmm.dll` 为代理目标，游戏加载时自动载入自定义 DLL
   > 选择 `winmm.dll` 而非 `shell32.dll` 作为代理目标，是因为 `shell32.dll` 层级更底层，代理风险更大
2. **API Hook** — DLL 加载时通过 [MinHook](https://github.com/TsudaKageworU/minhook) 拦截 `SHGetPathFromIDListA`
3. **路径替换** — 在拦截函数中将"我的文档"路径替换为 `<游戏目录>\save`

## 用法

### 直接使用

将仓库中已编译好的 `winmm.dll` 复制到游戏目录即可。启动游戏后会自动在游戏目录下创建 `save` 文件夹。

### 从源码构建

**前置条件：**

- Python 环境
- 支持 32 位编译的 MinGW 工具链（可从 [winlibs.com](https://winlibs.com/) 下载），并修改 `CMakeLists.txt` 中对应的路径

**构建步骤：**

```bash
# 克隆项目
git clone --recurse-submodules https://github.com/Irk2wd/GalPathModify.git
cd GalPathModify

# 生成代理源码
python helper/gen_proxy.py C:\\Windows\\SysWOW64\\winmm.dll -o src/

# 编译
mkdir build && cd build
cmake -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=../mingw32.cmake ..
mingw32-make
```

将 `build` 目录下生成的 `winmm.dll` 复制到游戏目录即可。
## Build Type and Logging

- `Debug` build: logging is enabled and writes `galpathmodify.log` in the game directory.
- `Release` build: logging is disabled (no log file output).

### Debug build

```bash
cmake -S . -B build-debug -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=mingw32.cmake -DCMAKE_BUILD_TYPE=Debug
cmake --build build-debug
```

### Release build

```bash
cmake -S . -B build-release -G "MinGW Makefiles" -DCMAKE_TOOLCHAIN_FILE=mingw32.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build build-release
```
