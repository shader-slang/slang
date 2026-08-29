# Add copyable structured-buffer aggregates

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM can materialize a nonempty copyable struct whose direct fields are
selected numeric scalar or vector values, update its fields through one local address, load the
complete aggregate, and store it to an exact read-write structured-buffer element. The existing
`tests/compute/half-structured-buffer.slang` fixture should pass direct CUDA runtime and PTX lanes
without another LLVM-builder ABI revision.

## Progress

- [x] (2026-08-29) Probed the first post-Slice-96 Half fixture and captured the final canonical IR.
- [x] (2026-08-29) Identified the exact `Thing` layout and operation chain: local aggregate, three
  field stores, whole-value load, and one `RWStructuredBuffer<Thing>` element store.
- [x] (2026-08-29) Defined one reusable copyable-struct classifier and physical alignment policy
  without broadening scalar-only entry or constant-buffer ABIs.
- [x] (2026-08-29) Generalized local aggregate, field-address, raw structured-buffer, load/store,
  and retained type validation for the exact copyable family.
- [x] (2026-08-29) Added focused fake-boundary coverage; the registered real-provider shader and
  `ptxas` run prove the new physical behavior without a redundant builder callback test.
- [x] (2026-08-29) Registered direct runtime/PTX lanes for the existing shader and validated
  output, PTX, and `ptxas` acceptance.
- [x] (2026-08-29) Formatted, built, ran focused/full/changed-shader validation, completed
  self-review, updated durable docs, and prepared the completed slice commit.

## Surprises and Discoveries

- The final linked IR is already direct and canonical. It contains
  `var Ptr<Thing>`, field addresses for `uint`, `float`, and `half4`, a whole `load Thing`, and a
  whole store to the pointer produced by `rwstructuredBufferGetElementPtr`.
- `Thing` has CUDA offsets `0`, `4`, and `8`, size 16, and carries one Half4 field. The established
  scalar-struct classifier rejects it even though every field already has a first-class provider
  representation.
- Slice 96's builder operation is already generic enough for this local. LLVM struct types,
  aggregate loads/stores, typed pointer offsets, and Half4 values also already exist, so this is a
  compiler policy/composition slice rather than a shield-API slice.
- CUDA and LLVM can disagree on vector-containing struct layout even when every direct field has a
  provider value type. `Thing` has identical offsets and stride under both rule sets, while a
  leading `half` followed by `half4` does not; eligibility therefore needs exact layout preflight,
  not only a field-family check.
- The fake provider's raw resource view was already element-kind driven. Extending that one generic
  descriptor with its existing aggregate kind was sufficient; no aggregate-specific fake API or
  production callback was needed.

## Decision Log

- Decision: introduce a selected copyable-struct family whose direct fields are established
  numeric scalar or vector values.
  Rationale: this exactly describes the measured `Thing` representation and composes existing
  value types. Boolean fields, nested structs, arrays, matrices, resources, and opaque types retain
  independent layout questions and remain rejected.
  Date/author: 2026-08-29, Codex.
- Decision: keep scalar-only entry-by-value and parameter-group classifiers separate.
  Rationale: accepting a local/resource representation does not prove CUDA launch ABI or constant
  buffer layout for Half/vector-containing structs. The broader classifier is used only in the
  roles demonstrated by this slice.
  Date/author: 2026-08-29, Codex.
- Decision: reuse builder ABI revision 11.
  Rationale: the provider API already accepts complete type handles for local storage, pointer
  offsets, loads, stores, and struct field addresses. Adding an aggregate-specific callback would
  duplicate semantic policy inside the LLVM shield.
  Date/author: 2026-08-29, Codex.
- Decision: require identical CUDA/LLVM field offsets and total size, while allowing preferred
  aggregate alignment to differ.
  Rationale: offsets and stride are the externally observable buffer contract. `Thing` satisfies
  them exactly; requiring equal preferred alignment would reject this valid representation, while
  ignoring offsets would silently corrupt adjacent field shapes. Incompatible layouts need a
  future explicit padding/legalization design.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

Consider the existing shader:

    struct Thing
    {
        uint pos;
        float radius;
        half4 color;
    };

    Thing thing;
    thing.pos = tid;
    thing.color = half4(...);
    thing.radius = v;
    outputBuffer[tid] = thing;

