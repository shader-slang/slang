# Prove scalar and pointer kernels through the LLVM 14 NVVM builder

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. It is the active,
uncommitted working log for Slice 4 of the direct NVVM backend experiment.

## Purpose and Observable Result

After this slice, the optional LLVM 14 NVVM builder can construct two non-empty kernels without
textual LLVM IR:

```cpp
extern "C" __global__ void writeScalar(int* destination, int value)
{
    *destination = value;
}

extern "C" __global__ void copyScalar(int* destination, const int* source)
{
    *destination = *source;
}
```

The test suite builds equivalent typed-pointer NVVM IR through the private builder ABI, serializes
bitcode, compiles it through libNVVM, and compares the resulting PTX with PTX produced by compiling
the CUDA source through NVRTC. The observable gate is semantic: both routes expose the same entry
names and parameter widths/order, `writeScalar` performs a 32-bit global store, `copyScalar`
performs a 32-bit global load and store, and the configured CUDA toolkit's `ptxas` accepts both
results.
Exact PTX text is deliberately not compared.

This is still a builder and differential-evidence slice. It does not route ordinary Slang PTX
requests to libNVVM and does not traverse or lower Slang IR.

## Progress

- [x] (2026-08-26 12:08Z) Re-read `.agent/PLANS.md`, the durable NVVM design, the Slice 3b hand-off,
  the current V2 ABI/provider/host implementation, and the existing real test helpers.
- [x] (2026-08-26 12:08Z) Audited candidate reference kernels and the LLVM 14 NVPTX address-space
  behavior. Address space 1 is the initial global-memory spelling to prototype.
- [x] (2026-08-26 12:08Z) Created this bounded Slice 4 ExecPlan before implementation.
- [x] (2026-08-26 12:18Z) Appended the provisional five-operation V2 tail, added the strict-C size
  probe, corrected host prefix retention, and implemented host wrappers. The Debug `compiler-core`
  target builds successfully outside the sandbox.
- [x] (2026-08-26 12:22Z) Implemented the five LLVM 14 provider operations and their validation.
  The standalone Release provider, including its strict C ABI translation unit, builds successfully
  outside the sandbox against LLVM 14.0.6.
- [x] (2026-08-26 12:31Z) Negotiated the coherent scalar-memory capability in V2 without changing
  V1 or the frozen Slice 3b V2 prefix.
- [x] (2026-08-26 12:38Z) Prototyped both raw CUDA/NVRTC and builder-generated address-space-1
  kernels at `compute_75`. Both routes expose the intended ABI and global-memory operations, and
  CUDA 12.9 `ptxas` accepts every resulting entry.
- [x] (2026-08-26 12:38Z) Added fake negotiation/forwarding/negative tests and real
  builder/diagnostic tests, including failure-after-write output sanitation and no-mutation
  coverage.
- [x] (2026-08-26 12:38Z) Added NVRTC-versus-libNVVM PTX normalization and CUDA 12.9 `ptxas`
  acceptance tests for both reference kernels and both compiler routes.
- [x] (2026-08-26 12:43Z) Built the standalone provider and host tests outside the sandbox, ran
  focused and regression gates outside the sandbox, inspected the binary boundary, formatted the
  changed code, and completed adversarial production/test self-review.
- [x] (2026-08-26 12:46Z) Distilled the frozen contract and measured results into
  `docs/design/nvvm-backend.md`, then completed the outcomes and hand-off here.

## Surprises and Discoveries

- Observation: `NVVMIRBuilder::initialize(const SlangNVVMBuilderAPI_V2&, ...)` currently copies the
  provider table and then replaces its reported `structureSize` with the local `sizeof`. Once V2
  grows, that would make a Slice 3b-only provider appear to contain the new tail.
  Evidence: `source/compiler-core/slang-nvvm-ir-builder.cpp:112-115`.
  Consequence: preserve `min(providerReportedSize, sizeof(localTable))` and make capability tests
  require both that retained size and every function in the coherent tail.

- Observation: the bundled LLVM 14 NVPTX tests distinguish generic and global kernel pointers.
  `build/nvvm-builder-deps/llvm-project/llvm/test/CodeGen/NVPTX/lower-kernel-ptr-arg.ll` shows an
  address-space-0 parameter requiring a global conversion, while address space 1 lowers directly
  to global memory operations. `ld-addrspace.ll` and `st-addrspace.ll` show address-space-1 `i32`
  loads/stores lowering to the expected global instruction family.
  Consequence: prototype the reference kernels as `i32 addrspace(1)*`; keep explicit address-space
  construction general enough for later lowering rather than inserting pointer/integer casts.

