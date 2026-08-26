# Lower an empty Slang compute entry point through NVVM

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the active,
uncommitted working log for Slice 6 of the direct NVVM backend experiment.

## Purpose and Observable Result

After this slice, an ordinary Slang program with one empty compute entry point can select the
experimental NVVM route and produce PTX:

```slang
[numthreads(1, 1, 1)]
void computeMain()
{}
```

```text
slangc empty.slang -target ptx -profile cuda_sm_7_0 \
    -emit-cuda-via-nvvm -entry computeMain -stage compute -o empty.ptx
```

The compiler links and optimizes the requested program as PTX-targeted Slang IR, proves that the
selected entry point has the exact canonical empty-compute shape, maps that shape one-to-one onto
the existing LLVM 14 NVVM builder, serializes verified typed-pointer LLVM bitcode, and submits the
exact `ObjectCode + LLVMIR + Kernel` artifact to the already-registered libNVVM compiler. The
result is `ObjectCode + PTX + Kernel` containing `.entry computeMain`.

This is deliberately a narrow capability. A non-empty body such as
`GroupMemoryBarrierWithGroupSync()` must stop at a stable unsupported-Slang-IR diagnostic before
the optional builder or libNVVM is loaded. The established default and explicit-NVRTC routes are
unchanged, and explicit NVVM never falls back.

## Progress

- [x] (2026-08-26 15:10Z) Re-read `.agent/PLANS.md`, the durable NVVM design and capability ledger,
  the Slice 5 hand-off, and the current linked-IR, PTX dispatch, downstream-option, builder,
  provider, diagnostics, cache-hash, and test paths.
- [x] (2026-08-26 15:10Z) Completed independent read-only audits of the canonical IR producer
  boundary, optional builder/downstream artifact contract, and smallest deterministic acceptance
  matrix.
- [x] (2026-08-26 15:10Z) Created this bounded Slice 6 ExecPlan before implementation.
- [x] (2026-08-26 15:57Z) Added and tested the target-specific minimal-compute legality/lowering
  owner.
- [x] (2026-08-26 15:57Z) Connected verified bitcode to the existing NVVM downstream continuation
  without duplicating
  downstream option, architecture, diagnostic, association, or timing policy.
- [x] (2026-08-26 15:57Z) Added deterministic fake end-to-end success, cache-identity, and failure
  coverage plus real-provider and `ptxas` evidence.
- [x] (2026-08-26 15:57Z) Built and ran focused/regression tests outside the sandbox, formatted and
  self-reviewed the diff, and updated the durable design and capability ledger.

## Surprises and Discoveries

- Observation: the canonical final empty body is `IRReturn` carrying `IRVoidLit`, rendered as
  `return_val(void_constant)`, not a zero-operand return instruction.
  Evidence: a `slangc -dump-ir` probe of the Slice 5 diagnostic fixture and the existing C-like,
  SPIR-V, and VM emitters' handling of `IRReturn::getVal()`.
  Consequence: legality checks this exact semantic shape before emitting builder `ret void`.

- Observation: `linkAndOptimizeIR` is file-local to `source/slang/slang-emit.cpp`, while the Slice 5
  `CodeGenContext::emitNVVMForEntryPoints` stub lives in `slang-code-gen.cpp`.
  Evidence: definitions at `slang-emit.cpp:969` and `slang-code-gen.cpp:1084` in base `98de4336a`.
  Consequence: move the method definition beside the other direct emitters and keep the narrow
  Slang-IR-to-NVVM mapping in a new target-specific emitter.

- Observation: `emitEntryPointsSourceFromIR` performs C-like-only simplification and is not the
  canonical producer for a direct backend.
  Evidence: the C-like emission path calls `simplifyForEmit`, while direct SPIR-V and LLVM call
  `linkAndOptimizeIR` themselves.
  Consequence: use the base PTX `CodeGenContext` and never create a CUDA-source subcontext.

- Observation: the downstream continuation already owns target capability translation, including
  `_cuda_sm_7_0 -> CUDASM 7.0`, plus optimization/debug/floating-point options, diagnostics,
  metadata associations, and timing.
  Evidence: `CodeGenContext::emitWithDownstreamForEntryPoints` in
  `source/slang/slang-code-gen.cpp`.
  Consequence: extend that continuation to accept an already-produced LLVM-IR source artifact
  rather than constructing a second `DownstreamCompileOptions` policy.

