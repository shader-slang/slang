# Generalize resource views and admit multidimensional wave execution

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, the direct libNVVM route accepts and executes the existing
`tests/cuda/wave-lane-index-multidim.slang` comparison test. The shader combines
`SV_GroupIndex`, a two-dimensional thread group, wave operations, control flow, and an ordinary
global `RWStructuredBuffer<float>`.

The builder no longer exposes callbacks named for `RWStructuredBuffer<int>`. Resource views are
assembled from generic struct and pointer operations, so one structural interface supports the
established scalar element types. The shortest observation is the shader's added direct CUDA lane
passing alongside its existing NVRTC lane.

## Progress

- [x] (2026-08-28) Reproduced the direct route's first stop and captured the final linked IR plus
  reference PTX for the existing shader.
- [x] (2026-08-28) Traced `SV_GroupIndex` to constant-index `getElement` operations on the CUDA
  legalizer's `threadIdx` and `blockDim` `uint3` values.
- [x] (2026-08-28) Replaced the element-specific builder resource callbacks with generic
  struct-value extraction and existing pointer-offset operations; advanced the exact forward-only
  ABI to revision 3.
- [x] (2026-08-28) Generalized canonical resource-view type lowering and validation to the
  selected scalar element set, and emitted vector extraction for both canonical IR spellings.
- [x] (2026-08-28) Added real/fake provider, negative-boundary, PTX, `ptxas`, and existing-shader
  runtime coverage.
- [x] (2026-08-28) Updated durable design/ledger records, ran the validation ladder and
  self-review, and prepared the plan and implementation for one commit.

## Surprises and Discoveries

- The final linked representation of `SV_GroupIndex` is not `swizzle`, the spelling used by the
  already-supported `SV_DispatchThreadID` shader. It is four constant-index `getElement`
  operations over the same canonical target-intrinsic `uint3` globals. Both are valid producers of
  one semantic vector-component operation.
- CUDA's raw `RWStructuredBuffer<float>` launch/global representation has the same 16-byte,
  8-aligned `{ float addrspace(1)*, i64 }` shape as the established integer view. Only the pointee
  type changes.
- The existing provider callback combines two independent structural operations: extracting field
  zero from a resource-view value and offsetting its pointer. Keeping that combined callback would
  require one provider entry for every element type despite LLVM already representing both steps
  generically.
- The resource index in ordinary Slang source is canonical UInt32, while the original raw-resource
  proof used Int32. Resource addressing therefore needs the already-established sign-independent
  32-bit transport contract; it does not need a cast or a second provider operation.
- The fake provider had implemented selected integer/float conversion capabilities but had never
  dispatched those typed operation families. The existing shader's lane-index-to-float conversion
  exposed that test-double gap; adding the missing dispatcher records brought the fake provider
  back into agreement with the real provider's declared typed-operation contract.
- The final shader graph contains five execution-vector extracts in the stable index sequence
  `2, 1, 1, 0, 0`, followed by two resource pointer extractions/offsets and float stores. This gives
  the fake route a precise structural assertion without matching incidental LLVM text.
- An initial validation run exposed an accidental edit at two neighboring pointer cases: ordinary
  pointer offsets became signed-only while fixed-array indices became sign-independent. Comparing
  the calls with Slice 72 restored each established contract and kept only resource indices newly
  sign-independent. The four representative failures then passed before the full rerun.

## Decision Log

- Decision: replace, rather than extend, the two `i32`-specific resource callbacks.
  Rationale: this backend is forward-only, and retaining compatibility would preserve the exact
  combinatorial interface the slice is removing. A resource view is constructed with
  `getStructType({ pointer(element, global), i64 })`; element addressing extracts field zero and
  invokes the existing generic pointer-offset operation.
  Date/author: 2026-08-28, Codex.
- Decision: add one generic `emitStructFieldValue` provider operation.
  Rationale: LLVM owns aggregate value identity and `extractvalue` validation. Slang owns the
  meaning of field zero in its accepted resource ABI. This keeps LLVM handles shielded without
  naming a source-language resource in the builder interface.
  Date/author: 2026-08-28, Codex.
- Decision: accept the established selected scalar values as resource elements while keeping the
  conventional global block at exactly one resource field.
  Rationale: pointer formation, size, alignment, ordinary load/store, and the resource view are
  already generic for selected integer scalars and float32. Vector/aggregate elements and
  multi-field parameter blocks have additional layout/operation contracts and remain separate
  measured boundaries.
  Date/author: 2026-08-28, Codex.
- Decision: map both one-component `swizzle` and constant-index `getElement` on canonical CUDA
  execution vectors to `emitVectorElementExtract`.
  Rationale: both shapes are intentionally produced by existing CUDA legalization. Normalizing
  them in an unrelated producer would disturb other targets; recognizing their exact common
  semantic contract in the NVVM consumer avoids inventing an emitter-only IR rewrite.
  Date/author: 2026-08-28, Codex.