The final IR producer has already legalized the output resource to the raw CUDA view used by
earlier structured-buffer slices. The entry body creates one `Ptr<Thing>` local, addresses each
field by semantic key, then loads `%Thing` and stores it to the exact pointer returned for
`RWStructuredBuffer<Thing>[tid]`. No constructor, helper ABI, nested aggregate, dynamic field, or
alternative spelling is present.

## Scope and Non-Goals

In scope:

- nonempty first-level structs whose fields are selected integer/Float16/Float32 scalars or
  two-through-four-lane selected numeric vectors;
- exact generic local pointers to those structs;
- field addresses and stores for their selected numeric fields;
- first-class whole-aggregate loads/stores;
- exact read-write structured-buffer views, data pointers, element pointers, and stores for those
  structs;
- the existing Half structured-buffer shader as runtime/PTX evidence.

Out of scope:

- Boolean storage, nested structs, fixed or unsized arrays, matrices, resources inside structs,
  opaque fields, padding synthesis, explicit pack pragmas, or dynamically indexed fields;
- read-only aggregate structured-buffer loads unless they occur in the measured fixture;
- mixed aggregate helper signatures, block phis, entry parameters, parameter/constant buffers,
  pointer returns, or pointer arithmetic outside established raw-buffer element offsets;
- a builder ABI revision or an aggregate operation enum.

## Architecture and Invariants

- The copyable classifier is the single source of truth for local/resource aggregate eligibility.
  It recursively accepts no aggregate field; every direct field must already be an established
  numeric value type.
- Entry-by-value structs and scalar parameter groups continue to use the narrower scalar-only
  classifier. Role-specific lowering cannot be widened by a cached provider handle.
- Struct field identity is resolved by semantic key, never source index. The provider receives the
  verified physical field index only after exact source field/result type validation.
- Whole-aggregate storage uses the same provider struct type for the local and the structured
  buffer element. Pointer offset therefore uses LLVM's verified type size rather than a host-side
  stride constant.
- Natural load/store alignment is derived from the selected physical value family. Unsupported
  types return zero and are rejected before emission.
- The new family remains a first-class value only where availability proves an existing producer;
  no aggregate constructor, phi, helper return, or syntax reconstruction is introduced.

## Interfaces and Dependencies

Add the copyable struct and local pointer classifiers plus vector-aware alignment in
`slang-emit-nvvm-type-lowering.*`. Reuse them in raw-buffer type/data-pointer/element-pointer
classification and in the direct emitter's local, field, load/store, availability, retained-type,
and emission paths. Keep scalar-only classifiers in launch and parameter-group roles.

Extend the fake provider's generic aggregate traces and add a compiler-boundary unit that asserts
the complete local-field-buffer chain. Add direct directives to the existing shader. Use a real
provider unit only if the established aggregate load/store test does not already cover the exact
heterogeneous struct and pointer-offset combination.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. CMake builds and tests run outside the sandbox per repository instructions.

## Milestones

1. Add exact copyable-struct/pointer/alignment classification and focused classifier coverage.
2. Admit the canonical `Thing` local, field stores, whole load, and read-write structured-buffer
   element store through preflight and emission.
3. Prove the compiler boundary with fake traces and preserve deterministic negative coverage for
   nested/Boolean/unrelated aggregate shapes.
4. Register and run the existing shader's direct runtime and PTX lanes, compile standalone PTX,
   and assemble it with CUDA 12.9.
5. Run full regression validation, perform the helper/special-case input-shape audit, update this
   plan and the durable design ledger, and commit.

## Validation and Acceptance

Acceptance requires focused fake and any necessary real-provider unit coverage; the complete
`slang-unit-test-tool/nvvm` prefix; every enabled `half-structured-buffer.slang` lane including new
direct runtime/PTX paths; standalone direct PTX; CUDA 12.9 PTX assembly; pinned clang-format 17;
and `git diff --check`.

Completed evidence:

- The standalone LLVM provider and Release host `slang-unit-test`, `slangc`, and `slang-test`
  targets build successfully.
