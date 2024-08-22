# Building Slang From Source

### TLDR

`cmake --workflow --preset release` to configure, build, and package a release
version of Slang.

## Prerequisites:

Please install:

- CMake
- A C++ compiler with support for C++17. GCC, Clang and MSVC are supported
- A CMake compatible backend, for example Visual Studio or Ninja

Optional dependencies include

- CUDA
- OptiX
- NVAPI
- Aftermath
- X11

## Get the Source Code

Clone [this](https://github.com/shader-slang/slang) repository. Make sure to
fetch the submodules also.

```bash
git clone https://github.com/shader-slang/slang --recursive
```

## Configure and build

For a Ninja based build system (all platforms) run:
```bash
cmake --preset default
cmake --build --preset release # or --preset debug
```

For Visual Studio run:
```bash
cmake --preset vs2022 # or 'vs2019' or `vs2022-dev`
start devenv ./build/slang.sln # to optionally open the project in Visual Studio
cmake --build --preset release # to build from the CLI
```

The `vs2022-dev` preset turns on features that makes debugging easy.

## Testing

```bash
build/Debug/bin/slang-test
```

See the [documentation on testing](../tools/slang-test/README.md) for more information.

## More niche topics

### CMake options

| Option                            | Default          | Description                                                        |
|-----------------------------------|------------------|--------------------------------------------------------------------|
| `SLANG_VERSION`                   | Latest `v*` tag  | The project version, detected using git if available               |
| `SLANG_EMBED_STDLIB`              | `FALSE`          | Build slang with an embedded version of the stdlib                 |
| `SLANG_EMBED_STDLIB_SOURCE`       | `TRUE`           | Embed stdlib source in the binary                                  |
| `SLANG_ENABLE_ASAN`               | `FALSE`          | Enable ASAN (address sanitizer)                                    |
| `SLANG_ENABLE_FULL_IR_VALIDATION` | `FALSE`          | Enable full IR validation (SLOW!)                                  |
| `SLANG_ENABLE_IR_BREAK_ALLOC`     | `FALSE`          | Enable IR BreakAlloc functionality for debugging.                  |
| `SLANG_ENABLE_GFX`                | `TRUE`           | Enable gfx targets                                                 |
| `SLANG_ENABLE_SLANGD`             | `TRUE`           | Enable language server target                                      |
| `SLANG_ENABLE_SLANGC`             | `TRUE`           | Enable standalone compiler target                                  |
| `SLANG_ENABLE_SLANGRT`            | `TRUE`           | Enable runtime target                                              |
| `SLANG_ENABLE_SLANG_GLSLANG`      | `TRUE`           | Enable glslang dependency and slang-glslang wrapper target         |
| `SLANG_ENABLE_TESTS`              | `TRUE`           | Enable test targets, requires SLANG_ENABLE_GFX, SLANG_ENABLE_SLANGD and SLANG_ENABLE_SLANGRT |
| `SLANG_ENABLE_EXAMPLES`           | `TRUE`           | Enable example targets, requires SLANG_ENABLE_GFX                  |
| `SLANG_LIB_TYPE`                  | `SHARED`         | How to build the slang library                                     |
| `SLANG_SLANG_LLVM_FLAVOR`         | `FETCH_BINARY`   | How to set up llvm support                                         |
| `SLANG_SLANG_LLVM_BINARY_URL`     | System dependent | URL specifying the location of the slang-llvm prebuilt library     |
| `SLANG_GENERATORS_PATH`           | ``               | Path to an installed `all-generators` target for cross compilation |

The following options relate to optional dependencies for additional backends
and running additional tests. Left unchanged they are auto detected, however
they can be set to `OFF` to prevent their usage, or set to `ON` to make it an
error if they can't be found.

| Option                   | CMake hints                    | Notes                                                               |
|--------------------------|--------------------------------|---------------------------------------------------------------------|
| `SLANG_ENABLE_CUDA`      | `CUDAToolkit_ROOT` `CUDA_PATH` |                                                                     |
| `SLANG_ENABLE_OPTIX`     | `Optix_ROOT_DIR`               | Requires CUDA                                                       |
| `SLANG_ENABLE_NVAPI`     | `NVAPI_ROOT_DIR`               | Only available for builds targeting Windows                         |
| `SLANG_ENABLE_AFTERMATH` | `Aftermath_ROOT_DIR`           | Enable Aftermath in GFX, and add aftermath crash example to project |
| `SLANG_ENABLE_XLIB`      |                                |                                                                     |

### LLVM Support

There are several options for getting llvm-support:

- Use a prebuilt binary slang-llvm library: `-DSLANG_SLANG_LLVM_FLAVOR=FETCH_BINARY`,
  this is the default
    - You can set `SLANG_SLANG_LLVM_BINARY_URL` to point to a local
      `libslang-llvm.so/slang-llvm.dll` or set it to a URL of an zip/archive
      containing such a file
    - If this isn't set then the build system tries to download it from the
      release on github matching the current tag. If such a tag doesn't exist
      or doesn't have the correct os*arch combination then the latest release
      will be tried.
- Use a system supplied LLVM: `-DSLANG_SLANG_LLVM_FLAVOR=USE_SYSTEM_LLVM`, you
  must have llvm-13.0 and a matching libclang installed. It's important that
  either:
    - You don't end up linking to a dynamic libllvm.so, this will almost
      certainly cause multiple versions of LLVM to be loaded at runtime,
      leading to errors like `opt: CommandLine Error: Option
      'asm-macro-max-nesting-depth' registered more than once!`. Avoid this by
      compiling LLVM without the dynamic library.
    - Anything else which may be linked in (for example Mesa, also dynamically
      loads the same llvm object)
- Have the Slang build system build LLVM:
  `-DSLANG_SLANG_LLVM_FLAVOR=BUILD_LLVM`, this will build LLVM binaries at
  configure time and use that. This is only intended to be used as part of the
  process of generating the portable binary slang-llvm library. This always
  builds a `Release` LLVM, so is unsuitable to use when building a `Debug`
  `slang-llvm` on Windows as the runtime libraries will be incompatible.
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

```bash
# build the generators
cmake --workflow --preset generators --fresh
mkdir my-build-platform-generators
cmake --install build --config Release --prefix my-build-platform-generators --component generators
# reconfigure, pointing to these generators
# Here is also where you should set up any cross compiling environment
cmake \
  --preset default \
  --fresh \
  -DSLANG_GENERATORS_PATH=my-build-platform-generators/bin \
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
cmake --preset vs2022 # or --preset vs2019
cmake --build --preset generators # to build from the CLI
cmake --install build --prefix generators --component generators
rm -rf build # The Visual Studio generator will complain if this is left over from a previous build
cmake --preset vs2022 --fresh -A arm64 -DSLANG_GENERATORS_PATH=generators/bin
cmake --build --preset release
```
