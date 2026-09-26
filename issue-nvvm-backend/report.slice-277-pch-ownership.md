# Isolate automatic NVRTC PCH files by compiler owner

## Motivation

Two independent Slang processes can compile unnamed CUDA sources while sharing a working directory.
Automatic NVRTC PCH then uses the same `default_program.pch`. Research275 traced independent owners
replacing/truncating that file:16 shared processes produced8 reuse failures and2 crashes, while24
serial/private-directory controls passed. The related262 deletion-error history remains preserved.

## Proposed solution

Give each `NVRTCDownstreamCompiler` one lazily acquired private directory for its default automatic
PCH files. Preserve source names and explicit caller directory options. Repeated calls through the
same compiler reuse their PCH; independently created compiler objects get separate default caches.
Release the owner's library reference before removing its directory, allowing NVRTC's normal final
cleanup first when possible. Surviving external library references remain supported by lifecycle tests.

## Change summary

- `source/core/slang-io.{h,cpp}` adds exclusive temporary-directory acquisition: Unix `mkdtemp` with
  private permissions; Windows reserves a unique file while acquiring an adjacent directory.
- `source/compiler-core/slang-nvrtc-compiler.cpp` owns the default namespace, preserves caller options,
  diagnoses acquisition failure and removes only its acquired directory at destruction.
- `tools/slang-unit-test/unit-test-io.cpp` checks distinct directory ownership and cleanup.
  `unit-test-nvrtc-pch-invalidation.cpp` adds actual-adapter lifetime and caller-directory tests while
  preserving the existing unnamed creation/reuse/invalidation and no-include checks.
- The completed plan, compact validation record and navigation retain exact acceptance and failures;
  raw binaries, traces, logs and earlier candidates stay under `build/nvvm-pch-ownership277`.

## Concepts and vocabulary

An automatic PCH stores the leading header region. A compiler owner is a downstream compiler object,
which can serve multiple calls; it is not necessarily the last reference to the NVRTC library.
Caller-selected directories remain caller-owned. A `not-created` status establishes reuse only after
proven creation for the same key, not by itself.

## Process report

Consider two processes compiling this generated CUDA source with an empty artifact name:

```cpp
#include "slang-cuda-prelude.h"
extern "C" __global__ void computeMain() {}
```

`emitEntryPointsSourceFromIR` creates an intentionally unnamed blob artifact. `ArtifactUtil::findPath`
returns its empty path and `NVRTCDownstreamCompiler::compile` preserves that program identity when
calling `nvrtcCreateProgram`. The valid leading include enables automatic PCH. The downstream adapter
owns the filesystem policy; `_addAutomaticPchOptions` now supplies a stable private `--pch-dir` without
changing source representation, filenames in diagnostics or the existing PCH eligibility gate.

The new helper inventory is exclusive directory acquisition, default-option/namespace ownership and
destructor cleanup; all remain at their owning layer. The initializer mutex protects only acquisition,
not compilation. An explicit long/short directory option bypasses ownership, including malformed forms
whose original diagnostics must reach the caller. Failure to acquire the default directory fails the
compile with a useful diagnostic. Unix acquisition is atomic. Windows retains its file reservation
while acquiring a distinct adjacent directory and never accepts an already-existing directory.

Each `ScopeProgram` destroys its program before the compile returns. The compiler destructor releases
its own NVRTC reference, then removes only its directory. `ScopeSharedLibrary` provides the existing
unload-before-resource-cleanup ordering precedent. Research276 qualified20 direct-library processes
and100 compiles, including retirement while other references survive. New actual-adapter tests prove
one compiler's destruction preserves another's reuse, caller directories/sentinels survive and named
source errors retain their filename. Separate compiler objects intentionally lose implicit shared-file
reuse; callers can select an explicitly managed shared directory. No compile-speed claim is made.

Focused qualification passes5 native tests with zero skips. The fixed275 comparison now passes all40
processes:8 serial,16 shared cwd and16 private cwd. Every process creates3 PCH files and passes the
native reuse/invalidation assertions. File traces prove40 distinct directories, successful private
creation/removal and overlapping PCH activity in all16 concurrent rounds. Root checks all320 raw hashes
and20 round overlap obligations; the worker separately confirms permissions, cleanup and loaded bytes.

The full checkpoint preserves all 1,740 main outcomes and 576 input hashes against270: 1,703 correct,
37 unresolved and20 retained resolved histories. All 1,100 old unit identities are preserved, with
three new passing tests: 1,090 pass/13 skip. All 1,248 semantic identities are preserved:
1,170 pass/78 skip. Runtime4, toolkit18 and all six material compile/assembly cells pass. Material
PTX, cubins and reported resources match270 exactly. Runner contracts pass: census3, discovery7,
complex14 plus one skip, results16. No corpus additions or outcome transitions require reclassification.

[Validation277](runtime-validation.slice-277.json) records source9210ef5a1 plus patch30e61108,
compiler version `2026.18.3-293-g9210ef5a1`, actual compiler SHA256 `aa1fe42e…`, unchanged provider
ABI42/hash `fbef1a9e…`, all37 runtime artifacts and22 dependency pins. The exact final source and
artifact checks pass after every gate. Full277 becomes the accepted baseline; targeted233 remains
historical and implementation cadence resets to zero. The authorized local development loop continues.

Earlier attempts are retained: the first build exposed two new test-helper API mistakes, corrected
using the existing cast helper and standard blob output convention. The first focused run passed4/5;
its new caller test incorrectly assumed separate option/value arguments were accepted. A direct
NVRTC12.9.86 probe rejected both separate spellings and accepted both equals spellings. The corrected
test exercises documented equals forms and malformed missing-value diagnostics; production performs
no argument normalization or version-specific workaround.

Root performs separate local audits and the worker self-reviews; thread limits prevented fresh-worker
and independent-review delegation. Default acquisition-error reporting and the Windows acquisition
branch are source-audited only. Same-compiler concurrent calls gain no new general thread-safety
promise. The vendor PCH-path message parser deliberately fails tests if its format changes. The exact
262 deletion diagnostic was not separately reproduced; the demonstrated shared-file ownership failure
is the directly qualified correction. Material GPU binding/input/output contracts remain unavailable.
