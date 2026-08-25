# Establish the NVVM Contract and Compile Handwritten IR Through Slang

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is a local working
log covered by the repository's `issue-*/plan.*.md` ignore rule and must not be committed. Distill
durable architecture into `docs/design/nvvm-backend.md` and implementation reasoning into the PR
description.

## Purpose and Observable Result

This slice establishes the compiler boundary on which the direct NVVM backend will be built. At
the end, Slang can discover and dynamically load a compatible libNVVM installation, report its
version, accept one handwritten NVVM IR 2.0 module through the downstream-compiler interface,
verify it, and produce a PTX artifact containing a visible empty kernel.

The shortest observable proof on the current Windows machine is:

1. build `slang-test` in Debug;
2. run the focused `nvvmDownstreamCompiler` unit test;
3. observe that the official empty-kernel-shaped IR verifies and the returned PTX contains
   `.visible .entry testEmpty`; and
4. observe that the real-toolkit smoke assembles the returned PTX with the matching toolkit's
   `ptxas.exe` for `sm_75`.

This slice does not lower Slang IR, alter `-target ptx`, or select the new compiler for ordinary
Slang compilation. It proves the external API and artifact boundary independently before that
larger integration begins.

## Progress

- [x] (2026-08-25 20:31Z) Mapped the current PTX-to-CUDA-source-to-NVRTC pipeline and the
  downstream compiler registration points.
- [x] (2026-08-25 20:31Z) Confirmed the agreed initial contract: CUDA 12+, NVVM IR 2.0, legacy
  LLVM-7 typed-pointer dialect, compute first, textual IR for bootstrap only, NVRTC unchanged.
- [x] (2026-08-25 20:31Z) Manually loaded the installed CUDA 12.2 libNVVM, observed the IR 1.11
  mismatch diagnostic, verified an IR 2.0 module, and emitted PTX 8.2 for `compute_75`.
- [x] (2026-08-25 20:31Z) Added the generic ExecPlan standard and durable NVVM design document.
- [x] (2026-08-25 21:16Z) Completed the Slice 0 audit of public enums, artifact descriptors,
  source-language behavior, loader discovery, diagnostics, tests, and version identity; corrected
  the plan before code changes.
- [ ] Add the public pass-through identity and the exact internal enum/name tables that consume it
  without renumbering an existing enum value.
- [ ] Add the dynamically loaded libNVVM ABI and downstream compiler implementation.
- [ ] Add coherent CUDA-toolkit discovery, retain the resolved library identity, and retain the
  selected toolkit root when it can be derived for later libdevice use.
- [ ] Add focused positive and negative unit tests that do not require a GPU.
- [ ] Build and run the focused test, assemble its PTX with `ptxas`, and run established downstream
  compiler/version regression tests.
- [ ] Perform the new-helper/special-case audit required by `AGENTS.md` and complete the outcomes
  and hand-off sections.

## Surprises and Discoveries

- Observation: the library under the installed CUDA `v12.2` directory reports NVVM 2.0 and NVVM IR
  2.0 with debug metadata 3.1. Directory naming alone is not a reliable IR-version query.
  Evidence: `nvvmVersion` returned `2.0`; `nvvmIRVersion` returned `2.0, 3.1`.

- Observation: an otherwise valid empty kernel carrying `!nvvmir.version = 1.11` fails before
  compilation.
  Evidence: `nvvmVerifyProgram` returned `NVVM_ERROR_IR_VERSION_MISMATCH` and the log stated that IR
  1.11 is incompatible with IR 2.0.

- Observation: changing only the module version metadata to 2.0 made the same LLVM-7-dialect text
  verify and compile for a pre-Blackwell architecture.
  Evidence: compilation for `compute_75` returned PTX 8.2 with `.visible .entry testEmpty()`.

- Observation: current CUDA 13 documentation describes modern and LLVM-7 dialects, while the
  `nvvmAddModuleToProgram` API text still names LLVM 7.0.1 input. Runtime queries and compile probes
  must resolve this documentation seam; an arbitrary current LLVM bitcode writer is not safe.
  Evidence: NVVM IR specification section 1 and libNVVM API documentation for
  `nvvmAddModuleToProgram`.

