# Slice 197: Reproduce the isolated NVVM provider build

## Purpose and Observable Result

A developer can run `python3 extras/build-nvvm-provider.py` from a clean Slang checkout to fetch
pinned LLVM 14.0.6 source, build its required static components, and build this checkout's NVVM
provider. Offline developers can supply an existing source directory or LLVM CMake package.

## Progress

- [x] 2026-09-22: Inspected standalone provider constraints, Linux build, and build skill.
- [x] 2026-09-22: Recorded this plan before implementation.
- [x] 2026-09-22: Implemented the host-aware build entry and README.
- [x] 2026-09-22: Validated fresh source and cached-package builds, pinned fetch/rerun,
      argument failures, provider symbol/dependency isolation, and O3 PTX/ptxas on both artifacts.
- [x] 2026-09-22: Recorded outcomes, reviewed and formatted for the sequential slice commit.

## Surprises and Discoveries

The standalone provider already rejects wrong LLVM versions, shared components, and exception-enabled
LLVM. Root Slang configuration is intentionally separate because its other LLVM integration may use
a newer LLVM with conflicting CMake target names. Cached LLVM source HEAD is
`f28c006a5895fc0e329fe15fead81e37457cb1d1` (LLVM 14.0.6).

## Decision Log

- 2026-09-22: Use Python standard library orchestration for argument handling and native/WSL paths.
  Keep build invariants in the existing standalone provider CMake configuration.
- 2026-09-22: Fetch an immutable LLVM commit; source and package overrides are explicit offline paths.
  Source builds use separate LLVM/provider directories and build only the required components.
- 2026-09-22: Follow the user's explicit instruction to commit finished plans with each slice. This
  is a task-specific exception to the generic active-working-log policy in `.agent/PLANS.md`.

## Context and Current Pipeline

Consider compiling `tests/cuda/nvvm-core-execution.slang` to PTX. Slang obtains the compiler-matched
provider through the C ABI in `source/compiler-core/slang-nvvm-ir-builder-api.h`. The module builds
LLVM 14 bitcode for libNVVM. Today developers must reconstruct the LLVM configure flags and manually
configure `source/slang-llvm-nvvm`; the new entry makes those steps explicit and repeatable.
It does not change compiler representations or the provider ABI.

## Scope and Non-Goals

Own `extras/build-nvvm-provider.py` and the provider README. Do not alter root packaging, runtime
loader behavior, compiler semantics, CUDA installation, or GPU execution. Package integration is
slice 198. Do not silently switch Windows-hosted WSL work to Linux tools.

## Architecture and Invariants

The script locates Slang from its own path, selects native CMake/Git tools, resolves all build inputs,
and configures LLVM and the provider separately. LLVM components are static PIC, no EH/RTTI, and
omit unused optional system dependencies. The provider source and headers always come from this
checkout. Existing exact-version/static/EH validation remains authoritative. Reruns reuse only the
explicit build directory and fail on incompatible CMake caches instead of deleting them.

## Interfaces and Dependencies

CLI: `--build-dir`, mutually exclusive `--llvm-source` / `--llvm-dir`, `--config`, `--jobs`,
`--generator`, and `--host`. Python 3, Git for managed source fetching, CMake, a C++ toolchain, and
Ninja (unless another generator is selected) are required. CUDA is required only for end-to-end
validation. Windows-hosted WSL requires `cmake.exe`, `git.exe`, and `wslpath`.

## Milestones

1. Add script and README examples for clean, offline-source, and existing-package builds.
2. Build a fresh provider against `build/llvm14/lib/cmake/llvm`.
3. Build a fresh LLVM binary directory from `build/llvm14-source/llvm` with four jobs.
4. Inspect exports and shared dependencies; use the new module for PTX compilation and assembly.

## Validation and Acceptance

Run help, argument rejection checks, both build paths, and an incremental rerun. Inspect
`nm -D --defined-only` and `readelf -d`: only the intended provider ABI may be exported and no shared
LLVM dependency may appear. Point `SLANG_NVVM_BUILDER_PATH` at the new module and compile
`tests/cuda/nvvm-core-execution.slang -entry computeMain -stage compute -target ptx
-emit-cuda-via-nvvm -capability cuda_sm_8_0 -O3`, then assemble with CUDA 13 `ptxas -arch=sm_80`.
Native Linux results do not establish native Windows/macOS or WSL execution coverage.

## Failure and Recovery

Commands stop on their first nonzero exit. Fix the diagnostic and rerun; no build/source tree is
deleted. Select a fresh build directory for generator, platform, or toolchain changes. Download
failures leave the managed fetch directory retryable. Source/package overrides avoid network access.

## Artifacts and Hand-Off

Retain commands/logs and binaries under `build/nvvm-slice197-*`. Parent integration updates durable
design notes and commits this plan plus the five-part report. No generated output is committed.

## Outcomes and Retrospective

Both source and package reuse paths are complete and validated on native Linux. The source build
compiled 667 dependency steps with four jobs. Both providers exported the single intended ABI and
passed O3 NVVM-to-PTX compilation plus sm_80 assembly. The fresh build eliminated optional zlib
and terminfo dependencies. Real pinned Git fetch and source verification passed. Logs are retained
under `build/nvvm-slice197-*`. Native Windows/macOS/WSL behavior remains unvalidated. Parent owns
formatting and the sequential slice commit. A pre-existing Linux exact-file override failure was
reported to slice 198; directory-form discovery works and no build-layer workaround was added.
