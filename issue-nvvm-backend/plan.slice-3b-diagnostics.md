# Expose LLVM verifier diagnostics through the NVVM builder ABI

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds and do not commit it.

## Purpose and Observable Result

Slice 3a can construct and serialize a minimal LLVM 14 NVVM module, but it discards the canonical
LLVM verifier text when the module is malformed. This slice makes that diagnostic cross the
optional-module boundary without returning provider-owned memory or introducing mutable global or
thread-local error state.

After this slice, the existing unterminated-block unit-test module fails serialization with a
host-owned diagnostic containing the kernel name and LLVM's terminator error. Adding `ret void` to
the same block then produces bitcode with an empty diagnostic. A fresh-process test also proves the
previously untested load order in which the LLVM 14 NVVM module is used before Slang loads its LLVM
21 downstream module.

The shortest local observation is a focused NVVM unit-test run with
`SLANG_NVVM_BUILDER_PATH` pointing at the separately built module. The diagnostic test must cross
the real DLL boundary, and the load-order test must run through a fully isolated test server.

## Progress

- [x] (2026-08-26) Verified the committed Slice 3a baseline at `88b9281a8`, inspected the exact V1
  ABI/provider/host code, and confirmed the worktree contains only the unrelated untracked
  `external/slang-binaries/` directory.
- [x] (2026-08-26) Defined the Slice 3b scope and selected an atomic V2
  serialization-and-diagnostics operation while freezing V1.
- [x] (2026-08-26) Added the V2 C ABI, host negotiation, atomic provider operation, platform export
  allowlists, and strict C probe; the standalone provider and compiler-core build successfully.
- [x] (2026-08-26) Ran the pre-Slice-3b test binary against the rebuilt provider; its V1-only host
  loaded the unchanged V1 getter and serialized the real empty kernel successfully (1/1).
- [x] (2026-08-26) Added fake V2 negotiation, status/shape, exact-byte, mismatch, and atomic-buffer
  tests. The focused V1/V2 protocol selection passes 3/3.
- [x] (2026-08-26) Captured real LLVM 14 verifier bytes through the host wrapper. The malformed
  `uniqueKernel` reports its missing terminator and the repaired module serializes with no
  diagnostic.
- [x] (2026-08-26) Added real-provider V1/V2 short-buffer and mixed-destination coverage; required
  sizes/status are preserved and neither destination is partially written.
- [x] (2026-08-26) Proved LLVM 21-first and LLVM 14-first operation in separate fully isolated
  test-server processes. The parent requires each child to report exactly 1/1 tests, preventing a
  zero-match exit from passing vacuously.
- [x] (2026-08-26) Rebuilt the provider and host tests, passed the complete NVVM prefix 27/27,
  passed the three established regression selections, confirmed optional provider absence, and
  rechecked the two-symbol/no-LLVM-DLL binary boundary.

## Surprises and Discoveries

- Observation: the committed V1 getter and both the provider and host require exact
  `sizeof(SlangNVVMBuilderAPI_V1)`. Appending a function to V1 would therefore make old and new
  binaries reject one another rather than negotiate an optional tail.
  Evidence: `slang_getNVVMBuilderAPI_V1` in `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` and
  `NVVMIRBuilder::initialize` in `source/compiler-core/slang-nvvm-ir-builder.cpp`.

- Observation: `llvm::verifyModule` already receives a `raw_null_ostream`, so the canonical
  producer generates the required text and only its sink needs to change. No downstream parser or
  reconstructed diagnostic is necessary.
  Evidence: `_serializeModule` in `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp`.

- Observation: a normal `slang-test` process probes every pass-through compiler before parsing
  command-line options. Even `-skip-api-detection` therefore loads LLVM 21 in the coordinator.
  `-use-fully-isolated-test-server` executes each selected unit test in a fresh `test-server`, whose
  global-session creation does not probe downstream compilers before the test function runs.
  Evidence: `tools/slang-test/slang-test-main.cpp` near the pass-through category setup and
  `tools/test-server/test-server-main.cpp::_executeUnitTest`; a focused isolated invocation of the
  existing coexistence test passes.

