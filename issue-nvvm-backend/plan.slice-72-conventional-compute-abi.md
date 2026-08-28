# Accept the first conventional CUDA compute shader

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct libNVVM route accepts and executes
`tests/cuda/compile-to-cuda.slang`, the first unmodified conventional compute shader selected by
the Slice 71 corpus census. The entry point continues to use ordinary
`SV_DispatchThreadID`/`[numthreads]` source and an ordinary global `RWStructuredBuffer<int>`;
callers do not need `[CUDAKernel]` or raw launch parameters.

The shortest observation is the shader's additional CUDA compute lane forwarded with
`-emit-cuda-via-nvvm`: render-test must populate `SLANG_globalParams`, launch a zero-parameter PTX
kernel, and compare all output values successfully.

## Progress

- [x] (2026-08-28) Reproduced the current `entry-point parameter` rejection and captured the final
  canonical IR plus NVRTC CUDA/PTX reference ABI.
- [x] (2026-08-28) Traced `SV_DispatchThreadID` to the existing CUDA varying-parameter legalizer and
  the resource to the canonical `ConstantBuffer<GlobalParams>` global parameter.
- [x] (2026-08-28) Prototyped and froze the externally visible constant-address-space global plus structural
  provider operations needed for `SLANG_globalParams`.
- [x] (2026-08-28) Routed direct NVVM through existing CUDA varying legalization and accepted the exact canonical
  execution/global-parameter graph.
- [x] (2026-08-28) Added focused fake/real-provider, compile, PTX, negative, and GPU runtime coverage.
- [x] (2026-08-28) Updated durable design/ledger records, ran the complete validation ladder and self-review,
  then commit the plan and implementation together.

## Surprises and Discoveries

- The direct target reaches `legalizeEntryPointVaryingParamsForCUDA`'s dispatch after linking and
  specialization, but the switch calls that pass only for `CUDASource` and `CUDAHeader`. A direct
  PTX target therefore retains the borrowed `uint3 : SV_DispatchThreadID` parameter even though an
  established producer already owns its canonical CUDA lowering.
- CUDA source emission does not pass conventional globals as kernel arguments. It emits
  `extern "C" __constant__ GlobalParams SLANG_globalParams`, and the runtime copies the reflected
  payload into that module symbol before launching a zero-parameter kernel. The NVRTC reference
  PTX contains `.const .align 8 .b8 SLANG_globalParams[16]`.
- The collected global has one `RWStructuredBuffer<int>` field at offset zero for the acceptance
  shader. Its canonical graph is `get_field_addr(globalParams, fieldKey)`, load of the resource
  view, `rwstructuredBufferGetElementPtr`, then ordinary numeric loads/stores.
- The current provider has array/vector construction and an internal global declaration, but no
  general struct type, struct-field address, or externally visible module-global definition.
- LLVM external linkage plus an `undef` initializer in address space 4 is a definition, not an
  unresolved declaration. libNVVM turns that exact form into
  `.visible .const .align 8 .b8 SLANG_globalParams[16]`, which the CUDA driver runtime discovers and
  populates without any new runtime path.
- Render-test's debug layer asks Slang for debug information, but the builder has no debug-metadata
  surface. Preserving those producer instructions made `DebugVar` the first runtime-only stop;
  stripping debug information as an explicit direct-target capability boundary lets the same
  semantic program compile in ordinary and debug-layer lanes.
- The libNVVM downstream compiler intentionally requires an explicit CUDA architecture. The
  existing shader lane therefore declares `cuda_sm_7_0`, matching all direct backend tests rather
  than adding a compiler default that could ignore the target device/toolkit contract.
- After the slice, the next measured corpus stops are `GenericAsm` (`cuda-layout.slang`),
  `getElement` (`wave-lane-index-multidim.slang`), and the exact multi-field conventional global
  boundary (`sampler-comparison-state-unused.slang`).

## Decision Log