- Observation: no existing checked-in `writeScalar` or `copyScalar` test can serve as the oracle.
  The nearest Slang CUDA tests already require resource, indexing, arithmetic, or thread-ID
  lowering that belongs to later slices.
  Consequence: keep exact raw CUDA source in the unit test and mirror it through the builder.

- Observation: on this machine `slang-test` exits during eager WGPU probing before selected tests
  run. The established focused invocation succeeds with `-skip-api-detection`.
  Consequence: every focused command in this plan uses that flag; this is a machine test-runner
  workaround, not NVVM product behavior.

- Observation: `NVRTCDownstreamCompilerUtil::locateCompilers` interprets a nonempty path as an
  exact NVRTC shared-library path, unlike the NVVM locator's toolkit-root input. Default discovery
  can also prefer an NVRTC library beside the test executable before `CUDA_PATH`, while the
  existing `ptxas` helper is deliberately rooted at `CUDA_PATH`.
  Consequence: this slice uses established default NVRTC discovery and records actual `ptxas`
  acceptance, but does not claim that discovery proves all three tools came from one toolkit.
  Strict toolkit-root selection needs its own discovery slice and regression matrix.

## Decision Log

- Decision: freeze `writeScalar(int*, int)` and `copyScalar(int*, const int*)` as the first two
  reference shapes.
  Rationale: together they exercise a scalar value parameter, pointer parameters, parameter
  lookup, a load, a store, kernel annotations, serialization, libNVVM, and PTX assembly without
  requiring constants, GEP, casts, arithmetic, or control flow.
  Date/Author: 2026-08-26, Codex.
  Revisit when: the prototype shows that this signature is not accepted NVVM IR 2.0 or cannot be
  compared stably with NVRTC.

- Decision: represent both kernel pointer parameters as typed `i32 addrspace(1)*` values.
  Rationale: the source contract says these pointers refer to global device memory, NVVM address
  space 1 preserves that provenance, and LLVM 14 lowers it directly to global operations. Generic
  pointers remain a legal address-space enum value for later Slang IR lowering.
  Date/Author: 2026-08-26, Codex.
  Revisit when: real libNVVM/NVRTC evidence shows an incompatible parameter ABI or rejects the
  module.

- Decision: append one all-or-nothing scalar-memory capability tail to V2; do not add V3 or a new
  getter.
  Rationale: Slice 3b explicitly made V2 append-only and froze only its diagnostic prefix. Old
  providers remain valid for empty kernels, while callers test the complete new prefix before
  using any new operation.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a concrete compatibility case cannot be expressed by size plus non-null function
  validation.

- Decision: compare normalized ABI and instruction families, then require `ptxas`; do not compare
  PTX bytes, register names, parameter names, whitespace, PTX version, or register counts.
  Rationale: NVRTC and libNVVM are different front doors to NVIDIA's toolchain and are allowed to
  format or optimize differently. Entry ABI, memory semantics, and assembler acceptance are the
  stable correctness observations for this slice.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a later performance slice defines a separately measured code-quality gate.

- Decision: keep runtime GPU execution outside the mandatory Slice 4 gate.
  Rationale: the design hand-off asks first for NVRTC-versus-libNVVM PTX evidence. Runtime launch
  adds CUDA driver/device availability and belongs to the later layered validation strategy.
  Date/Author: 2026-08-26, Codex.
  Revisit when: static differential evidence exposes an ambiguity that only execution can resolve.

- Decision: reject a provider size strictly between the frozen Slice 3b and Slice 4 minima.
  Rationale: the five appended function fields are one coherent capability, and no independently
  released intermediate prefix exists. Treating an impossible byte prefix as diagnostics-only
  would hide provider packing/version mistakes.
  Date/Author: 2026-08-26, Codex.
  Revisit when: a future ABI intentionally publishes an independently usable intermediate prefix.

## Outcomes and Retrospective