- Observation: LLVM 14 deletes `raw_svector_ostream::flush()` because the stream writes directly
  into its vector. Verifier capture therefore uses `raw_string_ostream` and an explicit flush,
  while assembly/bitcode use the unbuffered vector stream without a flush call.
  Evidence: the first standalone compile diagnostics and the subsequent successful rebuild.

- Observation: the rebuilt V2 provider remains compatible with a previously built Slice 3a host.
  Evidence: the old `nvvmIRBuilderSerializesEmptyKernel` binary loaded the new DLL through V1 and
  passed 1/1 before the Slice 3b unit-test module was rebuilt.

- Observation: a V2 write with one non-null destination is not a query, even when the omitted
  output has the only nonzero size. The first provider draft checked capacities only for supplied
  destinations and could therefore return success without writing all bytes.
  Evidence: independent review of `_serializeModuleWithDiagnostics`; the provider now treats only
  the both-null form as a query, and the real-provider mixed-destination sentinel test passes.

- Observation: `slang-test` returns exit code zero when a filter matches no tests. Child exit code
  alone would therefore make a misspelled self-spawn selector look like a successful load-order
  proof.
  Evidence: `TestReporter::didAllSucceed` and the final parent assertion that each child reports
  `100% of tests passed (1/1)`.

- Observation: LLVM's verifier API classifies invalidity independently of whether its diagnostic
  stream receives bytes. V2 promises a useful nonempty diagnostic for `INVALID`, so the canonical
  capture boundary supplies a stable generic message only when LLVM emits none.
  Evidence: `_materializeModule`; fake malformed-shape tests prove the host rejects an
  `INVALID` result with an empty diagnostic before issuing a write.

- Observation: MSBuild's FileTracker cannot update its tracking state in the filesystem sandbox.
  The same Windows-native CMake builds succeed when run with the approved build escalation.
  Evidence: the initial access-denied build and all subsequent successful provider/compiler-core/
  unit-test builds.

## Decision Log

- Decision: preserve `SlangNVVMBuilderAPI_V1` and `slang_getNVVMBuilderAPI_V1` byte-for-byte, and
  add `SlangNVVMBuilderAPI_V2` plus `slang_getNVVMBuilderAPI_V2`.
  Rationale: Slice 3a is now a committed compatibility point. A distinct version keeps old hosts
  working with the new provider and makes the new diagnostic requirement explicit.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the private ABI is deliberately replaced before distribution rather than evolved.

- Decision: compose V2 from a complete V1 table plus one required
  `serializeModuleWithDiagnostics` function instead of duplicating every V1 field.
  Rationale: V1 remains the single source of truth for handles and construction operations. V2
  handles necessarily come from its nested table in the same library, while the new operation is
  independently versioned and validated.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a future incompatible handle or module representation requires V3.

- Decision: make V2 an append-only, size-negotiated table from its first release. The caller
  supplies capacity; the provider copies only the known prefix, reports its full size, and hosts
  require every field they use to be present.
  Rationale: future scalar operations can extend V2 without repeating the exact-size limitation
  discovered in V1. Changing an existing field's signature or semantics still requires V3.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a capability/query-interface scheme is justified by multiple independent optional
  extensions.

- Decision: return serialization bytes, verifier bytes, and a verification status from one atomic
  operation. Transport success is distinct from `VALID` or `INVALID` verification status.
  Rationale: an invalid query must still report the diagnostic size so the host can allocate and
  repeat the call. A module-owned or global “last diagnostic” can become stale or detached from the
  operation that produced it.
  Date/Author: 2026-08-26, Codex.
  Revisit when: profiling justifies an explicit immutable serialization-result handle that avoids
  repeated verification/materialization.

