# Slice 197: Reproduce the isolated NVVM provider build

## Motivation

Consider compiling this existing workload:

```sh
build/Debug/bin/slangc tests/cuda/nvvm-core-execution.slang \
  -entry computeMain -stage compute -target ptx -emit-cuda-via-nvvm \
  -capability cuda_sm_8_0 -O3 -o build/core.ptx
```

It needs a compiler-matched provider built against LLVM 14.0.6. Developers previously had to
reconstruct the LLVM configure flags and build two independent CMake projects manually. The new
entry point makes this dependency reproducible from a clean checkout and supports offline reuse.

## Proposed solution

`python3 extras/build-nvvm-provider.py --jobs 4` fetches the immutable LLVM 14.0.6 release commit,
builds the required static components, and builds this checkout's provider. `--llvm-source` accepts
an offline LLVM source directory; `--llvm-dir` accepts an existing isolated LLVM package. Separate
LLVM/provider CMake caches preserve isolation from Slang's other LLVM integration. The standalone
provider keeps owning exact-version, static-library, exception, and export constraints.

## Change summary

- `extras/build-nvvm-provider.py` selects platform tools, fetches pinned source when needed, applies
  dependency build settings, and produces a provider under `artifacts/<config>`.
- `source/slang-llvm-nvvm/README.md` documents prerequisites, native/WSL host selection, offline paths,
  configuration and recovery, artifact locations, and component installation.
- The completed ExecPlan and this report retain the requested slice record. Generated binaries and
  validation output remain under `build/`.

## Concepts and vocabulary

The **provider** constructs typed-pointer LLVM 14 bitcode through the versioned Slang C ABI. It is
compiler-matched: its source and ABI headers come from the same checkout as Slang. An **isolated
LLVM package** exposes static LLVM 14.0.6 CMake targets only to the independent provider project.
The **managed source checkout** is the script-owned clean Git checkout at the immutable release
commit; an explicit source override remains developer-owned.

## Process report

The shader above reaches `NVVMIRBuilder::load`, then the provider's C ABI, then LLVM bitcode
construction and libNVVM. This slice changes how that provider is produced, not the compiler's
semantic representation. No helper rebuilds checked AST/IR data, changes equivalence, or adds a
fallback around malformed compiler input.

The helper inventory is `run`, `HostTools`, `fetch_llvm`, `configure`, `build`, and `main`. Each
survives as build orchestration: subprocess failure propagation, platform-specific paths/tool
selection, immutable source acquisition, independent CMake configuration, bounded target builds,
and argument coordination respectively. The unusual input shape is WSL paths consumed by Windows
executables; that shape is intentional, and `HostTools.path` owns its conversion. Missing Windows
tools fail explicitly. The script accepts `--host linux` only as an explicit WSL choice rather than
switching host after a failure.

LLVM's producer configuration uses static PIC libraries, disables exceptions/RTTI and optional
system dependencies, and builds the dependency closure of Core, BitWriter, and Support. It does not
build a shared LLVM library or import LLVM targets into root Slang CMake. Provider configuration
receives this checkout as `SLANG_SOURCE_DIR` and keeps CMake's cached `LLVM_DIR` aligned with the
explicit package selection on reruns. Otherwise changing `--llvm-dir` could retain the previous
cached package. Existing standalone checks remain authoritative instead of duplicating their logic
in Python. Managed Git source is checked for the exact commit and cleanliness; reruns never reset
a modified checkout. An interrupted fetch without HEAD may retry safely.

Validation on native Linux:

- A real network fetch checked out `f28c006a5895fc0e329fe15fead81e37457cb1d1`; repeated source
  verification succeeded (`build/nvvm-slice197-fetch.log`).
- A fresh provider build using `--llvm-dir build/llvm14/lib/cmake/llvm` and an incremental rerun
  succeeded (`build/nvvm-slice197-package.log`).
- Help, zero jobs, mutually exclusive inputs, missing package/source paths, and unsupported host
  selection returned their expected exit statuses (`build/nvvm-slice197-checks.log`).
- The resulting module exports exactly `slang_getNVVMBuilderAPI` and has no shared LLVM dependency.
  The cached-package path retains the cached dependency's zlib/terminfo requirements; the fresh
  source path explicitly disables these optional libraries.
- Pointing `SLANG_NVVM_BUILDER_PATH` at the new artifact directory compiled the motivating shader to
  PTX at O3; CUDA 13.4 `ptxas -arch=sm_80` assembled it successfully.

A fresh LLVM source build completed all 667 dependency steps and built the provider successfully
(`build/nvvm-slice197-source.log`). Its module exports only the intended ABI, has no shared LLVM,
zlib, or terminfo dependency, and passed O3 PTX compilation and sm_80 assembly
(`build/nvvm-slice197-source-artifact.log`). No native Windows,
macOS, or Windows-hosted WSL execution is claimed. GPU execution is unavailable on this host.

The validation also exposed a pre-existing Linux deployment issue: the documented exact-file
`SLANG_NVVM_BUILDER_PATH=/.../libslang-llvm-nvvm.so` fails E52016 while its directory form succeeds.
The build artifact is valid; `ldd -r` reports no unresolved symbols. Slice 198 owns loader/package
integration and will investigate this boundary. Slice 197 does not add a build-layer workaround.