- Observation: `SLANG_SOURCE_LANGUAGE_LLVM` exists publicly, but LLVM source support is not wired
  through every name, artifact, and downstream source-language table. The raw NVVM compile does not
  need those global mappings: its explicit `Assembly + LLVMIR + Kernel` artifact is the input-shape
  contract, while `options.sourceLanguage = SLANG_SOURCE_LANGUAGE_LLVM` is descriptive compile
  context only. This slice must not classify existing CPU LLVM artifacts as GPU artifacts globally.
  Evidence: `include/slang.h`, `source/core/slang-type-text-util.cpp`, and
  `source/compiler-core/slang-artifact-desc-util.cpp`.

- Observation: `IDownstreamCompiler::convert` receives only an input artifact and output
  descriptor, so it cannot carry the required architecture or floating-point/optimization policy.
  The later code-generation handoff already calls `compile` directly with
  `DownstreamCompileOptions`.
  Evidence: `source/compiler-core/slang-downstream-compiler.h` and
  `emitWithDownstreamForEntryPoints` in `source/slang/slang-code-gen.cpp`.

- Observation: `tools/gfx/slang.slang` is not a numeric mirror of the complete public
  `SlangPassThrough` enum; it currently stops after SPIR-V Optimize. Appending NVVM there would give
  it value 12 instead of the public ABI value 15.
  Evidence: compare the two enum declarations in `tools/gfx/slang.slang` and `include/slang.h`.

- Observation: the output descriptor for `SLANG_PTX` is `ObjectCode + PTX + Kernel`, not LLVM-style
  assembly. The bootstrap input is independently and explicitly `Assembly + LLVMIR + Kernel`.
  Evidence: `ArtifactDescUtil::makeDescForCompileTarget` in
  `source/compiler-core/slang-artifact-desc-util.cpp`.

- Observation: the only exhaustive source-language switch that needs a new case is
  `getDefaultSourceLanguageForDownstreamCompiler`, where NVVM maps to LLVM for generic prelude/query
  APIs. Count-sized session/compiler/locator arrays grow automatically; the PTX target requirement
  and global transition remain NVRTC; test categories and generic `-nvvm-*` options derive from the
  compiler name table.
  Evidence: `source/slang/slang-pass-through.cpp`, `source/slang/slang-global-session.*`,
  `source/compiler-core/slang-downstream-compiler-util.*`, `source/slang/slang-options.cpp`, and
  `tools/slang-test/slang-test-main.cpp`.

Add new entries here with a concise command, diagnostic, or code trace. Move settled conclusions
to the Decision Log and durable design document.

## Decision Log

- Decision: support CUDA 12.0+ and NVVM IR 2.0 in the first experimental backend.
  Rationale: CUDA 11 adds an incompatible NVVM IR 1.x representation without helping the initial
  compute proof.
  Date/Author: 2026-08-25, user and Codex.
  Revisit when: a concrete supported-user or CI requirement for CUDA 11 is identified.

- Decision: begin with the legacy LLVM-7 typed-pointer dialect.
  Rationale: it reaches pre-Blackwell GPUs and remains the broadest initial architecture path.
  Date/Author: 2026-08-25, user and Codex.
  Revisit when: the production bitcode spike or measured modern-dialect benefit supplies evidence.

- Decision: textual NVVM IR is permitted only for this bootstrap and for later diagnostic dumps.
  Rationale: it gives a readable, minimal external-API proof, but NVIDIA deprecates text input.
  Date/Author: 2026-08-25, user and Codex.
  Revisit when: never for production readiness; Slice 2 must validate bitcode.

- Decision: add a distinct `SLANG_PASS_THROUGH_NVVM` downstream compiler identity, appended before
  `SLANG_PASS_THROUGH_COUNT_OF`.
  Rationale: libNVVM has different discovery, versions, options, diagnostics, input language, and
  cache identity from NVRTC. Reusing the NVRTC identity would make routing and diagnostics
  ambiguous.
  Date/Author: 2026-08-25, Codex.
  Revisit when: only if the downstream compiler abstraction itself is replaced before code lands.

- Decision: dynamically load libNVVM and declare its small C ABI locally rather than adding a build
  or link dependency on a CUDA toolkit.
  Rationale: this matches existing downstream compilers, preserves optional toolkit support, and
  allows explicit path selection.
  Date/Author: 2026-08-25, Codex.
  Revisit when: the supported libNVVM distribution stops exposing a stable dynamic C API.

- Decision: keep the NVVM IR artifact internal and do not install a `CUDASource -> PTX` or
  `NVVMIR -> PTX` global transition in this slice.
  Rationale: ordinary PTX compilation must remain byte-for-byte on its established routing until
  the CUDA emission-method slice can select the representation deliberately.
  Date/Author: 2026-08-25, user and Codex.
  Revisit when: the experimental PTX routing slice begins.