- Decision: require nonempty serialized output for `VALID` but allow optional diagnostic bytes;
  require zero serialized output and a nonempty diagnostic for `INVALID`.
  Rationale: the current LLVM verifier emits no text for valid modules, but permitting diagnostic
  bytes lets a future provider carry warnings without another ABI revision. Strict invalid output
  shapes ensure the host never mistakes malformed provider output for a compiler result.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a separate structured warning channel is introduced.

- Decision: reserve both-null destinations for the query form and require non-overlapping caller
  storage for every destination and output-metadata value.
  Rationale: any call that supplies one destination is a write and must either copy every nonempty
  result or fail before copying either one. Non-overlap makes that atomicity contract well-defined
  without provider-specific alias analysis.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the ABI moves to an immutable result handle rather than caller-owned buffers.

- Decision: prefer V2 when its symbol is present, fail on a malformed V2, and fall back to V1 only
  when the V2 symbol is absent.
  Rationale: falling back after finding a broken V2 masks deployment errors. V1-only providers
  remain usable for Slice 3a behavior, while callers can explicitly require
  `supportsSerializationDiagnostics()` before broader lowering.
  Date/Author: 2026-08-26, Codex.
  Revisit when: diagnostics become mandatory for every use of the optional module.

- Decision: use Slang's fully isolated test-server mode for load-order validation.
  Rationale: the ordinary test coordinator eagerly probes LLVM, whereas the isolated server gives
  the selected unit test control over the first module load without adding a special executable.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the process API gains an explicit clean environment/module-load harness.

## Outcomes and Retrospective

Slice 3b is complete on the local Windows/CUDA 12.2 environment. The optional LLVM 14.0.6 module
now exports the frozen V1 getter plus a size-negotiated V2 getter. V2 carries serialization bytes,
verifier bytes, and status atomically into host-owned storage. The host prefers V2, rejects a
malformed advertised V2, and retains V1-only fallback. A previously compiled Slice 3a test passed
against the rebuilt provider through V1, demonstrating old-host/new-provider compatibility.

The real unterminated kernel returns `SLANG_FAIL` only after its LLVM diagnostic, including
`uniqueKernel` and the missing-terminator text, has been copied. Repairing that exact module with
`ret void` produces bitcode and an empty diagnostic. Fake tests cover embedded NULs, future-sized
tables, malformed output shapes, and query/write instability. Real-provider tests cover V1/V2
short buffers, the mixed-destination write edge, and preservation of untouched sentinels.

The final focused run passes 27/27 NVVM tests, including CUDA 12.2 libNVVM compilation and both
`ptxas` checks. The load-order test self-spawns fully isolated workers and proves both LLVM 21.1
then LLVM 14.0.6 and LLVM 14.0.6 then LLVM 21.1, with LLVM 14 used on both sides of the latter
load. The absent-provider run passes 3/3 fake tests and ignores the six real-provider tests. The
unclosable-library and downstream-version regressions pass 1/1 each; the ordinary NVRTC sampler
regression passes 2/2.

The final PE image exports exactly `slang_getNVVMBuilderAPI_V1` and
`slang_getNVVMBuilderAPI_V2`. Its normal dependencies are `ADVAPI32.dll` and `KERNEL32.dll`, its
delay-load dependencies are `SHELL32.dll` and `ole32.dll`, and it imports no LLVM DLL. Slice 4 can
therefore rely on a diagnostic-preserving, versioned, load-order-tested LLVM 14 construction
boundary. It still must add the first scalar/pointer IR operations; this slice deliberately does
not lower Slang IR or compare nontrivial PTX.

## Context and Current Pipeline

Slice 3a established this producer/consumer path:

```text
NVVMIRBuilder construction calls
    -> slang-llvm-nvvm (LLVM 14.0.6)
    -> LLVM verifier
    -> assembly or bitcode copied into host storage
    -> NVVMDownstreamCompiler
    -> CUDA libNVVM
    -> PTX
```

