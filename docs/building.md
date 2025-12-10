# Building Slang From Source

### TLDR

`cmake --workflow --preset release` to configure, build, and package a release
version of Slang.

## Prerequisites:

Please install:

- CMake (3.26 preferred, but 3.22 works[^1])
- A C++ compiler with support for C++17. GCC, Clang and MSVC are supported
- A CMake compatible backend, for example Visual Studio or Ninja
- Python3 (a dependency for building spirv-tools)

Optional dependencies for tests include

- CUDA
- OptiX
- NVAPI
- Aftermath
- X11

Other dependencies are sourced from submodules in the [./external](./external)
directory.

## Get the Source Code

Clone [this](https://github.com/shader-slang/slang) repository. Make sure to
fetch the submodules also.

```bash
git clone https://github.com/shader-slang/slang --recursive
```

You will need the git tags from this repository, otherwise versioning
information (including the Slang modules directory name and the library
filenames on macOS and Linux) will be incorrect. The above command should fetch
them for you, but if you're fetching from a fork you may need to explicitly
fetch the latest tags from the shader-slang repository with:

```bash
git fetch https://github.com/shader-slang/slang.git 'refs/tags/*:refs/tags/*'
```

## Configure and build

> This section assumes cmake 3.25 or greater, if you're on a lower version
> please see [building with an older cmake](#building-with-an-older-cmake)

For a Ninja based build system (all platforms) run:
```bash
cmake --preset default
cmake --build --preset releaseWithDebugInfo # or --preset debug, or --preset release
```

For Visual Studio run:
```bash
cmake --preset vs2022 # or 'vs2019' or 'vs2026'
start devenv ./build/slang.sln # to optionally open the project in Visual Studio
cmake --build --preset releaseWithDebugInfo # to build from the CLI, could also use --preset release or --preset debug
```

There are also `*-dev` variants like `vs2022-dev` and `vs2026-dev` which turn on features to aid
debugging.

### WebAssembly build

In order to build WebAssembly build of Slang, Slang needs to be compiled with
[Emscripten SDK](https://github.com/emscripten-core/emsdk). You can find more
information about [Emscripten](https://emscripten.org/).

You need to clone the EMSDK repo. And you need to install and activate the latest.


```bash
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
```

For non-Windows platforms
```bash
./emsdk install latest
./emsdk activate latest
```

For Windows
```cmd
emsdk.bat install latest
emsdk.bat activate latest
```

After EMSDK is activated, Slang needs to be built in a cross compiling setup: 

- build the `generators` target for the build platform
- configure the build with `emcmake` for the host platform
- build for the host platform.

> Note: For more details on cross compiling please refer to the 
> [cross-compiling](docs/building.md#cross-compiling) section.

```bash
# Build generators.
cmake --workflow --preset generators --fresh
mkdir generators
cmake --install build --prefix generators --component generators

# Configure the build with emcmake.
# emcmake is available only when emsdk_env setup the environment correctly.
pushd ../emsdk
source ./emsdk_env # For Windows, emsdk_env.bat
popd
emcmake cmake -DSLANG_GENERATORS_PATH=generators/bin --preset emscripten -G "Ninja"

# Build slang-wasm.js and slang-wasm.wasm in build.em/Release/bin
cmake --build --preset emscripten --target slang-wasm
```

> Note: If the last build step fails, try running the command that `emcmake`
> outputs, directly.

## Installing

Build targets may be installed using cmake:

```bash
cmake --build . --target install
```

This should install `SlangConfig.cmake` that should allow `find_package` to work.
SlangConfig.cmake defines `SLANG_EXECUTABLE` variable that will point to `slangc`
executable and also define `slang::slang` target to be linked to.

For now, `slang::slang` is the only exported target defined in the config which can
be linked to.

Example usage

```cmake
find_package(slang REQUIRED PATHS ${your_cmake_install_prefix_path} NO_DEFAULT_PATH)
# slang_FOUND should be automatically set
target_link_libraries(yourLib PUBLIC
  slang::slang
)
```

## Testing

```bash
build/Debug/bin/slang-test
```

See the [documentation on testing](../tools/slang-test/README.md) for more information.

## Debugging

See the [documentation on debugging](/docs/debugging.md).

## Distributing

### Versioned Libraries

As of v2025.21, the Slang libraries on **Mac** and **Linux** use versioned
filenames. The public ABI for Slang libraries in general is not currently
stable, so in accordance with semantic versioning conventions, the major
version number for dynamically linkable libraries is currently 0. Due to the
unstable ABI, releases are designed so that downstream users will be linked
against the fully versioned library filenames (e.g.,
`libslang-compiler.so.0.2025.21` instead of `libslang-compiler.so`).

Slang libraries for **Windows** do not have an explicit version in the
library filename, but the the same guidance about stability of the ABI applies.

Downstream users of Slang distributing their products as binaries should
therefor **on all platforms, including Windows** redistribute the Slang
libraries they linked against, or otherwise communicate the specific version
dependency to their users. It is *not the case* that a user of your product can
just install any recent Slang release and have an installation of Slang that
works for any given binary.

## More niche topics

### CMake options

| Option                            | Default                    | Description                                                                                  |
|-----------------------------------|----------------------------|----------------------------------------------------------------------------------------------|
| `SLANG_VERSION`                   | Latest `v*` tag            | The project version, detected using git if available                                         |
| `SLANG_EMBED_CORE_MODULE`         | `TRUE`                     | Build slang with an embedded version of the core module                                      |
| `SLANG_EMBED_CORE_MODULE_SOURCE`  | `TRUE`                     | Embed the core module source in the binary                                                   |
| `SLANG_ENABLE_DXIL`               | `TRUE`                     | Enable generating DXIL using DXC                                                             |
| `SLANG_ENABLE_ASAN`               | `FALSE`                    | Enable ASAN (address sanitizer)                                                              |
| `SLANG_ENABLE_COVERAGE`           | `FALSE`                    | Enable code coverage instrumentation                                                         |
| `SLANG_ENABLE_FULL_IR_VALIDATION` | `FALSE`                    | Enable full IR validation (SLOW!)                                                            |
| `SLANG_ENABLE_IR_BREAK_ALLOC`     | `FALSE`                    | Enable IR BreakAlloc functionality for debugging.                                            |
| `SLANG_ENABLE_GFX`                | `TRUE`                     | Enable gfx targets                                                                           |
| `SLANG_ENABLE_SLANGD`             | `TRUE`                     | Enable language server target                                                                |
| `SLANG_ENABLE_SLANGC`             | `TRUE`                     | Enable standalone compiler target                                                            |
| `SLANG_ENABLE_SLANGI`             | `TRUE`                     | Enable Slang interpreter target                                                              |
| `SLANG_ENABLE_SLANGRT`            | `TRUE`                     | Enable runtime target                                                                        |
| `SLANG_ENABLE_SLANG_GLSLANG`      | `TRUE`                     | Enable glslang dependency and slang-glslang wrapper target                                   |
| `SLANG_ENABLE_TESTS`              | `TRUE`                     | Enable test targets, requires SLANG_ENABLE_GFX, SLANG_ENABLE_SLANGD and SLANG_ENABLE_SLANGRT |
| `SLANG_ENABLE_EXAMPLES`           | `TRUE`                     | Enable example targets, requires SLANG_ENABLE_GFX                                            |
| `SLANG_LIB_TYPE`                  | `SHARED`                   | How to build the slang library                                                               |
| `SLANG_ENABLE_RELEASE_DEBUG_INFO` | `TRUE`                     | Enable generating debug info for Release configs                                             |
| `SLANG_ENABLE_RELEASE_LTO`        | `FALSE`                    | Enable LTO for Release builds                                                                |
| `SLANG_ENABLE_SPLIT_DEBUG_INFO`   | `TRUE`                     | Enable generating split debug info for Debug and RelWithDebInfo configs                      |
| `SLANG_SLANG_LLVM_FLAVOR`         | `FETCH_BINARY_IF_POSSIBLE` | How to set up llvm support                                                                   |
| `SLANG_SLANG_LLVM_BINARY_URL`     | System dependent           | URL specifying the location of the slang-llvm prebuilt library                               |
| `SLANG_GENERATORS_PATH`           | ``                         | Path to an installed `all-generators` target for cross compilation                           |

The following options relate to optional dependencies for additional backends
and running additional tests. Left unchanged they are auto detected, however
they can be set to `OFF` to prevent their usage, or set to `ON` to make it an
error if they can't be found.

| Option                   | CMake hints                    | Notes                                                                                        |
|--------------------------|--------------------------------|----------------------------------------------------------------------------------------------|
| `SLANG_ENABLE_CUDA`      | `CUDAToolkit_ROOT` `CUDA_PATH` | Enable running tests with the CUDA backend, doesn't affect the targets Slang itself supports |
| `SLANG_ENABLE_OPTIX`     | `Optix_ROOT_DIR`               | Requires CUDA                                                                                |
| `SLANG_ENABLE_NVAPI`     | `NVAPI_ROOT_DIR`               | Only available for builds targeting Windows                                                  |
| `SLANG_ENABLE_AFTERMATH` | `Aftermath_ROOT_DIR`           | Enable Aftermath in GFX, and add aftermath crash example to project                          |
| `SLANG_ENABLE_XLIB`      |                                |                                                                                              |

### Advanced options

| Option                             | Default | Description                                                                                                                    |
|------------------------------------|---------|--------------------------------------------------------------------------------------------------------------------------------|
| `SLANG_ENABLE_DX_ON_VK`            | `FALSE` | Enable running the DX11 and DX12 tests on non-warning Windows platforms via vkd3d-proton, requires system-provided d3d headers |
| `SLANG_ENABLE_SLANG_RHI`           | `TRUE`  | Enable building and using [slang-rhi](https://github.com/shader-slang/slang-rhi) for tests                                     |
| `SLANG_USE_SYSTEM_MINIZ`           | `FALSE` | Build using system Miniz library instead of the bundled version in [./external](./external)                                    |
| `SLANG_USE_SYSTEM_LZ4`             | `FALSE` | Build using system LZ4 library instead of the bundled version in [./external](./external)                                      |
| `SLANG_USE_SYSTEM_VULKAN_HEADERS`  | `FALSE` | Build using system Vulkan headers instead of the bundled version in [./external](./external)                                   |
| `SLANG_USE_SYSTEM_SPIRV_HEADERS`   | `FALSE` | Build using system SPIR-V headers instead of the bundled version in [./external](./external)                                   |
| `SLANG_USE_SYSTEM_UNORDERED_DENSE` | `FALSE` | Build using system unordered dense instead of the bundled version in [./external](./external)                                  |
| `SLANG_SPIRV_HEADERS_INCLUDE_DIR`  | ``      | Use this specific path to SPIR-V headers instead of the bundled version in [./external](./external)                            |

### LLVM Support

There are several options for getting llvm-support:

- Use a prebuilt binary slang-llvm library:
  `-DSLANG_SLANG_LLVM_FLAVOR=FETCH_BINARY` or `-DSLANG_SLANG_LLVM_FLAVOR=FETCH_BINARY_IF_POSSIBLE` (this is the default)
    - You can set `SLANG_SLANG_LLVM_BINARY_URL` to point to a local
      `libslang-llvm.so/slang-llvm.dll` or set it to a URL of an zip/archive
      containing such a file
    - If this isn't set then the build system tries to download it from the
      release on github matching the current tag. If such a tag doesn't exist
      or doesn't have the correct os\*arch combination then the latest release
      will be tried.
    - If `SLANG_SLANG_LLVM_BINARY_URL` is `FETCH_BINARY_IF_POSSIBLE` then in
      the case that a prebuilt binary can't be found then the build will proceed
      as though `DISABLE` was chosen
- Use a system supplied LLVM: `-DSLANG_SLANG_LLVM_FLAVOR=USE_SYSTEM_LLVM`, you
  must have llvm-21.1 and a matching libclang installed. It's important that
  either:
    - You don't end up linking to a dynamic libllvm.so, this will almost
      certainly cause multiple versions of LLVM to be loaded at runtime,
      leading to errors like `opt: CommandLine Error: Option
      'asm-macro-max-nesting-depth' registered more than once!`. Avoid this by
      compiling LLVM without the dynamic library.
    - Anything else which may be linked in (for example Mesa, also dynamically
      loads the same llvm object)
- Do not enable LLVM support: `-DSLANG_SLANG_LLVM_FLAVOR=DISABLE`

To build only a standalone slang-llvm, you can run:

```bash
cmake --workflow --preset slang-llvm
```

This will generate `build/dist-release/slang-slang-llvm.zip` containing the
library. This, of course, uses the system LLVM to build slang-llvm, otherwise
it would just be a convoluted way to download a prebuilt binary.

### Cross compiling

Slang generates some code at build time, using generators build from this
codebase. Due to this, for cross compilation one must already have built these
generators for the build platform. Build them with the `generators` preset, and
pass the install path to the cross building CMake invocation using
`SLANG_GENERATORS_PATH`

Non-Windows platforms:

```bash
# build the generators
cmake --workflow --preset generators --fresh
mkdir build-platform-generators
cmake --install build --config Release --prefix build-platform-generators --component generators
# reconfigure, pointing to these generators
# Here is also where you should set up any cross compiling environment
cmake \
  --preset default \
  --fresh \
  -DSLANG_GENERATORS_PATH=build-platform-generators/bin \
  -Dwhatever-other-necessary-options-for-your-cross-build \
  # for example \
  -DCMAKE_C_COMPILER=my-arch-gcc \
  -DCMAKE_CXX_COMPILER=my-arch-g++
# perform the final build
cmake --workflow --preset release
```

Windows

```bash
# build the generators
cmake --workflow --preset generators --fresh
mkdir build-platform-generators
cmake --install build --config Release --prefix build-platform-generators --component generators
# reconfigure, pointing to these generators
# Here is also where you should set up any cross compiling environment
# For example
./vcvarsamd64_arm64.bat
cmake \
  --preset default \
  --fresh \
  -DSLANG_GENERATORS_PATH=build-platform-generators/bin \
  -Dwhatever-other-necessary-options-for-your-cross-build
# perform the final build
cmake --workflow --preset release
```

### Example cross compiling with MSVC to windows-aarch64

One option is to build using the ninja generator, which requires providing the
native and cross environments via `vcvarsall.bat`

```bash
vcvarsall.bat
cmake --workflow --preset generators --fresh
mkdir generators
cmake --install build --prefix generators --component generators
vsvarsall.bat x64_arm64
cmake --preset default --fresh -DSLANG_GENERATORS_PATH=generators/bin
cmake --workflow --preset release
```

Another option is to build using the Visual Studio generator which can find
this automatically

```
cmake --preset vs2022 # or --preset vs2019, vs2026
cmake --build --preset generators # to build from the CLI
cmake --install build --prefix generators --component generators
rm -rf build # The Visual Studio generator will complain if this is left over from a previous build
cmake --preset vs2022 --fresh -A arm64 -DSLANG_GENERATORS_PATH=generators/bin
cmake --build --preset release
```

### Nix

This repository contains a [Nix](https://nixos.org/)
[flake](https://wiki.nixos.org/wiki/Flakes) (not officially supported or
tested), which provides the necessary prerequisites for local development. Also,
if you use [direnv](https://direnv.net/), you can run the following commands to
have the Nix environment automatically activate when you enter your clone of
this repository:

```bash
echo 'use flake' > .envrc
direnv allow
```

## Building with an older CMake

Because older CMake versions don't support all the features we want to use in
CMakePresets, you'll have to do without the presets. Something like the following

```bash
cmake -B build -G Ninja
cmake --build build -j
```

## Specific supported compiler versions

<!---
Please keep the exact formatting '_Foo_ xx.yy is tested in CI' as there is a
script which checks that this is still up to date.
-->

_GCC_ 11.4 and 13.3 are tested in CI and is the recommended minimum version. GCC 10 is
supported on a best-effort basis, i.e. PRs supporting this version are
encouraged but it isn't a continuously maintained setup.

_MSVC_ 19 is tested in CI and is the recommended minimum version.

_Clang_ 17.0 is tested in CI and is the recommended minimum version.

## Static linking against libslang-compiler

To build statically, set the `SLANG_LIB_TYPE` flag in CMake to `STATIC`.

If linking against a static `libslang-compiler.a` you will need to link against some
dependencies also if you're not already incorporating them into your project.

```
${SLANG_DIR}/build/Release/lib/libslang-compiler.a
${SLANG_DIR}/build/Release/lib/libcompiler-core.a
${SLANG_DIR}/build/Release/lib/libcore.a
${SLANG_DIR}/build/external/miniz/libminiz.a
${SLANG_DIR}/build/external/lz4/build/cmake/liblz4.a
```

## Deprecation of libslang and slang.dll filenames

In Slang v2025.21, the primary library for Slang was renamed, from
`libslang.so` and `slang.dll` to `libslang-compiler.so` and
`slang-compiler.dll`. (A similar change was made for macOS.) The reason behind
this change was to address a conflict on the Linux target, where the S-Lang
library of the same name is commonly preinstalled on Linux distributions. The
same issue affected macOS, to a lesser extent, where the S-Lang library could
be installed via `brew`. To make the Slang library name predictable and
simplify downstream build logic, the Slang library name was changed on all
platforms.

A change like this requires a period of transition, so on a **temporary**
basis: Linux and macOS packages now include symlinks from the old filename to
the new one. For Windows, a proxy library is provided with the old name, that
redirects all functions to the new `slang-compiler.dll`. The rationale here is
that applications with a complex dependency graph may have some components
still temporarily using `slang.dll`, while others have been updated to use
`slang-compiler.dll`. Using a proxy library for `slang.dll` ensures that all
components are using the same library, and avoids any potential state or
heap-related issues from an executable sharing data structures between the two
libraries.

These backwards compatability affordances, namely the proxy `slang.dll` and
`slang.lib` (for Windows) and the `libslang.so` and `libslang.dylib` symlinks
(for Linux and macOS), **will be removed at the end of 2026**. Until that time,
they will be present in the github release packages for downstream use.
Downstream packaging may or may not choose to distribute them, at their
discretion. **We strongly encourage downstream users of Slang to move to the
new library names as soon as they are able.**

## Notes

[^1] below 3.25, CMake lacks the ability to mark directories as being
system directories (https://cmake.org/cmake/help/latest/prop_tgt/SYSTEM.html#prop_tgt:SYSTEM),
this leads to an inability to suppress warnings originating in the
dependencies in `./external`, so be prepared for some additional warnings.