- Decision: derive libNVVM and later libdevice from one selected CUDA toolkit root.
  Rationale: mixing versioned compiler and bitcode components has no supported compatibility
  contract.
  Date/Author: 2026-08-25, Codex.
  Revisit when: NVIDIA documents a supported cross-toolkit component combination.

- Decision: exercise only `IDownstreamCompiler::compile` in this slice and retain the base
  `canConvert == false` / `convert == SLANG_E_NOT_AVAILABLE` behavior.
  Rationale: conversion lacks the target options needed to produce valid, deterministic PTX. The
  future routing seam uses `compile`, so implementing conversion would create an incomplete second
  entry point without proving the real boundary.
  Date/Author: 2026-08-25, Codex.
  Revisit when: the downstream conversion API gains target options or a concrete consumer requires
  architecture-independent conversion.

## Outcomes and Retrospective

The planning outcome is complete: the reusable planning contract is in `.agent/PLANS.md`, and the
durable architecture/support contract is in `docs/design/nvvm-backend.md`.

The implementation outcome is not yet complete. Before closing this plan, record:

- the exact API/version/discovery behavior implemented;
- focused test and `ptxas` results;
- whether discovery required a shared CUDA-toolkit helper;
- every helper/fallback/special case added and its input-shape audit;
- limitations exposed to the next bitcode/differential slice; and
- the files and decisions distilled into the PR description.

## Context and Current Pipeline

Consider the eventual user command:

```text
slangc kernel.slang -target ptx -entry kernel -stage compute
```

`sm_75` is not a valid Slang profile name. In this raw downstream slice, the unit test supplies
the exact `compute_75` architecture as a CUDASM 7.5 requirement in `DownstreamCompileOptions`.
Defining how an ordinary PTX request selects an exact CUDA architecture belongs to the later
emission-method/routing slice.

Today, `_getDefaultSourceForTarget` in `source/slang/slang-code-gen.cpp` maps PTX to
`CodeGenTarget::CUDASource`. `emitWithDownstreamForEntryPoints` builds a source sub-context with that
target, runs `linkAndOptimizeIR`, emits CUDA C++ through `CUDASourceEmitter`, and calls the compiler
registered for `CUDASource -> PTX`. `_initCodeGenTransitionMap` in
`source/slang/slang-global-session.cpp` registers NVRTC for that transition.

The raw libNVVM slice deliberately enters below that pipeline:

```text
handwritten NVVM IR artifact
    -> NVVMDownstreamCompiler::compile
       -> nvvmCreateProgram
       -> nvvmAddModuleToProgram
       -> nvvmVerifyProgram
       -> nvvmCompileProgram
       -> nvvmGetCompiledResult
    -> PTX artifact + associated diagnostics
```

This isolation proves that discovery, API calls, option spelling, logs, artifact ownership, and
cleanup work before any Slang IR representation is changed.

The downstream abstraction is defined in
`source/compiler-core/slang-downstream-compiler.h`. NVRTC's implementation in
`source/compiler-core/slang-nvrtc-compiler.cpp` is the closest lifecycle template, but its CUDA C++
headers, prelude support, and NVRTC-specific option logic are not reusable NVVM semantics.

## Scope and Non-Goals

### In scope

- a new appended `SlangPassThrough` enumerant, its internal `PassThroughMode` mirror, and exact
  name/switch consumers;
- compiler name lookup for `nvvm`, enabling generic `-nvvm-path`, `-nvvm-version`, and `-Xnvvm`
  parsing where applicable;
- a handwritten libNVVM ABI table with required and optional symbols;
- dynamic library discovery on the platforms for which Slang currently supports libNVVM;
- numeric NVVM and NVVM IR version capture, plus optional per-architecture LLVM version query;
- one NVVM IR assembly artifact to PTX conversion through `IDownstreamCompiler`;
- complete verifier/compiler logs associated with the result artifact;
- an explicit architecture and deterministic initial compile policy; and
- positive/negative unit coverage plus a local `ptxas` assembly check.

### Not in scope