Before Slice 3b, an unterminated block followed this trace:

```text
NVVMIRBuilder::serializeModule
    -> SlangNVVMBuilderAPI_V1::serializeModule
    -> slang-llvm-nvvm::_serializeModule
    -> llvm::verifyModule(module, raw_null_ostream)
    -> SLANG_FAIL
```

The result reached the host, but the verifier text was intentionally discarded. Slice 3b replaces
that route with the V2 atomic operation while retaining the V1 trace for old hosts. It changes only
the boundary between `NVVMIRBuilder` and `slang-llvm-nvvm`; it does not attach diagnostics to an
artifact because no Slang IR emitter or CUDA routing path consumes the builder yet.

“Transport result” means whether the ABI call and buffer protocol succeeded. “Verification
status” means whether LLVM ran and classified the module as valid or invalid. An invalid LLVM
module is a successful transport transaction with `INVALID` status and diagnostic bytes; the host
wrapper maps that status to `SLANG_FAIL` after copying the bytes.

## Scope and Non-Goals

In scope:

- an immutable V2 wrapper around the complete V1 construction table;
- an atomic, caller-owned serialization/diagnostic buffer protocol;
- host negotiation of V2 with deliberate V1-only fallback;
- fake ABI coverage for versioning, sizes, status, exact bytes, and no-partial-write behavior;
- real LLVM verifier diagnostics and valid-module serialization;
- direct real-provider insufficient-buffer coverage;
- both LLVM-21-first and LLVM-14-first operation in fresh processes;
- C compatibility, platform export allowlists, design documentation, and local binary inspection.

Explicitly deferred to Slice 4:

- integer, floating-point, pointer, or address-space types;
- parameters, constants, GEP, load/store, casts, attributes, arithmetic, or control flow;
- `writeScalar`/`copyScalar` reference kernels or NVRTC-versus-NVVM PTX comparisons;
- a capability ledger, CUDA routing, a public target, or Slang IR traversal/lowering;
- libdevice, resources, optimization work, runtime execution, packaging, and CI;
- serialization-result caching/freeze and performance conclusions.

## Architecture and Invariants

V1 is frozen. The new provider continues exporting its exact V1 getter so a Slice 3a host can use
all existing construction and serialization functions unchanged.

V2 contains `structureSize`, `abiVersion`, a complete `SlangNVVMBuilderAPI_V1 baseAPI`, and a
required `serializeModuleWithDiagnostics` function. The caller zero-initializes V2 and supplies its
capacity. The provider requires capacity through the last required Slice 3b field, copies at most
the caller's capacity, and reports the provider's complete structure size. The Slice 3b
minimum-size constant is frozen when later fields are appended. A host accepts larger provider
sizes, copies only its local prefix, clamps the stored table size to that prefix, and never reads a
reported future tail.

The new operation has two caller-owned outputs. On every call it reports exact serialized and
diagnostic byte counts; counts exclude a diagnostic NUL terminator. A query supplies null
destinations with zero capacities. A write supplies buffers sized from the query. Before copying
anything, the provider checks every supplied capacity. If one is insufficient it returns
`SLANG_E_BUFFER_TOO_SMALL`, reports both sizes and the verification status, and modifies neither
buffer. Only both-null destinations form a query; supplying either destination forms a write and
requires destinations for every nonempty result. Destination ranges and output-metadata storage
must not overlap.

The status contract is:

- `NOT_RUN`: invalid arguments, unsupported format, or another transport failure before LLVM
  verification;
- `VALID`: transport succeeds, serialized size is nonzero, and diagnostics may be empty;
- `INVALID`: transport succeeds, serialized size is zero, and verifier diagnostics are nonempty.

Query and write calls independently run LLVM verification/materialization. The host rejects a
provider whose sizes or status change between the two calls. Modules remain thread-confined, so a
caller cannot mutate a module concurrently between query and write.

