# LLVM 14 NVVM provider

`slang-llvm-nvvm` is the optional IR-construction provider used by Slang's direct libNVVM compute
backend. It isolates LLVM 14.0.6 from Slang's other LLVM integrations and exports only the C ABI in
`source/compiler-core/slang-nvvm-ir-builder-api.h`.

## Build and install

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