- `SlangEmitCUDAMethod` or command-line routing for ordinary `-target ptx`;
- Slang IR to NVVM IR lowering;
- a public `SLANG_NVVM_IR` compile target or artifact payload;
- production LLVM bitcode generation;
- libdevice linking or math lowering;
- GPU execution through the CUDA driver;
- resources, shared memory, atomics, waves, OptiX, debug metadata, RDC, or LTO;
- changing the default CUDA downstream compiler; or
- changing established NVRTC discovery or PTX output behavior except for a proven behavior-
  preserving extraction of shared toolkit-root enumeration.

## Architecture and Invariants

The slice preserves these invariants:

1. Existing public enum values never change. `SLANG_PASS_THROUGH_NVVM = 15` is appended immediately
   before `SLANG_PASS_THROUGH_COUNT_OF`, and all array dimensions continue to use `COUNT_OF`.
2. The compiler is optional. Slang builds and runs without CUDA headers, import libraries, or a
   toolkit installation.
3. Each compiler instance owns a strong reference to the loaded shared library, so no function
   pointer outlives its code.
4. Each compile creates one `nvvmProgram` and destroys it exactly once on success or failure.
5. A compile consumes exactly one in-memory NVVM IR artifact and returns one PTX artifact.
6. The output artifact and associated diagnostics are created before any verifier or compiler
   operation that can fail.
7. Verification and compilation use the same architecture and semantic policy options.
8. A verifier/compiler rejection records failure and a full error diagnostic on the PTX-desc
   artifact, calls `requireErrorDiagnostic`, and returns `SLANG_OK`, matching existing in-process
   compiler convention. Interface or operational failures may return a failing `SlangResult`, but
   still return the artifact.
9. The complete log is retained for both verifier and compiler failures. If libNVVM supplies only
   an empty/NUL log, the diagnostic falls back to `nvvmGetErrorString(result)`.
10. Automatic discovery never combines libNVVM and libdevice roots. This slice records the selected
   library path and retains a toolkit root when one can be derived; a system-loader-only result may
   leave the root unknown until a coherent libdevice lookup is actually required.
11. The loader distinguishes required symbols from optional version-query additions.
12. The locator returns `SLANG_E_NOT_FOUND` only when it finds no loadable candidate. A loaded but
    invalid candidate returns its initialization failure instead of masquerading as absence.
13. The ordinary PTX transition still resolves to NVRTC after this slice.
14. No CPU LLVM target or artifact is globally reclassified as a CUDA/NVVM artifact.

## Interfaces and Dependencies

### Public and mirrored identities

Append `SLANG_PASS_THROUGH_NVVM = 15` in `include/slang.h`. Mirror it in the complete internal
`PassThroughMode` enum in `source/slang/slang-pass-through.h`, and add `nvvm` to the compiler name
table in `source/core/slang-type-text-util.cpp`.

Do not append NVVM to `tools/gfx/slang.slang` in this slice. That declaration is an incomplete
legacy subset whose implicit values already diverge from the public enum after SPIR-V Optimize.
Repairing it requires explicit values and a compatibility audit as a separate change; casually
appending NVVM would publish the wrong numeric identity.

Audit every array indexed by `SLANG_PASS_THROUGH_COUNT_OF` and every exhaustive pass-through switch.
Do not add a default branch merely to silence a missing case; either NVVM has a defined role in that
switch or it should take the existing explicit none/unsupported behavior.

### New compiler files

Add:

```text
source/compiler-core/slang-nvvm-compiler.h
source/compiler-core/slang-nvvm-compiler.cpp
tools/slang-unit-test/unit-test-nvvm-downstream-compiler.cpp
```

Update `tests/downstream/downstream-compiler-version.slang` with an `-nvvm-version` lane and stable
`nvvm version:` check so the generic CLI parsing/output path is covered in addition to the API unit
test.

`slang-nvvm-compiler.h` exposes only the locator/testable utility surface needed by
`slang-downstream-compiler-util.cpp`. Keep the concrete compiler class private to the `.cpp` unless
a focused test demonstrates a reason for a larger interface.

The function table declares NVIDIA's public C ABI without including `nvvm.h`. Required functions
for this slice are:

```text
nvvmGetErrorString
nvvmVersion
nvvmIRVersion
nvvmCreateProgram
nvvmDestroyProgram
nvvmAddModuleToProgram
nvvmVerifyProgram
nvvmCompileProgram
nvvmGetCompiledResultSize
nvvmGetCompiledResult
nvvmGetProgramLogSize
nvvmGetProgramLog
```