- Decision: reuse `legalizeEntryPointVaryingParamsForCUDA` for the direct route.
  Rationale: the borrowed semantic parameter is valid pre-legalization IR, and the existing CUDA
  producer already defines the canonical `blockIdx * blockDim + threadIdx` representation. Adding
  an emitter-side interpretation of `SV_DispatchThreadID` would duplicate that policy.
  Date/author: 2026-08-28, Codex.
- Decision: extend the current forward-only builder with structural type/field operations and an
  explicit global linkage rather than a callback named for one shader or parameter block.
  Rationale: struct formation, field addressing, and symbol visibility are LLVM construction
  concepts; keeping them generic avoids another combinatorial resource/type interface. The host
  still owns recognition of Slang's exact supported canonical shapes.
  Date/author: 2026-08-28, Codex.
- Decision: initially accept only conventional parameter-block structs whose fields recursively
  lower through the existing value-type set, beginning with `RWStructuredBuffer<int>`.
  Rationale: this slice proves the ABI and existing-suite bridge without claiming arbitrary CUDA
  layout compatibility. Unsupported field types remain precise preflight failures before provider
  mutation.
  Date/author: 2026-08-28, Codex.
- Decision: replace the signed-i32x2-only binary-add family with the bounded dimensioned integer
  binary family already used for scalar widths, admitting one through four lanes.
  Rationale: CUDA varying legalization produces UInt3 multiply and add. A whole-signature UInt3
  special case would recreate the combinatorial interface that the current typed descriptor was
  designed to avoid; exact Slang type preflight still limits source admission independently.
  Date/author: 2026-08-28, Codex.
- Decision: keep direct NVVM IR debug-free until the builder has an explicit debug metadata
  contract.
  Rationale: debug producer instructions are neither executable semantics nor valid input to a
  provider that cannot encode their metadata. This is a target-capability decision applied before
  emission, not an emitter fallback for one test.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The first ordinary test-suite shader now compiles and executes through direct libNVVM without
source ABI annotations. The accepted subset is deliberately exact: one synthesized global
parameter struct with one raw `RWStructuredBuffer<int>` field at offset zero, size 16, alignment
8; the existing CUDA varying legalizer's UInt3 dispatch-thread graph; and a zero-parameter kernel.
The provider exposes generic struct type/field operations and global linkage through exact builder
ABI revision 2, while Slang owns the canonical-shape boundary.

Normal and NVVM-2.0-compatible builder assembly contain the external address-space-4 global,
field GEP, resource-view load, raw element address, store, and kernel annotation. Direct PTX
contains `.visible .const .align 8 .b8 SLANG_globalParams[16]`, a visible zero-argument
`computeMain`, and `%tid.x`, `%ntid.x`, and `%ctaid.x`. The existing runtime comparison passes both
the NVRTC and direct libNVVM CUDA lanes. A two-resource block is rejected before provider mutation.

The next slice should select one of the newly measured corpus boundaries. The closest extension of
this ABI is the multi-field conventional block in
`sampler-comparison-state-unused.slang`; `getElement` and `GenericAsm` are independent execution or
language-operation bundles and should remain separate unless the next corpus census shows a more
valuable composition.

Release validation passed 334/334 tests under `slang-unit-test-tool/nvvm`. The focused shader run
passed both CUDA comparison lanes plus the new diagnostic boundary (3/3 active tests), and focused
fake/real-provider coverage passed. Both the main compiler/test build and standalone LLVM 14
provider build completed. The repository formatting script could not run because `gersemi`,
`clang-format`, `prettier`, and `shfmt` are absent from its WSL path; all changed C++ files were
formatted with the repository-pinned clang-format 17 binary instead, and `git diff --check` passed.

## Context and Current Pipeline

Consider this existing shader:

```slang
RWStructuredBuffer<int> outputBuffer;

[shader("compute")]
[numthreads(4, 1, 1)]
void computeMain(uint3 dispatchThreadID : SV_DispatchThreadID)
{
    int tid = int(dispatchThreadID.x);
    outputBuffer[tid] = tid;
}
```

