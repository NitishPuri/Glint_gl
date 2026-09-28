# Building

## Linux (Pop!_OS / Ubuntu 22.04, verified 2026-09-29)

```bash
sudo apt install cmake ninja-build libglfw3-dev     # GLFW 3.3.6 is enough
git submodule update --init                         # ext/glm
./build.sh            # = cmake -S . -B build/Debug -G Ninja -DCMAKE_BUILD_TYPE=Debug && cmake --build build/Debug
./run.sh              # build + run ./build/Debug/Glint
./run.sh Release
```

Verified on an AMD Radeon Renoir: `Version: 4.6 (Core Profile) Mesa 25.1.5`, `GLSL 4.60`.

## Windows (original setup)

Visual Studio 2022 with CMake, either `generate.bat` + `build.bat` / `run.bat`, or open the folder in VS
(`CMakeSettings.json` defines the x64 Debug and Release Ninja configurations). GLFW comes from the prebuilt
`ext/glfw-3.4.bin.WIN64/lib-vc2022`.

## CMake overview (`CMakeLists.txt`)

- One target, `Glint`: globbed `src/**/*.cpp` + glad + all of `ext/imgui/*.cpp`, with headers and shaders
  added for IDE visibility.
- `ROOT="<source dir>"` is a compile definition that `getFilePath()` uses to locate shaders and assets at runtime.
- GLFW: prebuilt on `WIN32`, otherwise `find_package(glfw3 3.3)`.
- `std::format` check: `check_cxx_source_compiles`. If the check fails (GCC < 13), `{fmt}` 11.1.4 is
  fetched with FetchContent and linked. `Logger.h` picks the implementation through `<version>` /
  `__cpp_lib_format` and the `fmtlib` namespace alias.
- Non-MSVC Debug builds define `_DEBUG`, which enables `GLCall`, the GL debug context and the `KHR_debug` callback.
- The default build type is Debug; `compile_commands.json` is exported for clangd.
- Warnings: `/W4 /permissive-` (MSVC), `-Wall -Wextra -pedantic` (others).
- `tests/` (boost.ut) is **not** part of the build, and `tests/camera_test.cpp` refers to a
  `core/camera.h` that doesn't exist.

## What the Linux port changed (commit `1fa521e`)

| Problem | Fix |
|---|---|
| CMake linked `glfw3` from `lib-vc2022` and `opengl32.lib` | system GLFW + `OpenGL::GL` + `${CMAKE_DL_LIBS}` off Windows |
| `std::format` missing in GCC 11 | `{fmt}` fallback + `fmtlib` alias in `Logger.h` |
| `#pragma warning(disable:4996)` (MSVC) | guarded by `_MSC_VER` |
| `__debugbreak()` (MSVC intrinsic) | `DEBUG_BREAK()` → `std::raise(SIGTRAP)` elsewhere |
| `windows.h` memory and CPU queries | `sysinfo()` + `std::thread::hardware_concurrency()` |
| `std::shared_ptr` / `memcmp` without includes (MSVC included them transitively) | `<memory>`, `<string>` in `Renderer.h`; `<cstring>` in `VBOIndex.cpp` |
| `_DEBUG` not defined by GCC/Clang | `$<$<CONFIG:Debug>:_DEBUG>` |

The Windows build path is unchanged. Line endings are CRLF across the repo (commit `682f960`); the new
`.sh` scripts are LF.
