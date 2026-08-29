# Admit fixed numeric-array byte-address values

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct libNVVM treats a nonempty fixed array of one selected numeric scalar or
vector element type as a bounded pass-through value for byte-address loads and stores. The family
composes the existing generic array type, byte-offset pointer, and typed load/store operations; it
does not add array construction/extraction operations or change builder ABI revision 8.

The existing `tests/compute/byte-address-buffer-array.slang` must compile through direct libNVVM,
execute through the CUDA comparison harness, expose its scalarized Float/Float4 copy in direct PTX,
and pass CUDA 12.9 `ptxas -arch=sm_70`.

## Progress

- [x] (2026-08-29) Completed and committed Slice 85 as `4b51f3309` with 355/355 NVVM tests.
- [x] (2026-08-29) Compared the aggregate byte-address and vector-binary suite boundaries.
- [x] (2026-08-29) Captured the array shader's final graph: one direct
  `Array<Float4, 2>` byte load/store followed by established scalar Float loads, Float4
  construction, and Float4 stores.
- [x] (2026-08-29) Confirmed the real provider's generic array constructor already accepts any
  sized, loadable/storable element type.
- [x] (2026-08-29) Added one exact nonnested fixed numeric-array classifier and used it for value
  lowering and byte-address access.
- [x] (2026-08-29) Generalized fake-provider array identity and added positive pass-through plus
  nested rejection coverage.
- [x] (2026-08-29) Registered the existing shader for direct runtime/PTX evidence, repaired its
  backing allocation, and ran real libNVVM/`ptxas`.
- [x] (2026-08-29) Formatted, built, ran focused and complete validation, updated durable documents,
  self-reviewed, and prepared the completed slice for commit.

## Surprises and Discoveries

- Target legalization removes the source `Block` wrapper from the final byte operations. The wide
  access is exactly `Array<Float4, 2>`; the scalar-alignment path is already decomposed into eight
  Float loads, two Float4 constructors, and two Float4 stores. General struct construction or field
  extraction is not a prerequisite for this shader.
- The provider's `getArrayType` validates only module ownership, nonzero count, sized loadable
  element type, and LLVM array legality. Its integer-only appearance is a host/fake policy, not an
  LLVM or builder-ABI limitation.
- The existing test allocates 16 bytes even though its established accesses reach byte 47. The
  direct runtime lane needs a 48-byte backing buffer so the test measures compiler behavior rather
  than out-of-bounds device memory.
- Generic aggregate legalization retains the wide array value but canonicalizes its natural source
  load to the ordinary two-operand byte-load form. The direct boundary therefore receives the same
  four-byte no-promise alignment contract as other plain byte operations.
- `tests/cuda/cuda-vector-binary-ops.slang` begins at `shl` but then composes integer shifts,
  signed division/remainder, Boolean vectors, narrow vectors, Float vector arithmetic, and Float
  remainder. It is a broader operation-family slice and remains the next measured candidate rather
  than being split into one-operation increments.
- libNVVM accepts the one-level array load/store module, then removes the same-location wide copy.
  The other source path remains as eight Float loads and eight Float stores in final PTX; focused
  fake evidence retains the otherwise-optimized aggregate boundary.

## Decision Log

- Decision: admit exact nonempty `IRArrayType` values whose direct element is an established
  byte-address scalar (32-bit numeric or 64-bit integer) or two- through four-lane 32-bit numeric
  vector.
  Rationale: this is the canonical final shape produced for the existing shader and maps directly
  to the provider's generic LLVM array type. Restricting the element to the existing value family
  avoids inferring new scalar/vector policy from LLVM permissiveness.
  Date/author: 2026-08-29, Codex.
- Decision: admit the array only as an ordinary value and byte-address payload, not as a new entry,
  helper, structured-buffer, local-storage, pointer, construction, extraction, or arithmetic role.
  Rationale: the motivating graph passes one loaded array directly to a store. Broader roles have
  different ABI, SSA, and operation contracts and are not needed for observable suite progress.
  Date/author: 2026-08-29, Codex.
- Decision: keep nested arrays and array-bearing structs outside the admitted payload family.
  Rationale: a direct selected numeric element gives one bounded, nonrecursive classifier and one
  provider type construction. Recursive aggregates should be added only with explicit layout and
  value-flow evidence.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

Slice 86 establishes one nonrecursive fixed numeric-array byte payload without changing builder ABI
revision 8 or the real provider. Exact leaf policy, literal count, and direct element identity are
validated before the existing generic array type, byte pointer, and load/store operations run. The
focused fake test observes `Array<Float4, 2>` pass from an invariant load to a store, while a nested
array remains a pre-provider E52017 control. Two new and two adjacent unit tests pass 4/4.

The existing array shader passes its direct runtime and PTX lanes 2/2 with a corrected 48-byte
backing allocation. Real libNVVM accepts the array module, removes the same-location copy, and emits
eight Float loads plus eight Float stores for the scalar-alignment path. CUDA 12.9.86
`ptxas -arch=sm_70` accepts the module. The standalone provider and Release `slang-unit-test`,
`slang-test`, and `slangc` targets build successfully. The complete NVVM prefix passes 356/356;
pinned clang-format 17 and `git diff --check` pass.

The final self-review inventory found three intentional classifier/validation changes. The private
byte-leaf helper survives as the single source of the 32-bit numeric plus 64-bit integer policy for
both direct and array payloads; it prevents accidental narrow-element admission. The fixed-array
classifier validates canonical nonrecursive `IRArrayType` shape/count and is narrowed by the old
i32 device-array classifier rather than duplicated. Array store validation consumes an already-
available exact aggregate produced by the canonical load; it does not rebuild or inspect elements.
The fake records array identity when its type/pointer/load producers run. The fixture allocation
repair only makes its pre-existing memory range valid. No fallback, source-name match, graph walk,
recursive shape recovery, or malformed-IR workaround remains.