## Outcomes and Retrospective

The current builder ABI now has one generic aggregate-value extraction operation and no callback
named for a resource or element type. Both the real and fake providers build selected scalar
resource views from ordinary pointer, integer, struct, extraction, and pointer-offset operations.
The established raw integer resource remains green while an ordinary float resource exercises the
same structural path.

The existing multidimensional wave shader passes all five registered CUDA/Vulkan test lanes. A
direct `slangc` compile emits a visible zero-parameter kernel and the expected 16-byte constant
global; its PTX uses multidimensional thread/block-dimension registers, synchronized wave voting,
and two float global stores. CUDA 12.9 `ptxas` accepts the direct PTX for `sm_70`.

The Release builder/provider builds, focused real/fake tests, four restored regression tests, and
the complete 335-test NVVM prefix pass. Post-slice probes still stop at `GenericAsm` in
`cuda-layout.slang` and at the multi-field conventional-global address in
`sampler-comparison-state-unused.slang`; neither boundary was hidden by a permissive fallback.

## Self-Review and Input-Shape Audit

The new helper and special-case inventory is intentionally small:

- `_isNVVMSupportedResourceElementType` survives as the single selected-scalar policy gate. Its
  exact input is the canonical element type produced by resource type lowering; no equivalent type
  graph or source syntax is reconstructed.
- `_lowerRawRWStructuredBufferType` survives as the construction boundary for the canonical CUDA
  view `{ global pointer(element), UInt64 }`. It reuses the type lowerer's existing pointer,
  integer, and struct caches rather than adding another resource representation.
- `_getNVVMCUDAExecutionVectorElement` survives because CUDA varying legalization intentionally
  produces both one-component `swizzle` and constant-index `getElement` for the same target
  execution `uint3` values. It accepts only those two exact shapes, canonical bases, and indices
  zero through two; it does not walk arbitrary operand graphs or invent structural equivalence.
- The fake provider's internal resource-view handles survive only as test-double representations
  for structurally validated pointer/count structs. Separate integer and float handles preserve
  the element kind through raw parameters and conventional field loads without consulting pointer
  creation order. Production Slang and LLVM remain the semantic sources of truth, and tests assert
  the generic operation sequence rather than source-resource names.
- Sign-independent resource indices survive because ordinary source indexing canonically produces
  UInt32 while raw entry parameters may produce Int32. Both are valid 32-bit GEP indices and use
  the existing validator; there is no new coercion or fallback.

Removing any of these changes respectively restores the unsupported float-resource type, prevents
structural provider construction, rejects the existing `SV_GroupIndex` producer, makes the fake
provider unable to observe resource operations, or rejects canonical UInt32 resource indices. No
consumer repairs malformed IR, rebuilds syntax from semantic data, or returns a default for an
impossible shape.

## Context and Current Pipeline

Consider this existing shader:

```slang
uniform RWStructuredBuffer<float> outputBuffer;

[numthreads(8, 8, 1)]
void computeMain(uint lane : SV_GroupIndex)
{
    uint i = lane * 2;
    outputBuffer[i] = WaveIsFirstLane() ? 1.0 : 0.0;
    outputBuffer[i + 1] = float(WaveGetLaneIndex());
}
```

`legalizeEntryPointVaryingParamsForCUDA` replaces `SV_GroupIndex` with the established linear
thread-index expression. In final linked IR, that expression reads constant components from the
target-intrinsic `threadIdx` and `blockDim` `uint3` globals using `getElement`, then performs UInt32
multiply/add operations. `collectGlobalUniformParameters` places `outputBuffer` in the synthesized
one-field `GlobalParams` constant buffer. The entry loads its resource-view value, applies
`rwstructuredBufferGetElementPtr`, and performs ordinary float stores. Existing wave ballot,
lane-index helper, control-flow, phi, integer-to-float, and global-parameter machinery owns the
rest of the graph.

Preflight currently stops at `getElement`. If that spelling is admitted without the resource
generalization, the next stop is the hard-coded `RWStructuredBuffer<int>` recognizer/provider
callback. The reference NVRTC PTX exposes `.const .align 8 .b8 SLANG_globalParams[16]` and a
zero-parameter `computeMain` kernel.

## Scope and Non-Goals

In scope are exact constant-index extraction from canonical CUDA execution `uint3` values;
resource views for selected scalar integer types and float32; generic aggregate value extraction;
the existing shader's compile/PTX/GPU execution; and regression coverage for the established
integer view.

