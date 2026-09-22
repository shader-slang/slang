# LLVM 14 NVVM provider

`slang-llvm-nvvm` is the optional IR-construction provider used by Slang's direct libNVVM compute
backend. It isolates LLVM 14.0.6 from Slang's other LLVM integrations and exports only the C ABI in
`source/compiler-core/slang-nvvm-ir-builder-api.h`.

## Build and install

From a clean Slang checkout, build the provider and its isolated dependency with:

```sh
python3 extras/build-nvvm-provider.py --jobs 4
```

The entry point fetches LLVM 14.0.6 at the immutable commit
`f28c006a5895fc0e329fe15fead81e37457cb1d1`, builds only the required static LLVM components, and
builds the provider from this Slang checkout. LLVM uses PIC, no C++ exceptions or RTTI, and no
optional compression, terminal, XML, or HTTP libraries. The LLVM and provider CMake projects have
independent caches so they cannot conflict with Slang's other LLVM integration. The default work
directory is `build/nvvm-provider`; the final module is printed and lives under `artifacts/Release`.
No CUDA Toolkit or GPU is required to build the provider.

The prerequisites are Python 3, CMake, Git, a C++ compiler, and Ninja on Linux/macOS. On Windows use
`python` and install Visual Studio 2026 with C++ tools; the default generator is
`Visual Studio 18 2026`. Select another installed generator with `--generator`, for example
`--generator "Visual Studio 17 2022"`. WSL uses `cmake.exe` and `git.exe` with converted Windows
paths by default. Use `--host linux` explicitly for a Linux build in WSL. Missing Windows tools
produce an error rather than selecting Linux tools automatically. Native Windows/macOS and WSL
paths have not yet been validated by the Linux slice-197 run; macOS cannot execute the CUDA backend.

For offline development, reuse a source tree or an existing isolated LLVM CMake package:

```sh
python3 extras/build-nvvm-provider.py --llvm-source /path/to/llvm-project/llvm --jobs 4
python3 extras/build-nvvm-provider.py --llvm-dir /path/to/llvm14/lib/cmake/llvm --jobs 4
```

These options are mutually exclusive. Supplied source trees remain developer-owned; the resulting
LLVM package must still pass the provider's exact-version and static-library checks. Use
`--build-dir` for a separate work directory and `--config Debug` or `--config RelWithDebInfo` to
change configuration. On Windows, an existing LLVM package must have the matching static CRT
configuration (`MT` for Release/RelWithDebInfo, `MTd` for Debug). Reruns are incremental and stop at
the first error. Use a fresh build directory when changing host, generator, or toolchain; the script
does not delete or reset existing trees. A managed fetched checkout must remain clean and pinned.

To install the script-built provider, use its independent build directory:

```sh
cmake --install build/nvvm-provider/provider-build --config Release \
  --prefix /path/to/slang-install --component slang-llvm-nvvm
```

For direct CMake integration without the entry point, the equivalent provider-only commands follow.

Configure this directory as an independent CMake project. `SLANG_NVVM_LLVM_DIR` must name an LLVM
14.0.6 CMake package built as static component libraries, without C++ exceptions.

```text
cmake -S source/slang-llvm-nvvm -B build/slang-llvm-nvvm \
  -DSLANG_NVVM_LLVM_DIR=<llvm-14-prefix>/lib/cmake/llvm
cmake --build build/slang-llvm-nvvm --config Release
cmake --install build/slang-llvm-nvvm --config Release \
  --prefix <slang-install-prefix> --component slang-llvm-nvvm
```

The install component contains one platform module under `<slang-install-prefix>/bin`. Install it
into the same prefix as the Slang executables. It intentionally has no process-visible LLVM shared
library dependency.

## Discovery and updates

With no override, a global Slang session resolves the provider from the running executable's
directory. Set `SLANG_NVVM_BUILDER_PATH` to an alternate provider directory or exact module file for
development and custom deployment. A configured path is authoritative; Slang does not continue to
PATH or silently fall back to NVRTC when it is missing or incompatible. Diagnostic E52016 reports
the location that was searched.

The provider is compiler-matched and forward-only. Build it from the same Slang checkout as the
compiler, and deploy the compiler and provider together. The compiler requires the exact ABI
revision declared by `SLANG_NVVM_BUILDER_ABI_REVISION`; there is no compatibility promise between
revisions. Replace the files as one package update rather than mixing a new compiler with an older
provider. Each global session caches its first provider load result, so a process must create a new
global session after an on-disk update.