Resolve `nvvmLLVMVersion` optionally with its exact public signature,
`nvvmResult nvvmLLVMVersion(const char* arch, int* major)`. Resolve
`nvvmLazyAddModuleToProgram` now only if doing so does not make an older otherwise-supported CUDA 12
library fail initialization; its first use belongs to the libdevice slice.

### Downstream registration

Update `source/compiler-core/slang-downstream-compiler-util.cpp` to:

- include the new locator header;
- install the locator in the `SLANG_PASS_THROUGH_COUNT_OF` locator table; and
- avoid selecting NVVM as the default compiler for an existing source language.

Update the exhaustive `getDefaultSourceLanguageForDownstreamCompiler` switch in
`source/slang/slang-pass-through.cpp` so `PassThroughMode::NVVM` returns `SourceLanguage::LLVM`.
That mapping keeps generic prelude/query APIs from reaching the unknown-compiler assertion; it does
not make NVVM the default compiler for LLVM input.

Do not add a global LLVM source-language name, payload mapping, compatibility flag, or default in
this slice. The explicit raw artifact supplies the representation contract. The unit test may set
`DownstreamCompileOptions::sourceLanguage = SLANG_SOURCE_LANGUAGE_LLVM` as descriptive context, but
the compiler validates the artifact descriptor rather than inferring NVVM legality from that enum.

### Artifact contract

Represent the bootstrap input as an internal `Assembly + LLVMIR + Kernel` artifact and set
`DownstreamCompileOptions::sourceLanguage` to `SLANG_SOURCE_LANGUAGE_LLVM` and `targetType` to
`SLANG_PTX`. `compile` must accept exactly one source artifact with that input shape. Retain the
base implementation of `canConvert` and `convert`; neither can express the required target
architecture.

The unit test constructs the descriptor with `ArtifactDesc::make(ArtifactKind::Assembly,
ArtifactPayload::LLVMIR, ArtifactStyle::Kernel, 0)`, creates it through `ArtifactUtil`, and adds the
in-memory text as a blob representation. The compiler creates its output through
`ArtifactUtil::createArtifactForCompileTarget(SLANG_PTX)`, whose exact descriptor is
`ObjectCode + PTX + Kernel`, and calls the located compiler through
`IDownstreamCompiler::compile`. This proves the actual interface that later PTX routing will
consume.

### Toolkit discovery

Discovery precedence on the current Windows platform is:

1. the explicit path supplied through `setDownstreamCompilerPath`/`-nvvm-path`, accepting a library
   file, an NVVM root, or a toolkit root;
2. a logical `loader->loadSharedLibrary("nvvm", ...)` attempt, which supports injected fake loaders
   and platform/system loader installations;
3. the Slang module/instance directory when it contains a complete candidate;
4. `CUDA_PATH`, checking `nvvm\bin` and version-specific subdirectories; and
5. CUDA-looking toolkit paths already present on `PATH`.

Normalize an explicit decorated library filename into the unadorned name/path expected by
`ISlangSharedLibraryLoader` before loading it. For example, the loader contract does not accept a
Windows `.dll` suffix or a Linux `lib` prefix/`.so` suffix as its normal input shape. Keep the
original resolved candidate path separately for diagnostics and toolkit-root derivation.

Candidate filenames are version-sensitive. Match and rank plausible libNVVM basenames rather than
hardcoding only `nvvm64_40_0.dll`. On non-Windows platforms, use the paths and shared-library naming
rules from NVIDIA's public samples and Slang's existing platform helpers. After a logical/system
load, derive the actual library path and toolkit root from a resolved symbol when the platform can
report it; do not invent a root when it cannot.

Before writing a second CUDA root search, inspect whether the root enumeration in
`slang-nvrtc-compiler.cpp` can be extracted into a narrowly documented helper without changing
candidate order or behavior. Promote the extraction only if focused NVRTC discovery tests preserve
the old result; otherwise keep the first NVVM locator self-contained and record the rejected
duplication trade-off here.

The selected compiler retains its resolved library path, any derivable toolkit root, and the
expected `nvvm\libdevice\libdevice.10.bc` location when that root is known. The empty-kernel compile
neither reads nor adds libdevice.

### Version identity

Populate `DownstreamCompilerDesc::version` from `nvvmVersion`. Also retain the IR/debug tuple from
`nvvmIRVersion`, and implement `getVersionString` with both tuples plus
`SharedLibraryUtils::getSharedLibraryTimestamp` for the loaded implementation. The timestamp
distinguishes patch builds that all report the same numeric NVVM version and follows existing
downstream-compiler cache identity precedent. The generic `-nvvm-version` CLI can report only the
descriptor's NVVM major/minor; assert the fuller string through the focused internal test rather
than broadening the public API in this slice.

