# `external/` — third-party dependencies

This directory holds the third-party code Slang depends on, together with the
build plumbing that wires each dependency into the CMake build. This file
documents **what each dependency is for** and **which CMake option enables,
disables, or configures it**.

The authoritative source of truth is always the build files themselves —
[`external/CMakeLists.txt`](CMakeLists.txt), the top-level
[`CMakeLists.txt`](../CMakeLists.txt), [`tools/CMakeLists.txt`](../tools/CMakeLists.txt),
and [`.gitmodules`](../.gitmodules). This document is a curated overview kept
deliberately high-level so it does not drift as individual option lines move.

## Kinds of content in this directory

Not everything under `external/` is a git submodule. There are four distinct
kinds of content, and it helps to know which is which:

- **Git submodules (18)** — fetched by `git submodule update --init --recursive`:
  `glslang`, `spirv-tools`, `spirv-headers`, `vulkan` (Vulkan-Headers),
  `slang-rhi`, `glm`, `imgui`, `tinyobjloader`, `lua`, `metal-cpp`, `miniz`,
  `lz4`, `unordered_dense`, `fast_float`, `cmark`, `mimalloc`, `optix-dev`,
  `WindowsToolchain`.
- **Vendored headers (checked in, not submodules)** — small header sets kept
  directly in the tree: `dxc/` (`dxcapi.h`, `WinAdapter.h`), `stb/`, `spirv/`
  (`spirv.h`), `slang-tint-headers/`, `glext.h`, `wglext.h`, `renderdoc_app.h`.
- **Generated at build time** — populated by the build, not checked in:
  `glslang-generated/`, `spirv-tools-generated/`.
- **Fetched as prebuilt binaries (some with a source-build fallback)** —
  obtained by CMake at configure time rather than kept in the tree:
  `slang-tint`, `webgpu_dawn`, `slang-llvm`, and DXC. DXC is a prebuilt download
  on most configurations, but it is built from source when
  `SLANG_DXC_BUILD_FROM_SOURCE=ON`, on macOS by default, and as a Linux fallback
  when the prebuilt binary needs a newer GLIBC than the host provides (see
  `cmake/FetchDXC.cmake`). The helper scripts `build-llvm.sh` /
  `build-llvm.ps1` and `bump-glslang.sh` live alongside them.

## Dependency reference

| Dependency                                         | Purpose                                                                                                                                                                     | Enable / configure option(s) (default)                                                                                                                   |
| -------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `glslang`                                          | GLSL front-end and the SPIRV / SPIRV-Tools-opt / SPIRV-Tools-link libraries, consumed through the `slang-glslang` wrapper.                                                  | `SLANG_ENABLE_SLANG_GLSLANG` (ON); system build via `SLANG_USE_SYSTEM_GLSLANG` (OFF)                                                                     |
| `spirv-tools`                                      | SPIR-V validator / optimizer / linker. Built as part of the glslang path.                                                                                                   | Built when `SLANG_ENABLE_SLANG_GLSLANG` is ON; `SLANG_USE_SYSTEM_SPIRV_TOOLS` (OFF); `SLANG_ENABLE_SPIRV_TOOLS_MIMALLOC` (platform default)              |
| `spirv-headers`                                    | SPIR-V specification headers and grammar JSON used by the compiler core and code generation.                                                                                | `SLANG_USE_SYSTEM_SPIRV_HEADERS` (OFF)                                                                                                                   |
| `vulkan` (Vulkan-Headers)                          | Vulkan API headers. Also reused by `slang-rhi`'s own FetchContent request.                                                                                                  | `SLANG_USE_SYSTEM_VULKAN_HEADERS` (OFF)                                                                                                                  |
| `slang-rhi`                                        | Render-hardware-interface layer used by `gfx`, tests, and examples.                                                                                                         | `SLANG_ENABLE_SLANG_RHI` (ON)                                                                                                                            |
| `glm`                                              | Math library for tools and examples.                                                                                                                                        | Consumed by `tools/`; `SLANG_OVERRIDE_GLM_PATH` (OFF)                                                                                                    |
| `imgui`                                            | Immediate-mode GUI for the graphics examples.                                                                                                                               | Built when `SLANG_ENABLE_GFX`, or `SLANG_ENABLE_SLANG_RHI` together with `SLANG_ENABLE_TESTS`/`SLANG_ENABLE_EXAMPLES`; `SLANG_OVERRIDE_IMGUI_PATH` (OFF) |
| `tinyobjloader`                                    | Wavefront `.obj` loader for examples.                                                                                                                                       | Consumed by `tools/`; `SLANG_OVERRIDE_TINYOBJLOADER_PATH` (OFF)                                                                                          |
| `lua`                                              | Scripting language embedded by the `slang-fiddle` code generator (build tooling).                                                                                           | `SLANG_OVERRIDE_LUA_PATH` (OFF)                                                                                                                          |
| `metal-cpp`                                        | Metal C++ bindings for the Metal backend (macOS). Header-only `INTERFACE` target, always available.                                                                         | — (no option; header-only)                                                                                                                               |
| `miniz`                                            | zlib-compatible (de)compression used by the core and runtime.                                                                                                               | `SLANG_USE_SYSTEM_MINIZ` (OFF)                                                                                                                           |
| `lz4`                                              | LZ4 (de)compression used by the core and runtime.                                                                                                                           | `SLANG_USE_SYSTEM_LZ4` (OFF)                                                                                                                             |
| `unordered_dense`                                  | Fast hash-map/set container used across the core and runtime.                                                                                                               | `SLANG_USE_SYSTEM_UNORDERED_DENSE` (OFF)                                                                                                                 |
| `fast_float`                                       | Fast, correct floating-point parsing for the compiler core. Header-only `INTERFACE` target.                                                                                 | `SLANG_OVERRIDE_FAST_FLOAT_PATH` (OFF)                                                                                                                   |
| `cmark`                                            | CommonMark / GitHub-Flavored-Markdown parser (swiftlang fork) used by Slang's Markdown/documentation handling.                                                              | Always built; `SLANG_OVERRIDE_CMARK_PATH` (OFF)                                                                                                          |
| `mimalloc`                                         | Microsoft allocator. One checkout is shared between Slang and SPIRV-Tools.                                                                                                  | `SLANG_ENABLE_MIMALLOC` and `SLANG_ENABLE_SPIRV_TOOLS_MIMALLOC` (platform default); `SLANG_OVERRIDE_MIMALLOC_PATH` (OFF)                                 |
| `optix-dev`                                        | NVIDIA OptiX SDK headers for ray-tracing on CUDA.                                                                                                                           | `SLANG_ENABLE_OPTIX` (AUTO — requires `SLANG_ENABLE_CUDA`)                                                                                               |
| `WindowsToolchain`                                 | CMake toolchain helper files for Windows. Build tooling only, not a linked library.                                                                                         | — (build tooling)                                                                                                                                        |
| DXC _(prebuilt binary or source + `dxc/` headers)_ | DirectX Shader Compiler for DXIL code generation. `dxc/` holds the API headers; the compiler is a prebuilt download on most configurations and built from source in others. | `SLANG_ENABLE_DXIL` (ON); `SLANG_DXC_BUILD_FROM_SOURCE` and `SLANG_DXC_BINARY_URL` control how DXC is obtained (see `cmake/FetchDXC.cmake`)              |
| `slang-tint` / `webgpu_dawn` _(fetched binaries)_  | WGSL / WebGPU support.                                                                                                                                                      | `SLANG_EXCLUDE_TINT` (OFF), `SLANG_EXCLUDE_DAWN` (ON off-Windows, OFF on Windows)                                                                        |
| `slang-llvm` / LLVM _(fetched or system)_          | LLVM-based host/CPU code generation.                                                                                                                                        | `SLANG_SLANG_LLVM_FLAVOR` (`FETCH_BINARY_IF_POSSIBLE`)                                                                                                   |