Slice 4 is complete. The V2 ABI now has one coherent five-operation scalar-memory tail while V1
and the Slice 3b V2 prefix remain compatible. The host retains the provider's real reported size,
rejects impossible partial prefixes, validates the entire tail before advertising capability, and
sanitizes output handles on every failure path. The LLVM 14 provider creates signless integer and
typed-pointer types, obtains parameters, and emits direct aligned loads/stores only after checking
module/context ownership, the insertion block, pointer/value types, address space, and alignment.

The exact `writeScalar` and `copyScalar` kernels verify as typed `i32 addrspace(1)*` NVVM IR,
serialize as assembly and bitcode, compile through libNVVM for `compute_75`, and produce PTX with
the intended `[64, 32]` and `[64, 64]` parameter widths and global-memory instruction families.
NVRTC compilation of the exact CUDA source produces the same entry ABI and memory semantics.
CUDA 12.9 `ptxas` 12.9.86 accepts all four route/kernel combinations for `sm_75`.

The final outside-sandbox validation evidence is:

- standalone Release `slang-llvm-nvvm`, Debug `compiler-core`, and Debug `slang-unit-test` builds
  all succeeded against pinned LLVM 14.0.6;
- the complete NVVM unit prefix passed 32/32 with the provider selected;
- with the provider absent, all four fake-only tests passed and ten real-provider tests ignored;
- an explicit nonexistent provider path failed the selected real scalar test, proving it was not
  converted into a skip;
- the fully isolated LLVM coexistence selector, `unclosableSharedLibrary`,
  `getDownstreamCompilerVersion`, and both variants of
  `tests/cuda/sampler-comparison-state-unused` passed;
- the provider exports exactly the V1 and V2 getters, has no LLVM DLL dependency, and retains only
  its expected Windows system imports; and
- `clang-format --dry-run --Werror` and `git diff --check` passed for the changed code.

No generated PTX, cubin, or diagnostic log is retained. The test helpers use temporary storage, so
individual PTX byte sizes and `ptxas -v` resource summaries were not persisted and are not gates
for this semantic slice. Ordinary PTX routing remains unchanged on NVRTC. The remaining discovery
caveat is explicit: default NVRTC lookup may choose an executable-adjacent DLL while libNVVM and
`ptxas` use the configured CUDA toolkit. Actual assembly proves compatibility on this machine, but
strict three-tool toolkit identity remains a separate discovery slice.

## Context and Current Pipeline

Slice 3b established this tested path:

```text
NVVMIRBuilder construction calls
    -> slang-llvm-nvvm built against exactly LLVM 14.0.6
    -> LLVM verifier and host-owned assembly/bitcode bytes
    -> NVVMDownstreamCompiler
    -> CUDA libNVVM
    -> PTX
    -> ptxas
```

The builder currently creates a fresh `LLVMContext`, `Module`, and `IRBuilder` per module in
`source/slang-llvm-nvvm/slang-llvm-nvvm.cpp::ModuleState`. Module construction owns the NVPTX64
triple, the NVVM 64-bit data layout, and `!nvvmir.version`. Existing V1 calls create void function
types, functions, blocks, returns, and kernel annotations. V2 composes the complete V1 table and
adds atomic serialization plus verifier bytes. LLVM objects cross the shared-library boundary only
as opaque handles and remain owned by their module.

For `writeScalar`, the new producer-to-consumer trace will be:

```text
getIntegerType(32) + getPointerType(i32, global)
    -> getFunctionType(void, [global-i32-pointer, i32])
    -> declareFunction("writeScalar")
    -> getFunctionParameter(0 and 1)
    -> createBlock + setInsertBlock
    -> emitStore(value, destination, alignment 4)
    -> emitReturnVoid + markFunctionAsKernel
    -> serialize bitcode with verifier diagnostic
    -> libNVVM compute_75 -> PTX -> ptxas sm_75
```

`copyScalar` replaces the scalar parameter with a second global pointer, then emits one aligned
load and one aligned store. Raw CUDA source is compiled separately through NVRTC for the same
virtual architecture and becomes the differential oracle. No Slang AST or IR producer exists in
this slice, so invalid handles/shapes are ABI inputs and are rejected at the builder boundary.

The public NVVM IR 2.0 contract used here is: address space 1 denotes global memory; typed-pointer
loads/stores in supported address spaces are legal; address-space conversions, when later needed,
must use `addrspacecast`; and `!nvvm.annotations` marks kernels. The implementation targets the
already-frozen LLVM 14.0.6 typed-pointer dialect and 64-bit NVPTX module envelope.