When `nvvmLLVMVersion` exists, a query failure for one architecture is a target-compatibility
result, not a loader failure. Its architecture-dependent answer does not belong in the
instance-only version string; later cache work must combine it with the selected target profile.

An API-complete library with NVVM IR 1.x may be discoverable so the numeric query and
`getVersionString` can explain what was loaded, but compiling this slice's IR 2.0 artifact must
produce a clear incompatibility diagnostic.

### External dependency

The current local candidate is:

```text
C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.2\nvvm\bin\nvvm64_40_0.dll
```

The matching libdevice path is:

```text
C:\Program Files\NVIDIA GPU Computing Toolkit\CUDA\v12.2\nvvm\libdevice\libdevice.10.bc
```

The build must not include the CUDA header or link `nvvm.lib`; these paths are runtime test inputs.

## Milestones

### Milestone 0: Finish the architecture and input-shape audit

Read every `SlangPassThrough` switch and `COUNT_OF` array. Write the inventory in the Progress or
Surprises sections, including the required action for NVVM. Confirm the exact artifact descriptor
used by `ArtifactUtil::createArtifactForCompileTarget(SLANG_PTX)` and by the raw source artifact.

Inspect all new helper candidates before implementation:

- shared CUDA toolkit root enumeration;
- versioned libNVVM filename parsing/ranking;
- NVVM result-to-Slang result conversion;
- log retrieval; and
- compile-option construction.

For each, search for an existing helper in `core/` and `compiler-core/`. Record whether the helper is
reused, generalized, or genuinely NVVM-specific. This milestone is complete when the file list and
interfaces above match the repository rather than merely the initial proposal.

Status: complete on 2026-08-25. The audit removed conversion support, global LLVM source-language
wiring, and the unsafe `tools/gfx` mirror edit; confirmed the exact input/output artifact shapes;
added the mandatory source-language switch case; split fake and real tests; and corrected the
version, discovery, routing-sentinel, diagnostics, and `ptxas` evidence plans.

### Milestone 1: Add the compiler identity and loader

Append the public/mirrored enum value and compiler name. Add the new locator and register it in
`DownstreamCompilerUtil::setDefaultLocators`. Implement the handwritten ABI table, required/optional
symbol resolution, shared-library ownership, version queries, and compiler descriptor.

Add always-running fake-loader tests using dependency injection already present in the downstream
locator APIs. They must prove that a missing required symbol rejects a candidate and that lifecycle
cleanup works without adding a special production fallback. Keep these tests separate from the
environment-aware real-toolkit smoke so an unavailable-tool skip cannot hide ABI coverage. Do not
expose the concrete compiler solely for testing if the loader interface can exercise it.

The locator returns `SLANG_E_NOT_FOUND` only when no candidate can be found or loaded. Once a
candidate library loads, missing mandatory symbols or failing version/initialization calls return a
failure result rather than being collapsed into absence. The real smoke may skip only the former;
the fake missing-symbol test asserts the latter and that no compiler was registered.

Completion evidence:

- the compiler builds without CUDA development headers or import libraries;
- `checkPassThroughSupport(SLANG_PASS_THROUGH_NVVM)` succeeds on this machine;
- `-nvvm-version` reports the loaded numeric NVVM version, while the internal version-string test
  reports NVVM, NVVM IR/debug, and library timestamp identity; and
- the `tests/downstream/downstream-compiler-version` file test covers the CLI option and stable
  output prefix; and
- established NVRTC version/discovery tests still pass.

### Milestone 2: Implement coherent discovery

Implement explicit-path and automatic discovery with deterministic candidate ordering. Retain the
resolved library identity and a selected toolkit root when derivable. Add focused tests for
logical-name loader precedence, decorated-path normalization, candidate ranking, explicit-path
precedence, and missing-library diagnostics using temporary paths or injected loaders; do not
change the process-wide `CUDA_PATH` in a way that can leak between parallel tests.

On the current machine, record the resolved DLL and libdevice paths in the test/working evidence.
If a shared CUDA-root helper is extracted, run every focused NVRTC locator test before proceeding.

### Milestone 3: Verify and compile the empty module