No LLVM type, C++ object, exception, provider allocation, or provider-owned string crosses the
boundary. Diagnostic bytes are copied into a host `String`, including embedded NUL bytes.

## Interfaces and Dependencies

Extend `source/compiler-core/slang-nvvm-ir-builder-api.h` with fixed-width V2 status constants, the
new operation type, the composed `SlangNVVMBuilderAPI_V2`, its minimum required size, and the V2
getter name/type/declaration. The conceptual operation is:

```c
SlangNVVMResult_1 serializeModuleWithDiagnostics(
    SlangNVVMModuleHandle_1 module,
    SlangNVVMSerializationFormat_1 format,
    void* serializedDestination,
    size_t serializedDestinationSize,
    size_t* outSerializedSize,
    void* diagnosticDestination,
    size_t diagnosticDestinationSize,
    size_t* outDiagnosticSize,
    SlangNVVMVerificationStatus_2* outVerificationStatus);
```

`NVVMIRBuilder` keeps its existing V1 construction methods, adds V2 initialization and
`supportsSerializationDiagnostics()`, and adds a serialization overload returning a host-owned
diagnostic `String`. The existing overload remains usable with V1-only providers; the diagnostic
overload reports `SLANG_E_NOT_AVAILABLE` if V2 is unavailable.

`source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` owns the canonical verification and serialization
helper used by both versioned operations. The platform `.def`, ELF version script, and Mach-O
export list allow exactly the V1 and V2 getters. The strict C translation unit names both table
types so C compatibility stays part of every standalone module build.

No new external dependency is introduced. The ignored LLVM 14.0.6 source/build and CUDA 12.2
toolkit from Slice 3a remain the local validation environment.

## Milestones

### Milestone 1: Freeze V1 and prove V2 negotiation with a fake provider

Add the C declarations and host loading/validation before changing LLVM code. Extend the fake
library to expose both getters. Cover wrong V2 size/version, invalid nested V1, missing required V2
function, future-larger provider size, V2-present-but-broken without fallback, and V2-absent V1
fallback. Preserve direct V1 initialization behavior.

Promotion criteria: fake tests prove that the host never combines tables/providers, never hides a
broken advertised V2, and still accepts a conforming V1-only provider.

### Milestone 2: Prove the atomic byte protocol with fake valid and invalid results

Implement query/write copying in the host. Fake results cover valid bitcode, invalid verification
with an embedded-NUL diagnostic, insufficient serialized and diagnostic buffers with untouched
sentinels, invalid zero-sized/mismatched output, and query/write status or size changes.

Promotion criteria: every returned blob/string is host-owned, invalid verification is mapped only
after its diagnostic is copied, and malformed provider output is rejected.

### Milestone 3: Capture the real LLVM verifier result

Refactor the provider so V1 and V2 share one canonical materialization helper. Capture
`llvm::verifyModule` into local storage, explicitly flush it, and return `INVALID` without producing
assembly/bitcode. For valid input, serialize exactly as Slice 3a did and return `VALID` with no
diagnostic.

Promotion criteria: the real unterminated `uniqueKernel` module returns a host diagnostic with
stable LLVM 14 substrings and no blob. After `ret void`, the same module serializes with an empty
diagnostic. A direct real V2 call proves short buffers do not modify either supplied buffer.

### Milestone 4: Close the module load-order gap

Make the coexistence test self-spawn twice under an order environment variable using
`ScopedEnvVar`. Each child runs through
`-use-fully-isolated-test-server -server-count 1 -disable-retries -skip-api-detection`.

The LLVM-first child queries LLVM 21.1, loads/uses LLVM 14, and queries LLVM 21.1 again. The
NVVM-first child loads/uses LLVM 14, queries LLVM 21.1, then uses LLVM 14 again. A child path must
not call `SLANG_IGNORE_TEST`; the parent performs availability preflight so a child failure cannot
be mistaken for a skip.