## Scope and Non-Goals

In scope:

- an appended V2 capability for signless integer types, typed pointers with an explicit legal
  NVVM address space, function-parameter lookup, and aligned non-volatile load/store;
- host wrappers that clear outputs, report `SLANG_E_NOT_AVAILABLE` for an old provider, and reject
  provider success without the required output handle;
- provider-side module/context/type/value/insertion-point validation before mutating LLVM IR;
- fake tests for old/new negotiation and argument forwarding without LLVM or CUDA;
- real LLVM assembly and bitcode construction of both reference kernels;
- raw CUDA/NVRTC versus builder/libNVVM normalized PTX comparison at `compute_75`;
- configured-toolkit `ptxas -arch=sm_75 -v` acceptance for both routes;
- existing V1, Slice 3b diagnostics, coexistence, compiler, NVRTC, and absence behavior; and
- strict C/C++ compilation and binary export/dependency checks.

Non-goals:

- Slang IR traversal or lowering, CUDA emission-method routing, a public NVVM IR target, or making
  libNVVM the default PTX route;
- floating-point types, constants, GEP, casts, pointer arithmetic, attributes, arithmetic,
  comparisons, branches, phi nodes, loops, calls, or additional terminators;
- shared/constant/local-memory behavior beyond recognizing their address-space enum values;
- resources, intrinsics, libdevice, atomics, waves, autodiff, debug metadata, or optimization work;
- exact PTX equality, performance thresholds, runtime CUDA execution, packaging, or CI; and
- refactoring CUDA toolkit discovery shared by NVRTC/libNVVM beyond what the tests need.

## Architecture and Invariants

V1 remains byte-for-byte frozen. The existing Slice 3b V2 minimum through
`serializeModuleWithDiagnostics` also remains frozen. A second constant names the size through the
last required Slice 4 operation. A provider reporting only the old size is valid and retains empty-
kernel plus diagnostics behavior. A provider reporting the complete new prefix must supply every
new operation or initialization fails with `SLANG_E_NO_INTERFACE`. Future/larger providers remain
valid, and the host retains only `min(reportedSize, sizeof(localTable))`. Because the five fields
are one coherent capability rather than independently released prefixes, a reported size strictly
between the Slice 3b and Slice 4 minima is malformed and is rejected.

The new ABI carries no LLVM types, C++ objects, exceptions, provider allocations, or provider-owned
strings. Integer types are LLVM signless integer bit patterns. Pointer provenance is represented by
the typed pointer's NVVM address space; integer round trips are forbidden. All returned handles
remain context-compatible with and live for their module. Each module has a unique LLVM context,
so context identity is the canonical type/value ownership boundary.

Loads/stores are non-volatile in this slice. They require a current, unterminated insertion block
owned by the supplied module, a pointer value from that module, matching pointee/value types for a
store, and a nonzero power-of-two byte alignment. The reference kernels use alignment 4. Failed
operations validate their complete input shape before inserting any instruction. LLVM verification
remains the final structural authority; the provider never repairs malformed IR.

The normalized PTX comparison extracts one named entry at a time. It checks parameter order and
widths plus required load/store instruction families and deliberately ignores tool-generated names
and layout. `ptxas` is the canonical syntax/target acceptance boundary. Both routes use the same
explicit `compute_75`/`sm_75` policy and integer-only semantics, so floating-point defaults cannot
confound the result.

Ordinary `SLANG_PTX` requests continue using NVRTC. The optional module stays dynamically loaded
and retains exactly the V1 and V2 exports.

## Interfaces and Dependencies

Append these conceptual operations to `SlangNVVMBuilderAPI_V2` in
`source/compiler-core/slang-nvvm-ir-builder-api.h`:

```c
getIntegerType(module, bitWidth, outType)
getPointerType(module, pointeeType, addressSpace, outType)
getFunctionParameter(module, function, parameterIndex, outValue)
emitLoad(module, pointer, alignment, outValue)
emitStore(module, value, pointer, alignment)
```

The final typedefs use fixed-width ABI fields plus `size_t` indices and the existing opaque handle
types/result convention. `SlangNVVMAddressSpace_2` exposes the NVVM values generic/code 0, global
1, shared 3, constant 4, and local 5. The accepted integer-width set and exact load/store argument
contract must be documented beside the typedefs and locked down by negative tests before the ABI
tail is treated as frozen. No new getter or exported symbol is added.