Implement `compile`. The test supplies a CUDASM 7.5 capability and explicit policy inputs for
`-opt=3`, `-ftz=0`, `-prec-div=1`, `-prec-sqrt=1`, and `-fma=1`. Derive
`-arch=compute_75` from the required capability, derive options from existing semantic fields where
the mapping is already well defined, and otherwise use `compilerSpecificArguments` for this raw-IR
fixture. Do not hardcode the fixture's policy flags as compiler defaults. Create the PTX artifact
and diagnostics before calling libNVVM. Use an RAII program owner and retrieve the complete log
after verification and compilation.

Mirror the existing in-process compiler convention: a verifier/compiler rejection sets a failing
result on the associated diagnostics, adds the complete log as an error, calls
`requireErrorDiagnostic`, returns the PTX-desc artifact, and returns `SLANG_OK` from `compile` so the
caller can forward the structured diagnostics. An interface or operational failure may return a
failing `SlangResult`, but it still returns the artifact. When the reported log size is only the
trailing NUL, use `nvvmGetErrorString(result)` as the diagnostic text instead of emitting an empty
error.

Use this test module:

```llvm
target datalayout = "e-p:64:64:64-i1:8:8-i8:8:8-i16:16:16-i32:32:32-i64:64:64-i128:128:128-f32:32:32-f64:64:64-v16:16:16-v32:32:32-v64:64:64-v128:128:128-n16:32:64"
target triple = "nvptx64-nvidia-cuda"

define void @testEmpty() {
entry:
  ret void
}

!nvvmir.version = !{!0}
!nvvm.annotations = !{!1}
!0 = !{i32 2, i32 0}
!1 = !{void ()* @testEmpty, !"kernel", i32 1}
```

The real-toolkit smoke is environment-aware: it skips with the standard unavailable-tool mechanism
when libNVVM is absent, but it must fail rather than skip when a discovered compiler rejects the
known-valid module. The fake-loader ABI/lifecycle test always runs.

Completion evidence:

- verification succeeds;
- the result artifact payload contains `.visible .entry testEmpty`;
- the result and diagnostics have the expected artifact associations; and
- the program handle is destroyed on every injected failure point.

### Milestone 4: Add negative contract coverage

Add focused cases for:

- `!nvvmir.version = 1.11`, expecting the version-mismatch diagnostic;
- malformed NVVM IR, expecting the verifier log;
- a missing or unsupported CUDASM capability, expecting a target-option diagnostic;
- a missing required function symbol, expecting candidate initialization failure; and
- a fake API failure with an empty program log, expecting the `nvvmGetErrorString` fallback; and
- zero or multiple source artifacts, expecting the downstream interface contract failure.

Tests should assert stable semantic fragments and result classifications, not entire NVIDIA log
wording that can change between toolkits.

### Milestone 5: Validate locally and perform the principled-change audit

Build and run the focused unit tests. Have the real-toolkit smoke write its successful artifact to
a unique path created with `File::generateTemporary`, assemble it with the matching toolkit's
`ptxas.exe`, capture the verbose output, and clean the PTX/cubin on success. A fixed `$env:TEMP`
filename would race parallel test runs. Do not leave generated PTX, cubin, or logs in the
repository.

Run the established downstream compiler version test and any artifact-description tests touched by
the new source/artifact mapping. Confirm an ordinary `-target ptx` test still selects NVRTC.

Inventory every new helper, fallback, filename special case, and platform branch in the diff. For
each, record its exact input, producer, whether that input is canonical, the downstream consumer,
and the focused test that fails without it. Replace any silent impossible-shape fallback with a
contract diagnostic or assertion at the proper boundary.

Complete Outcomes and Retrospective, distill stable findings into `docs/design/nvvm-backend.md`, and
prepare the five-part PR narrative before declaring the plan finished.

## Validation and Acceptance

### Build

Use Windows-native tools on the current checkout. The `slang-build` skill is preferred when it is
available. If it remains unavailable, use the already configured Windows build as allowed by
`AGENTS.md`:

```powershell
cmake.exe --build build --config Debug --target slang-test
```

Do not reconfigure or delete the existing build directory for this slice unless the build itself
proves that reconfiguration is required.

### Focused tests

The intended focused command is:

```powershell
build\Debug\bin\slang-test.exe slang-unit-test-tool/nvvmDownstreamCompilerAbi
build\Debug\bin\slang-test.exe slang-unit-test-tool/nvvmDownstreamCompiler
```