- `nvvmSlangCopyableStructLocalStoresToStructuredBuffer` passes and records one 8-byte-aligned
  local, three typed field pointers, one whole-aggregate load, one raw-buffer pointer offset, and
  four stores with alignments `4, 4, 8, 8`.
- `nvvmSlangRejectsAdjacentStructuredBufferShapesBeforeProviderMutation` rejects the incompatible
  `half`/Half4 layout before builder discovery. The general unsupported-IR, unsupported `double2`
  resource, and Slice 96 stateful-helper tests continue to pass.
- The complete `slang-unit-test-tool/nvvm` prefix passes 370/370 with the standalone provider.
- All four lanes in `tests/compute/half-structured-buffer.slang` pass: Vulkan, CUDA/NVRTC, direct
  CUDA runtime comparison, and direct PTX FileCheck.
- Standalone optimized direct output is 1,159 bytes of PTX. CUDA 12.9.86
  `ptxas -arch=sm_70` accepts it and emits a 2,920-byte cubin.
- The next existing Half fixture stops at the exact `GenericAsm("__float2half")` and
  `GenericAsm("__half2float($0)")` helper bodies in `half-opaque-convert.slang`.

## Self-Review and Input-Shape Audit

Inventory every new classifier, alignment case, and emitter branch. The accepted input is the
canonical final representation produced by ordinary local-variable and raw structured-buffer
legalization; it is not an accidental spelling to patch downstream. Verify that scalar-only
entry/constant-buffer roles remain unchanged, that no custom type equivalence or graph walk is
added, and that unrelated aggregate locals fail before provider mutation. The revert drill is the
existing shader: removing copyable local admission must restore the measured `var` diagnostic;
removing aggregate raw-buffer admission must expose the subsequent exact resource boundary.

The completed inventory is:

- `asNVVMSupportedCopyableStructType` survives as the single field-family classifier. It accepts
  only direct selected numeric fields and does not recurse into aggregates.
- `asNVVMSupportedLocalCopyableStructPointerType` survives as an exact one-operand generic
  `Ptr<copyable-struct>` classifier. It does not admit borrowed mixed structs, pointer results,
  pointer phis, or other address spaces.
- `_hasNVVMCompatibleCopyableStructLayout` survives as an ABI proof, not a compensating fallback.
  It compares the canonical producer type under CUDA and LLVM layout rules and rejects mismatched
  offsets or stride before provider mutation; it neither pads nor rewrites the type.
- Vector-aware `getNVVMNumericValueAlignment` survives because the provider load/store contract
  needs the physical alignment already implied by each established numeric value descriptor.
- `selectedReachableStructTypes` survives because linked canonical type definitions referenced by
  accepted locals/resources are semantic dependencies. Unrelated globals remain rejected.
- The fake resource element extension survives as the existing generic element-kind descriptor's
  aggregate row. It introduces no production API and is exercised by the focused transport test.

No new custom equivalence, syntax reconstruction, operand-graph walk, textual patch, or silent
default remains. Scalar-only launch and parameter-group roles still use their prior classifier.

## Failure and Recovery

If LLVM's natural struct layout disagrees with the linked CUDA offsets or `ptxas` rejects the
module, record exact LLVM IR, offsets, and diagnostics and stop rather than inserting manual
padding or textual patches. Generated dumps, PTX, and cubins stay under ignored `build/`. Never
reset unrelated work or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Record fake trace counts, provider IR/layout evidence, runtime output bytes or values, PTX/cubin
sizes, focused/full test counts, next exact fixture stop, and the completed self-review inventory.
Distill the durable copyable aggregate boundary into `docs/design/nvvm-backend.md`.

## Outcomes and Retrospective

This slice advances an existing suite shader with a larger coherent capability rather than one
field operation at a time. The compiler composes the generic ABI introduced in Slice 96 and the raw
buffer/value operations from earlier slices; the LLVM shield did not grow. Exact role separation
and layout comparison keep that generalization honest: the implementation supports compatible
numeric-field aggregates without claiming arbitrary CUDA aggregate layout.

The durable capability ledger is the Slice 97 section in `docs/design/nvvm-backend.md`. Generated
IR dumps, PTX, and cubins remain under ignored `build/`. Slice 98 should start from the measured
opaque Half conversion helpers rather than broad generic-assembly support.