`source/compiler-core/slang-nvvm-ir-builder.{h,cpp}` gains `supportsScalarOperations()` and one
wrapper per operation. `source/slang-llvm-nvvm/slang-llvm-nvvm.cpp` implements them using LLVM 14
typed pointers and `IRBuilder` aligned load/store construction. The strict-C probe in
`source/slang-llvm-nvvm/slang-nvvm-ir-builder-api-c.c` checks both frozen prefix sizes.

`tools/slang-unit-test/unit-test-nvvm-compiler.cpp` extends the fake table and real provider tests.
The real differential side uses the existing `NVVMDownstreamCompiler`/artifact helpers and the
existing NVRTC downstream compiler, with the CUDA source held in test-local host storage. It uses
the established locators rather than introducing another discovery contract: the NVVM and
`ptxas` paths are rooted at `CUDA_PATH`, while default NVRTC discovery can choose an
executable-adjacent library first. Assembly proves that the selected tools interoperate, but does
not prove strict toolkit identity.

Authoritative external references:

- NVIDIA NVVM IR specification: `https://docs.nvidia.com/cuda/nvvm-ir-spec/index.html`.
- NVIDIA libNVVM sample overview: `https://github.com/NVIDIA/cuda-samples/tree/master/Samples/7_libNVVM`.

Local dependencies already prepared by prior slices are LLVM 14.0.6 under
`build/nvvm-builder-deps/llvm-project`, its static Release build, the standalone Release provider,
the Debug Slang build, CUDA libNVVM/NVRTC, and `ptxas`.

## Milestones

### Milestone 1: Prototype and freeze the reference ABI

Compile the exact CUDA source through NVRTC for `compute_75`. Build the equivalent AS1 typed-pointer
module through the smallest provisional builder operations, serialize it, and compile through
libNVVM. Inspect named entry parameter widths/order and global memory instruction families, then
run both PTX blobs through `ptxas -arch=sm_75 -v`.

Promotion criteria: both routes have the intended entry ABI, global operations, and assembler
acceptance. Promote the provisional operations as the V2 tail and preserve the evidence in unit
tests. Discard criteria: if AS1 is rejected or changes the external ABI, remove the provisional
tail, record the exact PTX/verifier/compiler evidence, and prototype AS0 plus an explicit
`addrspacecast`; do not add an integer-pointer conversion or a consumer-side PTX workaround.

### Milestone 2: Negotiate the V2 scalar capability safely

Freeze the Slice 3b size constant, add a Slice 4 size constant, preserve the provider-reported size
in the host, and extend the provider's bounded copy. Add fake tests for old Slice 3b size, complete
new size, one missing advertised function, and a future-larger table. Prove scalar wrappers return
`SLANG_E_NOT_AVAILABLE` against V1 and old-V2 providers and that an old-capacity query to a new
provider leaves trailing sentinels untouched.

Promotion criteria: old host/new provider and new host/old provider work; malformed advertised
tails fail instead of silently degrading; no code reads beyond the retained prefix.

### Milestone 3: Implement and validate scalar-memory operations

Add host wrappers and provider calls for the five operations. Extend the fake provider with exact
argument/call-count/output forwarding checks. Add real negative coverage for null outputs,
unsupported integer widths/address spaces, cross-module handles, an out-of-range parameter,
non-pointer load/store, mismatched pointee/value type, no insertion point, and a terminated block.

Promotion criteria: all invalid shapes fail before mutation, all successful calls return a non-null
module-owned handle where applicable, and existing V1/Slice 3b tests remain unchanged.

### Milestone 4: Construct and verify both kernels

Build `writeScalar` and `copyScalar` with only the new primitives plus existing V1 operations.
Serialize assembly with diagnostics and check typed AS1 signatures, aligned load/store shapes,
entry names, `!nvvm.annotations`, and empty verifier text. Serialize bitcode and compile it through
real libNVVM for `compute_75`.

Promotion criteria: LLVM verifies both modules and libNVVM returns nonempty PTX.

### Milestone 5: Preserve differential and assembler evidence

Compile the raw CUDA source through NVRTC, normalize each named PTX entry, compare parameter widths
and order, assert required global-memory instruction families, and run the NVRTC and libNVVM PTX
through the configured CUDA toolkit's `ptxas`.