- Observation: the ordinary file-test harness has no requirement bit for the independently
  optional `slang-llvm-nvvm` provider.
  Evidence: `TestRequirements` and `_extractSlangCTestRequirements` do not recognize
  `-emit-cuda-via-nvvm` or provider availability.
  Consequence: keep the checked-in file lane provider-independent and negative. Put deterministic
  success in an injected unit test and real-provider success in an optional unit test that treats
  an explicit broken provider path as failure.

- Observation: a PTX target without a CUDA SM capability reaches the NVVM compiler without a
  required architecture, which that compiler rejects intentionally.
  Evidence: the existing downstream compiler contract and a focused option audit.
  Consequence: positive Slice 6 tests select `cuda_sm_7_0` and prove `-arch=compute_70` reaches
  libNVVM. The lowerer does not invent a default architecture.

- Observation: once the builder produces real output, its binary identity affects the shader
  cache just as the downstream libNVVM binary does.
  Evidence: Slice 5 hashes only `NVVMDownstreamCompiler::getVersionString`; the provider owns the
  LLVM serialization that now influences PTX.
  Consequence: production discovery must expose/cache one builder identity and append it for the
  explicit direct-NVVM target. Absence may leave no optional identity in a hash, but codegen still
  fails honestly; replacing a successfully loaded provider must not reuse a stale key.

- Observation: `[numthreads(1, 1, 1)]` retains module-scope `integer_constant` instructions even
  though the selected entry body is otherwise empty.
  Evidence: the first real-provider probe reached E52017 on `integer_constant`; the linked module
  contains those constants as canonical operands of entry-point metadata rather than storage or
  executable code.
  Consequence: the validator permits `IRConstant` alongside hoistable representation nodes while
  still rejecting every other unselected semantic global. Removing that allowance makes the
  motivating empty kernel fail before builder creation.

- Observation: `cuda_sm_7_0` is a target capability, not a profile.
  Evidence: the first fake public-route test supplied it through `TargetDesc::profile`, so
  libNVVM correctly reported that no explicit compute architecture was present.
  Consequence: the public-API fixture now supplies `CompilerOptionName::Capability` and proves both
  verifier and compiler receive `-arch=compute_70`; production propagation required no repair.

- Observation: a linked component returned from a test helper does not independently keep its
  creating `ISession` alive.
  Evidence: a cache-hash fixture refactor released the local session before hashing and all direct
  tests failed or faulted.
  Consequence: the helper returns the session explicitly, matching existing component-test
  patterns. This was a test-lifetime issue, not a production backend issue.

## Decision Log

- Decision: accept only defined compute entry points returning void, with zero parameters, exactly
  one parameterless block, and only `IRReturn(IRVoidLit)`.
  Rationale: this is the exact canonical linked shape observed for the motivating source and every
  accepted semantic fact maps directly to a V1 builder operation. Parameters, calls, values, and
  control flow belong to Slice 7.
  Date/Author: 2026-08-26, Codex.
  Revisit when: Slice 7 supplies an ABI/control-flow contract and differential evidence.

- Decision: consume `LinkedIR.entryPoints` and the existing `IREntryPointDecoration` name.
  Rationale: linking already selects requested entries and applies entry-point renaming. Rescanning
  globals or reconstructing names would create a second source of truth.
  Date/Author: 2026-08-26, Codex.
  Revisit when: linked-IR ownership changes globally.

- Decision: validate the full supported shape before optional dependency discovery.
  Rationale: an unsupported Slang program should have the same diagnostic on every machine and
  must not become a placeholder empty kernel merely because builder/libNVVM availability differs.
  Date/Author: 2026-08-26, Codex.
  Revisit when: feature negotiation becomes an explicit pre-link contract.

- Decision: disable generic existential/resource legalization for the direct path.
  Rationale: CUDA-source resource legalization does not define an NVVM representation. This slice
  rejects those inputs instead of letting a consumer reinterpret a C++-oriented shape.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a later slice owns the corresponding NVVM ABI.

- Decision: require the V2 serialization-diagnostics prefix in production lowering even though
  the structural builder operations are all V1.
  Rationale: generated compiler IR must be verified and its LLVM verifier text must reach the user;
  silently serializing through a legacy provider is not an adequate production diagnostic
  boundary.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a deployment compatibility requirement explicitly permits V1-only providers and
  defines an equally observable validation path.