Linking and `collectGlobalUniformParameters` synthesize a module-level
`ConstantBuffer<GlobalParams>` global. `GlobalParams` contains the resource view, whose CUDA host
layout is a device pointer followed by a 64-bit count. The entry's semantic parameter remains a
borrowed `uint3` today because `slang-emit.cpp` does not invoke CUDA varying legalization for the
direct PTX target. `_validateNVVMFunction` in `slang-emit-nvvm.cpp` therefore rejects it before
provider discovery.

For CUDA source, `CUDAEntryPointVaryingParamLegalizeContext` in
`slang-ir-legalize-varying-params.cpp` replaces the parameter with values derived from target
intrinsic globals named `threadIdx`, `blockIdx`, and `blockDim`. `CUDASourceEmitter::emitParameterGroupImpl`
then exposes the collected struct as `SLANG_globalParams`. Render-test uses reflection to build the
payload, `cuModuleGetGlobal` to find the symbol, and `cuMemcpyHtoD` before launch. Slice 72 preserves
that producer-to-runtime ABI while changing only the final IR consumer.

## Scope and Non-Goals

In scope are conventional compute system semantics already handled by the CUDA legalizer; a
structural provider contract; the exact collected constant-buffer/global-param form; conventional
`RWStructuredBuffer<int>` resource loads and stores; an externally visible
`SLANG_globalParams`; and one existing shader's compile/runtime acceptance.

Out of scope are arbitrary struct/resource fields, textures/samplers, explicit constant-buffer
objects, entry-point uniform parameters, multiple parameter blocks, OptiX stages, broad CUDA-suite
enablement, or backward compatibility for this experimental builder ABI.

## Architecture and Invariants

The entry-point legalization pass owns semantic varying inputs. After it runs, the NVVM emitter
sees a zero-parameter conventional compute kernel and canonical CUDA execution values. Raw
`[CUDAKernel]` uniform launch parameters remain untouched because the varying legalizer processes
only parameters whose layout classifies them as varying.

The Slang emitter owns mapping canonical Slang structs, field keys, resource types, address spaces,
and global linkage to structural builder calls. The provider owns LLVM type identity, GEP
construction, global linkage, LLVM verification, and NVVM IR 2.0 text compatibility. The provider
API does not mention `GlobalParams`, field names, or Slang IR opcodes.

`SLANG_globalParams` is one external-linkage definition in constant address space 4, with an
undefined initializer that reserves storage and 8-byte alignment for the first accepted layout.
It is not a launch argument. Shared-memory globals remain internal linkage in address space 3.

Preflight remains authoritative: every accepted global, field, address operation, load, and target
intrinsic is validated before provider discovery. Emission does not contain a fallback for unknown
field keys or types.

## Interfaces and Dependencies

Change `SlangNVVMBuilderConstructionAPI`, `NVVMIRBuilder`, the real LLVM 14 provider, and the fake
provider in lockstep. Add a generic struct-type constructor and struct-field-pointer operation.
Extend global declaration with an exact linkage enum supporting internal and external definitions.
Because the interface is forward-only and all callbacks are required, the ABI revision remains an
exact source/provider match rather than compatibility scaffolding.

The external prototype must serialize valid LLVM 14 assembly and valid LLVM 7-era NVVM IR 2.0
assembly, compile through libNVVM, and produce a visible `.const` PTX symbol discoverable by the
existing CUDA driver runtime.

## Milestones

1. Add provider construction tests for a `{ RWStructuredBuffer<int> }` struct, field GEP, constant
   address-space external definition, load, and kernel use. Reject foreign types/values, invalid
   field indices/linkage/alignment, and duplicate symbols.
2. Invoke CUDA varying legalization for direct emission and add fake-emitter tests showing the
   conventional entry has no launch parameters and requests thread/block execution semantics.
3. Extend NVVM type lowering and preflight for the exact collected parameter-group struct, target
   intrinsic globals, `get_field_addr`, and resource-view loads. Emit the module global and graph
   only through the new structural builder contract.
4. Add a direct lane to `tests/cuda/compile-to-cuda.slang`, prove stable PTX evidence, and execute
   the existing comparison test on the GPU. Add precise negative coverage for the nearest excluded
   field/global shape.