Promotion criteria: the stable semantic observations match and all four entry/artifact combinations
are accepted. Generated PTX/cubin and logs stay in test-owned temporary storage.

### Milestone 6: Regress, audit, and distill

Run the complete NVVM unit prefix, isolated LLVM load-order test, established downstream/NVRTC
regressions, provider-absent behavior, strict standalone build, exports/dependencies, and diff
checks. Inventory each helper/fallback/special case and perform the required input-shape audit.
Update the durable design with the frozen scalar ABI and measured evidence.

Promotion criteria: every acceptance item below has recorded evidence, ordinary PTX still uses
NVRTC, and the worktree contains only intended source/design/test edits plus the pre-existing
untracked `external/slang-binaries/` directory and this active plan.

## Helper and Input-Shape Audit

The final inventory and verdicts are:

- `NVVMIRBuilder::_supportsScalarOperations` survives as the single source of truth for the
  coherent appended prefix. Its input is the bounded table returned by the provider getter. That
  size plus the five exact function fields is the canonical ABI representation; accepting a
  zero-filled or partial tail would be an accidental alternative spelling. Fake old/current,
  partial-size, missing-function, and future-size providers prove this boundary owns the check.
- Rejecting sizes strictly between the Slice 3b and Slice 4 minima survives. No released provider
  owns such a prefix, so this shape is malformed ABI rather than a smaller capability. The fake
  partial-prefix case fails without this check.
- Clamping and retaining the provider-reported table size survives. The getter is the semantic
  source of truth; replacing it with the local `sizeof` makes an old provider appear to own
  unreadable tail fields. Old-capacity sentinel and old-provider tests exercise the producer/
  consumer boundary.
- The typed `_validateHandleResult` host helper survives after being revised to clear a non-null
  handle whenever the provider returns failure. A provider may legally write before detecting its
  error, but no failed call may leak that stale handle to a caller. Fake success-with-null and
  failure-after-write cases prove both directions.
- `_isNVVMAddressSpace` survives as exact validation of the public fixed-width enum. Unknown
  integers are out-of-contract ABI input; the function does not infer or rewrite address spaces.
- `_getLoadablePointerType` survives. A type/value from a builder call is canonical only when its
  LLVM context equals the supplied module's unique context and its pointer has a loadable typed
  pointee. Context identity and LLVM's pointer type are the existing sources of truth; no operand
  or substitution graph is walked. Cross-module and non-pointer cases require the check.
- `_hasValidInsertionBlock` survives. The current module-owned, unterminated block is intentional
  transient builder state, not malformed upstream IR. Rejecting no block, a foreign block, or a
  terminated block before mutation is the provider's construction-boundary responsibility.
- `_isValidAlignment` was reduced to LLVM's existing `isPowerOf2_32` helper plus the ABI's nonzero
  requirement. Invalid alignment is direct call input, and negative tests prove the check.
- Rejecting a store through address space 4 survives because NVVM constant memory is read-only.
  The pointer's canonical address space is the source of truth; the provider neither casts it nor
  repairs the request. The dedicated negative test fails without the rule.
- The strict-C prefix probes survive as compile-time ABI layout checks for both published minima.
- The PTX entry parser and reference-kernel builder survive as test-only helpers. They normalize
  only primitive scalar parameter widths and entry-scoped global load/store tokens, and they
  orchestrate public private-ABI calls without synthesizing or patching serialized IR. Unsupported
  PTX output fails the test rather than falling back to a product interpretation.

No added helper rebuilds syntax or structural shape from a semantic value. No AST, Slang IR,
`Val`, witness, substitution, or lookup representation is involved in this slice; raw ABI handles
and explicit builder state are the producer inputs, so their validation belongs at this boundary.

## Validation and Acceptance

The intended focused selectors are:

- `slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarAPI`
- `slang-unit-test-tool/nvvmIRBuilderRejectsInvalidScalarOperations`
- `slang-unit-test-tool/nvvmIRBuilderBuildsScalarReferenceKernels`
- `slang-unit-test-tool/nvvmIRBuilderDifferentialScalarPTX`
- `slang-unit-test-tool/nvvmIRBuilderPtxasAcceptsScalarReferenceKernels`

Exact command names may be consolidated if one fixture owns one expensive compilation, but the
five behaviors must remain separately diagnosable. Run builds and tests outside the sandbox, using
Windows-native tools from the repository root:

```powershell
cmake.exe --build build/nvvm-builder-deps/slang-llvm-nvvm-build `
    --config Release --target slang-llvm-nvvm -- /m
cmake.exe --build build --config Debug --target compiler-core -- /m
cmake.exe --build build --config Debug --target slang-unit-test `
    -- /p:BuildProjectReferences=false /m

$env:SLANG_NVVM_BUILDER_PATH = `
    (Resolve-Path -LiteralPath `
        'build/nvvm-builder-deps/slang-llvm-nvvm-build/Release').Path
build/Debug/bin/slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmIRBuilderNegotiatesScalarAPI
build/Debug/bin/slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmIRBuilderRejectsInvalidScalarOperations
build/Debug/bin/slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmIRBuilderBuildsScalarReferenceKernels
build/Debug/bin/slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmIRBuilderDifferentialScalarPTX
build/Debug/bin/slang-test.exe -skip-api-detection `
    slang-unit-test-tool/nvvmIRBuilderPtxasAcceptsScalarReferenceKernels
build/Debug/bin/slang-test.exe -skip-api-detection slang-unit-test-tool/nvvm
```

Also run the isolated coexistence selector and the established unclosable-library, downstream
version/API, downstream-file, and NVRTC sampler regressions listed in the Slice 3b plan. Run the
fake prefix with `SLANG_NVVM_BUILDER_PATH` absent and prove real scalar tests ignore cleanly; an
explicit broken builder path must fail.

Acceptance requires:

- strict C and C++ standalone module builds against exactly LLVM 14.0.6;
- byte-for-byte V1 and field-for-field Slice 3b V2 prefix compatibility;
- old/new V2 size negotiation in both directions, including sentinels and malformed tails;
- fake scalar negotiation and forwarding tests without LLVM/CUDA;
- deterministic real negative-operation coverage with no partial IR mutation;
- verified assembly showing AS1 typed pointers, `i32`, align-4 load/store, entry annotations, and no
  verifier diagnostic;
- nonempty PTX from both real routes for explicit `compute_75`;
- matching normalized entry ABI and global-memory instruction families;
- successful configured-toolkit `ptxas -arch=sm_75 -v` acceptance;
- all existing NVVM and named regression tests passing;
- clean provider exports (exactly V1/V2) and no LLVM DLL dependency;
- ordinary `-target ptx` remaining on NVRTC; and
- `git.exe diff --check` clean with no retained generated PTX/cubin.

## Failure and Recovery

All build products and generated evidence remain under ignored `build/` or test temporary
directories. Every build/test command is safe to rerun. Do not copy the provider into the ordinary
Debug bin directory; select it with `SLANG_NVVM_BUILDER_PATH` so absence behavior stays testable.

If the provisional AS1 kernel fails, remove only the unpromoted V2 tail and its tests, retain the
frozen V1/Slice 3b behavior, and prototype AS0 plus `addrspacecast` from recorded evidence. If an
old provider cannot be distinguished, stop and fix reported-size retention rather than probing a
function field beyond the advertised prefix. If LLVM accepts construction but libNVVM rejects the
bitcode, preserve the complete verifier/compiler diagnostic and correct the producer shape; do not
rewrite serialized bytes or suppress verification. If PTX normalization is unstable, reduce it to
entry-scoped semantic tokens backed by both observed outputs rather than special-casing a tool
version.

Do not reset, delete, or modify the unrelated `external/slang-binaries/` directory. Removing the
new V2 tail, wrappers, provider functions, and Slice 4 tests restores Slice 3b without disturbing
NVRTC or downstream discovery.

## Artifacts and Hand-Off

Retain LLVM source/build trees, the standalone module, host binaries, and any temporary PTX/cubin
only under ignored `build/`. Record exact tool versions, test counts, ignored tests, output sizes,
and `ptxas -v` observations in Progress and Outcomes; do not check generated artifacts in.

Before completion, update `docs/design/nvvm-backend.md` with the frozen V2 scalar prefix, AS1
reference-kernel decision, normalized comparison contract, and measured results. Distill the
motivation, principled producer-side implementation, file summary, vocabulary, full code trace,
rejected alternatives, and input-shape audit into the eventual five-part PR description. The next
slice must be able to start from that durable design plus this hand-off without conversation
history.