- Decision: reuse the existing downstream continuation with an explicit LLVM artifact and NVVM
  compiler selection.
  Rationale: architecture, effective options, timing, artifact associations, and downstream
  diagnostics already have one owner. A second continuation would drift immediately.
  Date/Author: 2026-08-26, Codex.
  Revisit when: direct backends gain a shared first-class intermediate-artifact pipeline.

- Decision: make direct NVVM a target-program query used by intermediate-language policy.
  Rationale: PTX normally implies CUDA source today. Direct NVVM must not inherit C++ l-value-cast
  decisions merely because both routes end at PTX; the empty shape is inert, but this invariant is
  required before Slice 7 expands the subset.
  Date/Author: 2026-08-26, Codex.
  Revisit when: intermediate representation policy is no longer expressed as a source language.

## Outcomes and Retrospective

Slice 6 is complete. An explicit direct-NVVM PTX target now links ordinary Slang IR, validates the
exact empty compute subset, emits verified LLVM 14 typed-pointer bitcode through the optional V2
builder, and passes `ObjectCode + LLVMIR + Kernel` through the registered libNVVM compiler. The
real result was 291 bytes of PTX 8.8 targeting `sm_70`, with `.visible .entry computeMain()`, and
CUDA 12.9 `ptxas` accepted it.

The deterministic lane proves the exact builder operation sequence and lifetimes, byte-for-byte
bitcode handoff, `compute_70` options, a consistent `computeMain` fake PTX result, V2 verifier text,
unsupported-`call` rejection before module/program creation, missing-builder E52016 behavior with
no fallback, and that builder availability changes the shader hash while hash and codegen reuse
one session load result. With the provider absent, 5/5 fake tests passed and 2 real tests were
ignored. With the Release LLVM 14.0.6 provider selected, the complete NVVM prefix passed 40/40,
including both real ordinary-Slang tests and `ptxas` acceptance. The provider-independent file
negative, option parser, two routing/hash tests, invalid-option test, three NVRTC sampler lanes, and
two explicit pass-through lanes also passed.

The self-review inventory retained five deliberate boundaries: the effective target-program
query, the named canonical-shape validator, scoped module destruction, the explicit downstream
artifact continuation, and session-owned builder discovery/cache identity. The only shape-specific
allowance is `IRConstant`: `[numthreads]` canonically produces those module operands, they are not
omitted storage or code, and the positive test fails without the allowance. The diff introduces no
custom IR/`Val` equivalence, operand-graph search, syntax reconstruction, C-like repair, fallback,
or silent default. Parameters and all non-empty behavior remain the first Slice 7 boundary.

## Context and Current Pipeline

Consider the motivating source:

```slang
[numthreads(1, 1, 1)]
void computeMain()
{}
```

Slice 5 parses `-emit-cuda-via-nvvm` into one canonical `EmitCUDAMethod` option. The PTX arm of
`CodeGenContext::_emitEntryPoints` reads the effective `TargetProgram` option set and calls
`emitNVVMForEntryPoints`; at base `98de4336a`, that method emits E52014 without linking IR.

The new method remains on the PTX context and calls `linkAndOptimizeIR`. `linkIR` clones only the
requested program and records the selected `IRFunc*` values in `LinkedIR.entryPoints`; its entry
specialization already preserves the checked stage and applies name overrides to
`IREntryPointDecoration`. The shared link/optimization pipeline then specializes and cleans the
Slang IR. For the example, the final selected function is `Func(Void)`, has no parameters, one
block, and only `IRReturn(IRVoidLit)`.

The new NVVM emitter validates that exact representation. Only after validation does the global
session load the optional `slang-llvm-nvvm` module from `SLANG_NVVM_BUILDER_PATH` or its logical
name using the configured shared-library loader. The emitter creates an NVPTX64/NVVM 2.0 module,
declares the exact linked entry name as `void()`, appends `entry`, emits `ret void`, marks the
function as a kernel, and asks V2 to verify and serialize bitcode. The host wraps those bytes in
`ObjectCode + LLVMIR + Kernel` and retains linked post-emit metadata.

Finally, the existing downstream continuation discovers `PassThroughMode::NVVM`, passes the LLVM
artifact with `SourceLanguage::LLVM`, translates target capabilities to libNVVM options, compiles,
extracts downstream diagnostics before inspecting the result, preserves source associations, and
returns PTX. The default/NVRTC and true pass-through branches do not use this producer.

## Scope and Non-Goals

In scope:

- canonical link-and-optimize entry into the direct PTX path;
- exact empty, zero-parameter compute entry-point validation and lowering;
- LLVM 14 provider discovery/lifetime/identity through the global session;
- V2 verifier diagnostics and exact LLVM-bitcode artifact creation;
- reuse of the registered NVVM downstream continuation;
- deterministic fake end-to-end success and negative tests;
- optional real provider/libNVVM/PTX/`ptxas` evidence;
- stable diagnostics and durable design/ledger updates; and
- default NVRTC and Slice 5 routing regressions.

Non-goals:

- scalar parameters, pointer ABI, loads/stores, arithmetic, calls, branches, loops, or phis;
- builtins, barriers, launch-bounds metadata, globals, resources, address spaces, aggregates, or
  shared memory;
- vertex/ray/OptiX stages, CUDA graph/device code, RDC, LTO, libdevice, or debug metadata;
- changing the default PTX route or public target enum;
- making provider availability an unconditional file-test requirement;
- packaging the optional LLVM 14 module; and
- comparing PTX text byte-for-byte with NVRTC.

## Architecture and Invariants

`TargetProgram` owns effective emission-method selection. `CodeGenContext` owns orchestration.
`linkAndOptimizeIR` owns canonical linked Slang IR. `LinkedIR.entryPoints` owns selected-entry
identity. The new NVVM emitter owns only legality and one-to-one lowering into the opaque builder
ABI. `NVVMIRBuilder` owns provider ABI validation and retains the provider library. The global
session owns the lazily cached builder because both cache identity and code generation consume it.
`NVVMDownstreamCompiler` remains the only owner of libNVVM options and compilation.

The builder outlives every module; each module is destroyed on every exit and before the builder is
released. Modules remain thread-confined. No LLVM type, object, allocator, or exception crosses
the ABI. Production lowering requires exact LLVM 14.0.6, NVVM IR 2.0, typed pointers, NVPTX64, and
the V2 verifier prefix.

The emitter does not mutate or repair Slang IR. It rejects any selected function that is not the
canonical supported shape and rejects live non-hoistable module state it cannot emit. Hoistable
types, layouts, constants, and capability declarations may remain because the accepted function
does not require emitted storage or code for them. Unsupported legality is checked before the
builder and downstream compiler so no operation can be silently dropped.

### Input-shape and special-case audit

Planned new helpers/special cases:

- `TargetProgram::shouldEmitNVVMDirectly`: survives as the one effective-option representation
  query; it prevents CUDA-source policy from leaking into the direct path.
- minimal-compute validator: survives as the named legality boundary. Its concrete producer is
  `linkAndOptimizeIR`, its accepted shape is canonical, and the barrier-call negative fails if the
  validator is removed.
- scoped builder module: survives as an ownership boundary. It performs no semantic repair.
- downstream source-artifact override: survives as a representation-preserving continuation; it
  uses the exact builder artifact and does not reconstruct source.
- optional-builder loader/cache: survives as dependency ownership and cache identity; it does not
  turn absence into fallback.

No custom `Val`/IR equivalence, arbitrary operand-graph traversal, syntax reconstruction, generic
argument inference, C-like rewrite, placeholder kernel, or silent default is permitted.

## Interfaces and Dependencies

Add a target-specific internal interface in `source/slang/slang-emit-nvvm.h/.cpp` accepting
`CodeGenContext`, `LinkedIR`, and an initialized `NVVMIRBuilder`, and returning an artifact. Keep the
API internal and LLVM-free.

Extend `CodeGenContext::emitWithDownstreamForEntryPoints` with an optional explicit source-artifact
description sufficient to say: compiler NVVM, source LLVM, artifact already produced. Existing
callers retain their defaults and behavior. The override is invalid during true pass-through and
must not invoke CUDA-source emission.

Expose a session-internal lazy builder accessor and a builder identity string. The identity includes
the frozen ABI/version/pointer-model facts, retained V2 prefix/capabilities, and loaded module file
timestamp derived from a provider function pointer. `_setSharedLibraryLoader` clears the cached
builder alongside downstream compilers.

Add stable diagnostics after E52015 for optional builder unavailable/incompatible, unsupported
linked Slang IR shape/opcode, and builder emission/verification failure. V2 verifier bytes are
attached as raw diagnostic detail and are never discarded.

External requirements for real success are the separately built Release LLVM 14 provider and a
CUDA toolkit containing compatible libNVVM. `cuda_sm_7_0` supplies `compute_70`; optional `ptxas`
validation uses the matching CUDA toolkit executable.