Vendored headers not in the table above are used directly by their consumers
and have no dedicated option: `stb/` (image I/O for examples), `spirv/`
(`spirv.h`), `slang-tint-headers/`, `glext.h` / `wglext.h` (OpenGL extension
headers), and `renderdoc_app.h` (the RenderDoc in-application API).

## Build-wide option families

Several dependencies are controlled by the same families of options rather than
a bespoke switch:

- **`SLANG_ENABLE_*`** — turn a feature and its dependency on or off, e.g.
  `SLANG_ENABLE_SLANG_GLSLANG`, `SLANG_ENABLE_SLANG_RHI`, `SLANG_ENABLE_DXIL`,
  and `SLANG_ENABLE_OPTIX`. Some are plain booleans; the CUDA/OptiX/NVAPI family
  defaults to `AUTO` (enabled when the corresponding SDK is found).
- **`SLANG_USE_SYSTEM_*`** — use a system-installed package via `find_package`
  instead of the bundled submodule. Defined for `MINIZ`, `LZ4`,
  `VULKAN_HEADERS`, `SPIRV_HEADERS`, `UNORDERED_DENSE`, `SPIRV_TOOLS`, and
  `GLSLANG`. All default **OFF** (use the bundled submodule).
- **`SLANG_OVERRIDE_*_PATH`** — build the dependency from a source checkout at a
  path you supply instead of the in-tree submodule. Defined for `LZ4`, `MINIZ`,
  `UNORDERED_DENSE`, `VULKAN_HEADERS`, `SPIRV_HEADERS`, `SPIRV_TOOLS`,
  `GLSLANG`, `GLM`, `IMGUI`, `SLANG_RHI`, `TINYOBJLOADER`, `LUA`, `MIMALLOC`,
  `CMARK`, and `FAST_FLOAT`. All default **OFF**.
- **`SLANG_SLANG_LLVM_FLAVOR`** — how the LLVM-backed `slang-llvm` library is
  obtained: `FETCH_BINARY`, `FETCH_BINARY_IF_POSSIBLE` (default),
  `USE_SYSTEM_LLVM`, or `DISABLE`. A custom download location can be given with
  `SLANG_SLANG_LLVM_BINARY_URL`.

## Submodule pin policy

A few submodules carry non-default settings in [`.gitmodules`](../.gitmodules):

- `spirv-tools` sets `slang-skip-pin-check = true`. Slang routinely pins to a
  SPIRV-Tools fix that is upstreamed as a PR but not yet merged to the tracked
  branch, so the branch-reachability check is skipped (the SHA is still verified
  to be fetchable from the official Khronos URL).
- `lua` tracks the `v5.4` maintenance branch; `cmark` tracks the `gfm` branch;
  `fast_float` tracks the `v8.2.7` branch. These `branch=` overrides are
  required because the pinned commit is not reachable from the remote's default
  branch — reachability is still enforced, just against the named branch.
- `miniz` sets `ignore = untracked` so build-generated files in that checkout
  do not show up as local modifications.