Out of scope are dynamic vector indexing, arbitrary vectors/aggregates, structured user-defined
resource elements, textures/samplers, multiple global fields, explicit constant buffers, resource
atomics, backward ABI compatibility, and broad CUDA-suite enablement.

## Architecture and Invariants

The CUDA varying legalizer owns the linearization of `SV_GroupIndex`; the emitter accepts only its
canonical constant-component graph. Slang preflight proves the base is an accepted `uint3`, the
index is an in-range executable integer constant, and every use is available/dominating before the
provider is discovered.

Slang owns the raw CUDA resource ABI: a literal unpacked struct whose first field is a global
address-space pointer to the canonical selected scalar and whose second field is an unsigned
64-bit count. The provider owns literal LLVM struct identity, `extractvalue`, pointer offset/GEP,
LLVM verification, and compatible text serialization. Resource addressing is emitted as
`extractvalue buffer, 0` followed by `getelementptr element, dataPointer, index`.

The accepted conventional global remains one synthesized field at offset zero, total size 16,
alignment 8. Loads of resource views use alignment 8; scalar element loads/stores use their
natural alignment. Unsupported element, consumer, or block shapes fail preflight before provider
creation or mutation.

## Interfaces and Dependencies

Advance `SLANG_NVVM_BUILDER_ABI_REVISION` and change
`SlangNVVMBuilderConstructionAPI`, `NVVMIRBuilder`, the LLVM 14 provider, and the fake provider in
lockstep. Remove `getRawRWStructuredBufferI32Type` and
`emitRawRWStructuredBufferI32ElementPointer`; add `emitStructFieldValue`. Continue to use the
existing `getStructType`, `getPointerType`, `getIntegerType`, and `emitPointerOffset` interfaces.

No new LLVM/libNVVM dependency is introduced. Both ordinary LLVM 14 assembly and rewritten NVVM
IR 2.0-compatible text must assemble and compile through the existing provider pipeline.

## Milestones

1. Replace the builder ABI surface and add real-provider construction/rejection tests for generic
   struct-value extraction, including foreign values, invalid field indices, and incompatible
   aggregate types.
2. Rework fake-provider structural types/values enough to observe the generic sequence and update
   established integer resource tests to assert struct construction, field extraction, and pointer
   offset instead of resource-named callbacks.
3. Generalize canonical Slang resource type helpers/lowering and conventional global recognition.
   Add exact `getElement` validation/emission alongside `swizzle`; keep dynamic or non-CUDA vector
   access rejected.
4. Add a direct lane to `tests/cuda/wave-lane-index-multidim.slang`, negative coverage for the
   closest unsupported resource element, and stable PTX/`ptxas`/GPU runtime evidence.
5. Update `docs/design/nvvm-backend.md` and the capability ledger, format changed sources, run the
   focused and full NVVM suites, perform the input-shape audit, and commit the completed slice.

## Validation and Acceptance

Run every CMake build and test outside the sandbox. Acceptance requires:

- builder ABI negotiation reports the new exact revision and both real/fake providers expose no
  resource-element-specific callbacks;
- real provider tests serialize a generic resource view and `extractvalue`/GEP sequence in normal
  and NVVM-compatible assembly, and reject invalid aggregate extraction without mutation;
- fake emitter tests prove integer and float resource graphs use structural calls and exact type
  relations; an excluded resource element fails before provider discovery;
- direct `slangc` compilation of `wave-lane-index-multidim.slang` produces a visible zero-argument
  kernel, `SLANG_globalParams[16]`, multidimensional CUDA execution-register reads, wave operations,
  and float global stores accepted by `ptxas`;
- the existing shader's NVRTC and direct runtime comparison lanes pass on the CUDA GPU;
- the full Release `slang-unit-test-tool/nvvm` prefix passes;
- formatting and `git diff --check` pass; and
- no generated output or `external/slang-binaries/` content is staged.

## Failure and Recovery

Make the provider API replacement atomically across the host, real provider, and fake provider;
partial builds are expected to fail until all four agree, and a rebuild safely recovers. If LLVM
rejects literal-struct `extractvalue`, isolate it in the real-provider unit test and inspect the
owning module/type checks rather than restoring an element-specific callback.

If the existing shader exposes an operation outside the measured graph after `getElement` and
float resources are admitted, record its exact producer and decide whether it belongs to this
semantic bundle. Do not add a permissive default. All experimental changes can be removed without
affecting the established CUDA source/NVRTC route.

## Artifacts and Hand-Off

Retain compiler IR/PTX and focused test logs under ignored `build/` paths when useful. Distill the
generic raw-resource representation, the exact vector-extraction boundary, validation evidence,
and the next corpus diagnostic into durable design/ledger documents. Complete this plan's living
sections and self-review, then commit it with Slice 73.