5. Update `docs/design/nvvm-backend.md` and the capability ledger, format changed sources, run
   focused and full tests, audit new helpers/special cases, and commit the completed slice.

## Validation and Acceptance

Run every CMake build and test outside the sandbox. Acceptance requires:

- the real builder's structural/global tests to serialize both normal LLVM assembly and compatible
  NVVM IR 2.0 assembly, with LLVM verification valid;
- direct `slangc` compilation of `tests/cuda/compile-to-cuda.slang` to PTX containing a visible
  zero-parameter kernel, CUDA execution-register reads, and `SLANG_globalParams` constant storage;
- the added `COMPARE_COMPUTE` direct lane to pass on the CUDA GPU;
- raw `[CUDAKernel]`, shared-memory, and current source/NVRTC routes to remain green;
- the complete Release `slang-unit-test-tool/nvvm` prefix to pass;
- formatting and `git diff --check` to pass; and
- no generated PTX, build output, or `external/slang-binaries/` content to be staged.

## Failure and Recovery

Prototype the external constant global first. If LLVM verification succeeds but libNVVM emits an
external declaration rather than allocated `.const` storage, compare external-linkage definitions
with `externally_initialized` and retain only the form whose PTX is host-populatable. Remove failed
prototype-only tests/artifacts before promotion.

If CUDA varying legalization changes raw kernel behavior, isolate the pass to conventional entry
points based on the existing raw `IRCudaKernelDecoration` contract rather than teaching emission
two meanings for one parameter. If a later unsupported op appears in the acceptance shader, record
the exact canonical producer/consumer trace; widen this slice only when it is part of the same
global/resource ABI bundle.

All edits are additive or exact interface replacements in the experimental path. Rebuilding the
provider and compiler restores a consistent pair after any partial API change.

## Artifacts and Hand-Off

Retain locally the dumped final Slang IR, reference CUDA/PTX, direct NVVM IR/PTX, and focused test
logs under ignored `build/` paths. Distill the supported conventional ABI, exact field subset,
symbol form, and the next existing-suite diagnostic into the durable design and capability ledger.
Complete the input-shape audit and validation evidence in this plan, then commit it with Slice 72.

## Self-Review

The new-helper/special-case inventory is:

- `_getNVVMConventionalGlobalParams` survives as the one exact first-rung shape recognizer. Its
  input is produced by `collectGlobalUniformParameters`; it checks the synthesized decoration,
  exact field type/key/layout, and is required by the conventional fake/runtime tests. It does not
  rebuild source syntax or search alternate graphs.
- `_getNVVMCUDAExecutionGlobalOperation` and its descriptor helper survive as the mapping from the
  exact target-intrinsic globals produced by `CUDAEntryPointVaryingParamLegalizeContext` to the
  established semantic execution operations. Removing the CUDA legalizer route reproduces the
  original entry-parameter failure; interpreting `SV_DispatchThreadID` in emission was rejected.
- `_lowerStructType` survives as generic recursive structural lowering, reachable only after the
  exact whole-module preflight admits the owning struct. The provider owns literal LLVM struct
  identity; Slang owns field semantics and layout admission.
- `fixUpFuncType` is a producer-side invariant repair after any varying legalizer replaces entry
  parameters. Removing it reproduces the parameter-count mismatch because the entry block and
  function type disagree.
- Direct debug stripping survives as a declared target capability boundary. The revert drill made
  the runtime debug-layer lane stop at canonical `DebugVar`; no executable IR shape was malformed,
  and the provider currently has no debug-metadata contract to preserve it.
- The raw resource-element consumer check was widened from store-only to load-or-store, exactly as
  required by `compile-to-cuda.slang`. The full-prefix drill exposed accidental atomic admission;
  the final check rejects consumers other than ordinary loads/stores, and the adjacent-shape matrix
  passes before provider discovery.

There is no fallback, structural equivalence relation, silent default, syntax reconstruction, or
arbitrary operand-graph walk. Emission asserts the shape already proven by preflight and forwards
the canonical field index and semantic values to the provider.