## Context and Current Pipeline

Consider the existing source:

```slang
struct Block { float4 val[2]; };
buffer.Store(0, buffer.LoadAligned<Block>(0));
buffer.Store<Block>(16, buffer.LoadAligned<Block>(4, 4), 16);
```

For the naturally aligned first copy, final linking strips the one-field `Block` wrapper and keeps
one canonical `Array<Float4, 2>` byte load followed by a byte store of that same array value. For
the alignment-four second load, byte-address legalization emits eight Float loads and two Float4
constructors; the aligned stores consume those Float4 values directly. Slice 84 already handles
that scalar/vector branch.

The direct emitter currently rejects the wide array at `_getNVVMByteAddressAccess` because the
payload classifier names only selected scalar/vector types. Type lowering separately restricts
ordinary array values to the fixed signed-i32 array family introduced for device array pointers.
The provider callback itself is generic and no final instruction constructs or extracts the array.

## Scope and Non-Goals

In scope are exact nonempty fixed arrays with a direct selected numeric scalar or bounded 32-bit
numeric-vector element; ordinary value lowering; read-only/read-write byte loads and read-write
stores; established literal alignment and invariant policy; fake element/count identity; the named
existing shader; direct runtime/PTX; and `ptxas` validation.

Out of scope are nested arrays; array-bearing structs as direct payloads; runtime-sized or unsized
arrays; array construction, extraction, indexing, arithmetic, comparison, phi, helper/entry ABI,
structured-buffer elements, local/shared/global storage, or new device-pointer shapes; matrices;
Boolean or unsupported numeric elements; status loads/atomics; runtime alignment; and bounds repair
beyond correcting the named fixture's allocation.

## Architecture and Invariants

One classifier resolves only canonical `IRArrayType` with a literal count in `[1, UINT32_MAX]` and
a direct element accepted by the established byte-address leaf family. It does not recurse. The
existing signed-i32 device-array classifier narrows this broader type family for pointer-specific
roles, so one mapping owns array shape/count validation.

The type lowerer creates the exact provider element type and calls the existing generic array type
operation. Byte access creates a pointer to that exact array and loads/stores the first-class array
value. Store SSA validation accepts only an already-available value with the same exact canonical
type relation established by the descriptor; it does not reconstruct elements or source structs.

The fake provider records the array element type and count and gives the resulting array its own
value kind. A load through an array byte pointer therefore satisfies only a matching array store,
not a scalar/vector consumer.

## Interfaces and Dependencies

Update `source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` with the bounded numeric-array
classifier, reuse it inside the existing i32 array classifier, and admit it only for ordinary value
lowering and byte payloads. Update byte-store SSA validation in `source/slang/slang-emit-nvvm.cpp`
to validate an array as an exact available aggregate rather than as a scalar/vector numeric value.

Generalize only the fake provider's existing array type/value identity under
`tools/slang-unit-test/`. No real-provider, facade, callback, semantic operation, feature flag, ABI
revision, or compatible-text rewrite is expected. Add direct CUDA comparison and PTX lanes to the
existing shader and keep generated artifacts under ignored `build/nvvm-slice86/`.

## Milestones

1. Add the canonical bounded numeric-array classifier and reuse it for i32 array narrowing.
2. Generalize value lowering and byte access while keeping every other array role unchanged.
3. Generalize fake array identity, prove direct load-to-store flow, and retain a nested-array
   pre-provider rejection.
4. Expand the existing fixture allocation and register direct CUDA runtime/PTX lanes.
5. Compile through real libNVVM, inspect optimized PTX, and run CUDA 12.9 `ptxas -arch=sm_70`.
6. Run focused regressions and the complete NVVM prefix, update durable design/ledger records,
   self-review, and commit this plan with the implementation.

## Validation and Acceptance

Run all CMake builds and tests outside the sandbox. Acceptance requires:

- exact numeric array element/count lowering calls the established generic array type operation;
- read-only and read-write array byte loads plus read-write stores use exact generic byte pointers,
  alignment, and load flags;
- a nested numeric array remains deterministic E52017 before builder discovery or mutation;
- established signed-i32 device-array pointer tests remain unchanged;
- the existing shader's direct CUDA lane matches its established comparison result using a valid
  48-byte backing allocation;
- direct PTX passes FileCheck, and CUDA 12.9 `ptxas -arch=sm_70` accepts it;
- standalone provider and Release host/test builds pass;
- focused tests and the complete `slang-unit-test-tool/nvvm` prefix pass;
- pinned clang-format 17 and `git diff --check` pass; and
- `external/slang-binaries/` and generated `build/` artifacts remain unstaged.

## Failure and Recovery

If libNVVM rejects an aggregate array load/store, compare normal LLVM 14 text, NVVM-compatible
text, and final PTX. Do not scalarize in the direct emitter: target byte-address legalization owns
alignment-driven scalarization, while the retained array operation is the canonical aligned shape.

If the fake array type collides with the historical i32 device-array handle, add element-kind/count
identity at the type producer and make load/store consume that identity; do not infer array contents
from later uses. All changes are one forward-only slice and can be reverted together; ABI revision
8 remains unchanged.

## Artifacts and Hand-Off

Retain final linked IR, direct PTX, CUDA runtime output, and `ptxas` artifacts under ignored
`build/nvvm-slice86/`. Distill the fixed numeric-array value boundary, existing-file results,
remaining recursive aggregate and vector-operation boundaries, and exact validation totals into
`docs/design/nvvm-backend.md` and `docs/design/nvvm-backend-capability-ledger.md` before committing
Slice 86.