Promotion criteria: both child processes exit successfully and produce nonempty LLVM 14 bitcode
on each side of the second module load.

### Milestone 5: Validate and distill durable conclusions

Build the standalone module and focused host targets, run the complete NVVM prefix and established
regressions, inspect exports/dependencies, and update `docs/design/nvvm-backend.md`. Inventory every
new helper/fallback/special case and apply the `AGENTS.md` input-shape audit before completion.

Promotion criteria: all required evidence below is recorded and no generated output outside
ignored `build/` is left in the worktree.

## Helper and Input-Shape Audit

The production helper inventory is intentionally small:

- `_isCompatibleV1` is the single source of truth for the frozen V1 table, reused by direct V1 and
  nested V2 initialization. It replaces duplicated validation rather than defining a second
  equivalence relation.
- `_isVerificationStatus` validates the only two statuses allowed after successful transport. It
  does not infer or repair a provider result.
- `_isSerializationFormat` centralizes the two supported wire values. V2 calls it before
  verification, while `_materializeModule` calls it after verification so V1 retains its committed
  invalid-input precedence. `nvvmIRBuilderRejectsInvalidOperations` locks down both results.
- `_materializeModule` owns LLVM's canonical verifier/serializer sequence for both ABI versions.
  It does not reconstruct LLVM diagnostics or patch IR. Its one fallback supplies stable text only
  if LLVM reports invalidity without emitting bytes, because V2's `INVALID` contract requires a
  useful nonempty diagnostic. The fake empty-diagnostic case proves the host rejects a provider
  that violates this invariant.
- `_fillBuilderAPIV1` constructs the one canonical V1 table used by both getters. The V2 getter
  embeds that table rather than maintaining a duplicate mapping.

The new host overload and `getAPIV2` expose already-negotiated capability; they do not add an
alternate module representation. A future-sized provider table is a canonical append-only ABI
shape. `NVVMIRBuilder::initialize(V2)` accepts its known prefix and clamps the retained
`structureSize`, so no consumer can mistake truncated local storage for the reported future tail.

Consider the real malformed test module: `declareFunction` creates `uniqueKernel`, `createBlock`
adds `entry`, and serialization occurs before `emitReturnVoid`. This unterminated module is an
intentional transient builder state, not malformed Slang IR from an upstream lowering pass; Slice
4 has not introduced one yet. LLVM's verifier at the serialization boundary is therefore the
correct producer of the failure and its diagnostic. The test then calls `emitReturnVoid` on that
same block and proves the canonical module becomes serializable without any consumer-side patch.

The mixed-destination and short-buffer shapes are valid ABI inputs. The provider must classify any
call with one destination as a write, validate all nonempty outputs, and fail before either copy.
The real sentinel tests fail if that atomic check is weakened. Conversely, `VALID` with zero
serialized bytes, `INVALID` with an empty diagnostic, and query/write size/status changes are
accidental provider spellings; the fake tests prove the host rejects each after the earliest
possible call rather than adding a fallback interpretation.

Test-only helpers (`_getRealNVVMBuilderLocation`, `_loadRealNVVMBuilder`,
`_populateEmptyNVVMKernel`, `_queryLLVM21`, `_buildCoexistenceProbe`,
`_exerciseNVVMLLVMCoexistence`, and `_reportCoexistenceChildFailure`) separate availability,
module construction, and fresh-process orchestration. They do not modify compiler state outside
their owned process. The scoped environment sentinel is restored after each synchronous child,
and the parent requires a 1/1 child summary so a missing selector cannot pass vacuously.

No new AST, Slang IR, `Val`, witness, substitution, lookup, or custom-equivalence helper appears in
this slice. The audit found no downstream special case compensating for a malformed upstream
representation.

## Validation and Acceptance

Required local evidence:

- the standalone strict C and C++ module builds against exactly LLVM 14.0.6;
- raw V1 remains queryable and functional;
- V2 size/version/nested-table/function validation has deterministic fake negative coverage;
- V2 absence falls back to V1, but malformed advertised V2 fails;
- fake diagnostics preserve exact pointer-plus-count bytes including embedded NULs;
- fake and real insufficient buffers report exact sizes and modify neither supplied buffer;
- real invalid LLVM IR returns a nonempty host diagnostic and no serialized blob;
- repairing that same module produces bitcode and an empty diagnostic;
- both LLVM load orders pass in separate fully isolated processes;
- generated bitcode still compiles through CUDA 12.2 libNVVM and its PTX passes `ptxas`;
- the focused NVVM prefix plus unclosable-library, downstream-version, and ordinary NVRTC
  regressions pass;
- the module exports exactly the V1/V2 getters and imports no LLVM DLL;
- builder-absent behavior remains optional and `git.exe diff --check` is clean.

Use Windows-native tools from the repository root. The core commands are:

```powershell
cmake.exe --build build/nvvm-builder-deps/slang-llvm-nvvm-build `
    --config Release --target slang-llvm-nvvm -- /m
cmake.exe --build build --config Debug --target compiler-core -- /m
cmake.exe --build build --config Debug --target slang-unit-test `
    -- /p:BuildProjectReferences=false /m

$env:SLANG_NVVM_BUILDER_PATH = `
    (Resolve-Path -LiteralPath `
        'build/nvvm-builder-deps/slang-llvm-nvvm-build/Release').Path
build/Debug/bin/slang-test.exe slang-unit-test-tool/nvvm
build/Debug/bin/slang-test.exe `
    -use-fully-isolated-test-server -server-count 1 -disable-retries -skip-api-detection `
    slang-unit-test-tool/nvvmIRBuilderCoexistsWithLLVM21

build/Debug/bin/slang-test.exe slang-unit-test-tool/unclosableSharedLibrary
build/Debug/bin/slang-test.exe slang-unit-test-tool/getDownstreamCompilerVersion
build/Debug/bin/slang-test.exe tests/cuda/sampler-comparison-state-unused
dumpbin.exe /exports build/nvvm-builder-deps/slang-llvm-nvvm-build/Release/slang-llvm-nvvm.dll
dumpbin.exe /dependents build/nvvm-builder-deps/slang-llvm-nvvm-build/Release/slang-llvm-nvvm.dll
git.exe diff --check
```

Update the exact coexistence test selector if implementation renames it. Record counts, ignored
tests, expected failures, and any sandbox/environment-specific reruns in Progress and Outcomes.

## Failure and Recovery

All LLVM and standalone-module build output stays under `build/nvvm-builder-deps/`. The ABI changes
are additive: removing the V2 getter/table, its host overload, and its tests restores the committed
Slice 3a behavior without affecting libNVVM discovery, NVRTC, or `slang-llvm`.

If V2 cannot return diagnostics without ambiguous buffer/result semantics, stop rather than add a
provider-owned pointer or mutable last-error fallback. Retain the V1 path and record the exact
counterexample here. If verifier wording varies, assert only stable structural fragments rather
than replacing nonempty LLVM output with a synthesized explanation. If isolated test-server
execution does not control load order, trace its global-session/downstream calls before adding a
new helper program.

Do not delete or modify the unrelated `external/slang-binaries/` directory. Do not copy the module
into `build/Debug/bin`; use `SLANG_NVVM_BUILDER_PATH` so default-absence behavior remains testable.

## Artifacts and Hand-Off

Keep the LLVM source/build, standalone module build, and test binaries only under ignored `build/`.
Do not retain generated PTX/cubin, diagnostic dumps, or temporary child-process files.

Before hand-off, update this plan's living sections and distill the settled V2/status/buffer and
load-order contracts into `docs/design/nvvm-backend.md`. Record the helper/input-shape audit and the
next Slice 4 prerequisites here for the eventual five-part PR description. This active plan is a
working log and must remain uncommitted.