Use the actual registered unit-test name if the test harness applies a different spelling, and
update this plan immediately.

Also run:

```powershell
build\Debug\bin\slang-test.exe slang-unit-test-tool/getDownstreamCompilerVersion
build\Debug\bin\slang-test.exe tests/downstream/downstream-compiler-version
build\Debug\bin\slang-test.exe tests/cuda/sampler-comparison-state-unused
```

The CUDA regression is a routing sentinel, not a new-backend capability test. Confirm its log or
compiler selection still identifies NVRTC where observable.

### PTX assembly

The real-toolkit unit test resolves `ptxas.exe` from the same toolkit root as libNVVM, writes the
returned PTX and cubin to unique OS-temporary paths, and invokes the equivalent of:

```text
ptxas.exe -arch=sm_75 -v <unique-input.ptx> -o <unique-output.cubin>
```

Acceptance requires exit code zero and successful cleanup. Record the toolkit root, libNVVM/IR
versions, PTX header, architecture, and captured `ptxas -v` resource summary in Outcomes and
Retrospective. If the library was found through a system loader and no coherent toolkit root can be
derived, report the missing `ptxas` evidence distinctly rather than assembling with an unrelated
toolkit.

### Acceptance checklist

- Slang builds without a static CUDA/libNVVM dependency.
- Existing public enum values are unchanged.
- The NVVM compiler is discoverable explicitly and automatically on this machine.
- `-nvvm-version` reports NVVM 2.0, and the focused internal version-string test reports NVVM IR
  2.0/Debug 3.1 plus library timestamp identity for the current DLL.
- The known-valid IR verifies and produces a PTX artifact containing `testEmpty`.
- `ptxas` assembles that artifact for `sm_75`.
- Negative tests retain useful associated diagnostics.
- Missing libNVVM causes a standard skip/unavailable result, not a crash or false success.
- Ordinary PTX compilation still routes through NVRTC.
- No generated binary or working-log artifact is added to the commit.

## Failure and Recovery

All code changes in this slice are additive except an optional behavior-preserving extraction of
CUDA toolkit discovery. No transition selects NVVM, so a partial implementation cannot affect
ordinary PTX compilation unless it changes shared discovery or enum-indexed tables.

If loader initialization fails, use the missing-symbol and selected-candidate evidence before
adding another filename or path fallback. A new fallback is allowed only for a documented toolkit
layout and needs a focused test.

If the known-valid module fails:

1. record `nvvmVersion`, `nvvmIRVersion`, the exact target options, and the full log;
2. compare the module with NVIDIA's maintained empty-kernel sample;
3. confirm libNVVM and any future libdevice path share one toolkit root; and
4. treat a dialect/version mismatch as a compatibility result, not an invitation to mutate IR
   until the verifier happens to accept it.

If the shared CUDA-root extraction changes NVRTC discovery, revert that extraction and keep the
NVVM locator self-contained for this slice. Do not preserve a cross-compiler refactor without a
test that proves both callers' candidate order and selected path.

Reruns are safe: each compile owns a fresh `nvvmProgram`, tests use unique OS-temporary paths, and
no compiler transition or persistent cache schema is changed.

## Artifacts and Hand-Off

The real-toolkit test keeps the exact NVVM IR in its source fixture, uses unique temporary files for
the raw PTX and assembled cubin, and removes those files after a successful run. It captures
verifier/compiler logs and `ptxas -v` output in the test result. On failure, print enough path and
option information to reproduce the case without relying on a fixed retained filename.

Record version/options/resource summaries in this plan rather than committing generated outputs.

At completion, update `docs/design/nvvm-backend.md` with only durable contract changes. The next
ExecPlan is the production bitcode feasibility slice. Its starting evidence is the compiler
boundary and test fixture delivered here; it must not assume that Slang's LLVM 21 bitcode writer is
compatible with the loaded libNVVM.

## Authoritative External References

The implementation contract comes from:

- https://docs.nvidia.com/cuda/libnvvm-api/index.html
- https://docs.nvidia.com/cuda/nvvm-ir-spec/index.html
- https://github.com/NVIDIA/cuda-samples/tree/master/Samples/7_libNVVM
- https://docs.nvidia.com/cuda/libdevice-users-guide/basic-usage.html

The plan summarizes the required pieces above so a later executor does not need the original
research conversation. CICC reverse-engineering notes may suggest experiments but cannot alter the
acceptance contract.