## Milestones

1. Establish direct-target policy and legality.
   Modify `slang-target-program.h`, `slang-type-layout.cpp`, `slang-emit.cpp`, and add
   `slang-emit-nvvm.h/.cpp`. A fake/in-memory test proves the canonical empty body is accepted and
   the barrier call is rejected before builder use.

2. Produce verified NVVM bitcode with correct ownership and diagnostics.
   Add session-owned provider discovery/identity and map the accepted function through the V1
   structural operations plus required V2 serialization. Tests prove exact call sequence, entry
   name, bitcode bytes, module destruction, verifier text, and no mutation on failure.

3. Reuse downstream compilation and cross the complete public route.
   Extend the existing downstream continuation to accept the exact LLVM artifact. A combined fake
   loader drives ordinary Slang source through target selection, linked IR, builder, registered
   libNVVM, and PTX. It proves `-arch=compute_70`, artifact bytes/descriptors, and no E52014.

4. Preserve honest boundaries and established behavior.
   Replace the Slice 5 E52014 file expectation with a stable unsupported-`call` expectation, update
   the link-options routing/hash test, and run default NVRTC, explicit NVRTC, true pass-through,
   option parsing, invalid-option, component-hash, and downstream-version regressions.

5. Collect real evidence and hand off.
   With `SLANG_NVVM_BUILDER_PATH` set to the local Release provider, compile the ordinary empty
   Slang entry through real libNVVM, check `.entry computeMain`, optionally assemble it with
   `ptxas`, run the full NVVM unit prefix, format, inspect the diff, update design/ledger, and record
   outcomes here.

## Validation and Acceptance

All CMake builds and tests run outside the sandbox, as required by repository instructions. Use
Windows-native tools from `C:\src\slang`.

Focused build:

```text
cmake.exe --build --preset debug --target slang-test
```

Focused tests (exact names finalized with implementation):

```text
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvmSlangEmptyCompute
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvmSlangUnsupportedIR
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvmSlangBuilderDiagnostics
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvmSlangBuilderIdentity
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/cudaEmissionMethod
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/nvvm-unsupported-ir
```

Real provider and complete prefix:

```text
$env:SLANG_NVVM_BUILDER_PATH = \
  'C:\src\slang\build\nvvm-builder-deps\slang-llvm-nvvm-build\Release'
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvmSlangRealEmptyCompute
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm
```

Relevant regressions:

```text
build\Debug\bin\slang-test.exe -skip-api-detection slang-unit-test-tool/parseCUDAEmissionMethods
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/sampler-comparison-state-unused
build\Debug\bin\slang-test.exe -skip-api-detection tests/cuda/cuda-compile
```

Acceptance requires deterministic fake proof of the public route, provider-independent rejection of
non-empty IR before dependency use, verifier-diagnostic propagation, optional real `.entry
computeMain` evidence, `ptxas` acceptance when present, unchanged NVRTC/pass-through results, no
format diff under pinned clang-format 17, and clean `git diff --check`.

## Failure and Recovery

All new code is gated behind explicit `SLANG_EMIT_CUDA_VIA_NVVM`; deleting the new direct emitter
and restoring E52014 returns to the Slice 5 boundary without changing default PTX. Linking and
validation occur before loading dependencies, so unsupported-source failures are reproducible.
Builder modules use scope cleanup, so every failed operation is safely rerunnable.

An absent/incompatible default provider yields the named provider diagnostic. An explicit broken
`SLANG_NVVM_BUILDER_PATH` is always a failure, never a skipped or logical-name fallback. Missing
libNVVM uses the existing downstream-compiler-not-found path. Missing architecture remains a
downstream input diagnostic. V2 verifier invalidity leaves no bitcode artifact and forwards the
captured text. No failure is allowed to retry through NVRTC.

Do not remove `external/slang-binaries/`; it is unrelated untracked workspace state. Do not commit
this ExecPlan.

## Artifacts and Hand-Off

Keep this plan current with exact commands, counts, diagnostics, rejected alternatives, and local
tool versions. Distill stable architecture into `docs/design/nvvm-backend.md`, capability status
into `docs/design/nvvm-backend-capability-ledger.md`, and the implementation narrative into the
required five-part PR description. The next slice must start from the first rejected scalar/ABI or
control-flow shape and must not assume the existing mixed CUDA pipeline is already correct for
NVVM.
