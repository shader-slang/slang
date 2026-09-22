# Slice 198: Build and package the compiler-matched NVVM provider

## Motivation

Consider compiling the existing workload from an installed Slang package:

```sh
slangc tests/cuda/nvvm-core-execution.slang -entry computeMain -stage compute \
  -target ptx -emit-cuda-via-nvvm -capability cuda_sm_8_0 -O3 -o core.ptx
```

Previously the root build did not produce or package the optional provider, so users copied a
separately built module manually. Deployment testing also found two concrete failures: specifying
an existing `libslang-llvm-nvvm.so` file appended `.so` again, and a normal package built with
`SLANG_EMBED_CORE_MODULE=OFF` omitted the actual compiler library and its core cache. The latter
left `libslang.so` pointing at a missing library and prevented installed `slangc` from starting.

## Proposed solution

Opt-in root builds configure the existing standalone provider in an independent CMake process,
using an explicit isolated LLVM14 package and the same checkout/compiler/configuration as Slang.
They stage its module beside compiler/test executables and install it with the normal binary
package. Default library loading preserves existing platform files. The nonembedded compiler and
its runtime core cache now belong to the normal runtime install component.

## Change summary

- `cmake/NVVM.cmake` provides default-OFF `SLANG_ENABLE_NVVM_PROVIDER`, validates the isolated package
  input and native host, configures separate child caches per configuration, stages the module, and
  installs the `slang-llvm-nvvm` component under `bin`.
- Root `CMakeLists.txt` includes that integration after executable targets exist; `CMakePresets.json`
  includes the provider component in ordinary binary package presets.
- `source/slang/CMakeLists.txt` classifies the nonembedded runtime compiler and cache as runtime
  installation artifacts; the separate embedded-mode bootstrap library remains generator-only.
- `source/core/slang-shared-library.cpp` preserves an existing exact filename instead of applying
  platform naming twice. `nvvmIRBuilderLoadsExactProviderFile` exercises real provider loading and
  proves an invalid explicit file cannot fall back to a valid decorated sibling.
- The provider README documents opt-in root builds and packaging. This report and completed plan
  retain the requested slice record; generated artifacts stay under `build/`.

## Concepts and vocabulary

The **provider** is the compiler-matched module exporting the NVVM builder C ABI and statically
linking LLVM14. An **ExternalProject** configures another CMake project in a separate process/cache,
which prevents LLVM14 target names from colliding with the root compiler's LLVM21 integration.
The **nonembedded compiler** loads a separate core-module cache at runtime; although bootstrap
code also uses this compiler, its installation role is still the runtime compiler.

## Process report

The shader above reaches `NVVMIRBuilder::load`, whose provider source shares the compiler's ABI
header. `ExternalProject_Add(slang-nvvm-provider-build)` invokes the existing standalone provider
project without importing its LLVM14 targets into root CMake. The child still enforces exact
LLVM14.0.6 static components, exception settings, and export restrictions. Forwarded generator,
compiler, configuration and native toolchain settings preserve the intended binary platform.
Both `SLANG_NVVM_LLVM_DIR` and cached `LLVM_DIR` receive the explicit package, preventing a stale
child cache from retaining an earlier package on reruns.

Each configuration has its own child cache and module. A prototype demonstrated that
ExternalProject's initial directory creation does not expand `$<CONFIG>`, so a configuration-aware
make-directory step precedes child configuration. This is a valid configuration expression owned
by CMake orchestration, not malformed compiler data. `BUILD_ALWAYS` invokes the child's dependency
scanner on each targeted build; that scanner tracks provider sources and shared ABI headers.
`copy_if_different` stages the resulting module beside actual target output directories without
changing timestamps when the module is unchanged. Cross compilation is explicitly rejected in
this native-build slice. No dependency download or alternative provider selection occurs.

For an exact-file override such as `/opt/slang/bin/libslang-llvm-nvvm.so`,
`DownstreamCompilerUtil::_findPaths` intentionally produces that existing file path. Previously
`DefaultSharedLibraryLoader::loadSharedLibrary` called `SharedLibrary::load`, which decorated it
again and attempted `libslang-llvm-nvvm.so.so`. The filesystem path is already the source of truth:
the default loader now calls its existing `loadPlatformSharedLibrary` for that shape. A local bare
filename is prefixed with `./` so POSIX `dlopen` uses the selected local file. Invalid explicit files
fail; there is no retry against a differently named library. Undecorated library names still use
the existing naming path, and custom loader interfaces are unchanged. The real provider regression
would fail without this loader boundary change; fake loaders never exercised platform suffixing.

Normal installation exposed a separate producer-role mistake: in the nonembedded configuration,
`slang_add_target` created the actual `slang` runtime library with `INSTALL_COMPONENT generators`.
Its alias also served bootstrap tooling, but that consumer does not make the runtime compiler a
generator-only artifact. Removing that override preserves the existing public library installation
contract, and the required nonembedded core cache follows the same runtime component. The separate
bootstrap library in the embedded branch remains generator-only. This fixes the producer's
component assignment instead of adding generator artifacts to the ordinary package as a workaround.

The helper/special-case inventory is the optional isolated CMake pipeline, its explicit native-host
restriction and configuration-directory step, and the default loader's existing-file branch.
Each survives at its owning build or filesystem boundary for the reasons above. No compiler AST,
IR, witness, or semantic equivalence representation changes are introduced.

Validation on native Linux with CUDA Toolkit 13.4:

- Isolated Ninja Multi-Config Debug and Release builds produced separate provider modules beside
  custom executable directories. Ninja single-config with an unspecified build type also passed.
- Default-disabled configuration succeeded. Missing isolated package and cross-compilation requests
  failed with explicit diagnostics. An unchanged rebuild preserved the staged module timestamp,
  and child Ninja dependencies include `slang-nvvm-ir-builder-api.h`.
- Root enabled-provider Debug `slangc`/`slang-test` build passed. The new exact-file regression passed.
  Focused NVVM/routing/skip-reporting/shared-library gates passed **421 tests, 54 ignored**.
- Clean normal runtime and provider component installation succeeded after building the enabled
  `slangi`, dispatcher and gfx install targets. The unmodified `cpack --preset debug` produced an
  archive containing `slangc`, its compiler library, the nonembedded core cache and provider.
- Extracting that ordinary package to a new directory and compiling from `/tmp` with
  `LD_LIBRARY_PATH` unset passed adjacent, directory-override and exact-file-override loading at
  both O0 and O3. All six PTX files assembled with `ptxas -arch=sm_80`.
- Invalid and missing authoritative provider overrides returned E52016 despite a compatible adjacent
  provider in the relocated install. GPU execution remains unavailable; Windows/macOS builds and
  package execution are not claimed.

Evidence is retained in `build/slice198-prototype/`, especially `regression.log`,
`normal-package-relocation.log`, `validate-package.py`, root install/package logs and the isolated
prototype builds. The normal package is `build/build/slice198-normal-packages/slang.zip` and its
relocated extraction is `build/slice198-package-relocated/`.
