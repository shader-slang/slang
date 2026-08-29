# Address mutable structured-buffer aggregate fields

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM composes its existing copyable-struct resource representation with
generic struct-field and sequential-element addressing. A canonical writable
`RWStructuredBuffer<S>` element whose direct fields are selected numeric values can be addressed
by field and vector lane without reconstructing a whole aggregate. The existing
`tests/compute/write-structured-buffer-field.slang` fixture should pass direct runtime and PTX
lanes with its unchanged output.

## Progress

- [x] (2026-08-29) Reprobed the remaining CUDA compute corpus after Slice 116 and grouped its first
  boundaries into helper ABI, conventional resource/parameter fields, wide values, packing,
  atomics, and harness-provided dynamic-specialization inputs.
- [x] (2026-08-29) Selected the structured-buffer field fixture and audited its final producer
  shape: a layout-compatible two-`int4` struct, two canonical writable element pointers, direct
  keyed field addresses, one constant vector-lane address, one vector load/extract, and one scalar
  store.
- [x] (2026-08-29) Generalized the existing struct-field and sequential-pointer resolvers for
  writable copyable resource elements without changing the builder ABI or widening immutable
  physical storage.
- [x] (2026-08-29) Made accepted raw resource parameters retain their own aggregate element type,
  removing an accidental dependency on a same-typed local variable.
- [x] (2026-08-29) Added focused positive and adjacent negative coverage, promoted and validated
  the real fixture, assembled PTX, ran the full NVVM prefix, documented, formatted, and
  self-reviewed.

## Surprises and Discoveries

- The conventional global-parameter field is already accepted. The generic raw-buffer classifier
  already admits layout-compatible copyable structs, and Slice 97 proved whole-aggregate stores.
  The diagnostic is emitted by the later field address rooted at the resource element because that
  resolver currently admits only the sole-array physical matrix wrapper in this role.
- The next lane address is also an established operation, but its base has the canonical explicit
  scalar-layout pointer spelling produced by raw structured-buffer legalization. The local numeric
  pointer classifier intentionally accepts only compact local pointers, so the sequential resolver
  must derive this writable vector base from the already validated field-address producer.
- Physical matrix storage uses the same resource-element branch but remains deliberately
  immutable at the logical level. Resource mutability must therefore be established by the
  copyable-struct family, not inferred from every read-write resource pointer.
- The focused raw-kernel test initially reached a retained global `struct` failure even though the
  same aggregate resource type passed when Slice 97's shader also declared a local of that type.
  Retained-type selection handled conventional resource fields but did not treat raw function
  parameters as producers of their element declaration. A shared resource-element selector now
  gives both entry spellings the same dependency and layout check.
- All five explicit native/direct lanes of the promoted fixture pass. Its automatically synthesized
  WebGPU lane still fails Dawn bind-group-layout creation on this machine, so the two new direct
  lane IDs were rerun independently and provide this slice's runtime/PTX evidence.

## Decision Log

- Decision: extend the existing `NVVMStructField` result with role-neutral mutability and consume
  it in pointer validation and sequential addressing.
  Rationale: local copyable structs and writable copyable resource elements share the same field
  contract, while conventional parameter fields and physical matrix storage remain immutable.
  One producer-aware property avoids adding a resource-specific builder callback or a second
  field classifier.
  Date/author: 2026-08-29, Codex.
- Decision: keep the accepted aggregate family exactly equal to Slice 97's layout-compatible,
  first-level selected-numeric copyable structs.
  Rationale: that family is already the canonical resource type and layout source of truth.
  Nested structs, Boolean storage, arrays, incompatible layouts, and arbitrary aggregate graphs
  retain their existing pre-provider rejection.
  Date/author: 2026-08-29, Codex.
- Decision: derive retained aggregate declarations from every accepted structured-buffer view,
  whether the view is a conventional global field or a raw entry parameter.
  Rationale: the resource signature is the semantic source of truth for its element type. Requiring
  a local aggregate to keep that declaration alive was an accidental producer-consumer coupling.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The direct compiler now treats field mutability as a role-neutral property of the resolved
producer. Writable copyable resource elements and mutable locals admit selected numeric fields;
conventional parameter fields and physical matrix wrappers retain their immutable rules. A vector
lane address can reuse an exact writable field producer even when raw-buffer legalization gives
its pointer the explicit scalar-layout spelling. No arbitrary explicit pointer is accepted, and
no builder or LLVM-provider interface changed.

Focused fake coverage records two raw-view extracts, two aggregate pointer offsets, semantic field
indices `1, 0`, one sequential lane pointer, one aligned `int4` load/extract, and one scalar store.
The existing whole-aggregate and physical-matrix tests pass, and the incompatible copyable layout
still stops before provider mutation. The full NVVM prefix passes 385/385.

`write-structured-buffer-field.slang` passes direct runtime with the unchanged 32 integers except
that each `b.y` lane becomes the first element's `a.x` value `1`. Its direct PTX lane contains the
selected kernel, a vector global load, and a scalar global store. The 681-byte PTX module assembles
with CUDA 12.9.86 `ptxas -arch=sm_70` to a 2,792-byte cubin.

The next bounded corpus choices remain separate: associated-type lookup first stops at a helper
result type, constant-buffer packing stops at another struct-field shape, and 64-bit bitcasts stop
at a wide helper parameter. None is folded into this resource-field slice.

Self-review inventory:

- The `NVVMStructField::isMutable` generalization survives. The existing resolver proves the base
  pointer family, semantic key, physical index, and exact result pointee before setting it. Removing
  the resource case restores the fixture's field-address rejection; treating every resource field
  as mutable would incorrectly widen physical matrix storage and was not done.
- The sequential-pointer field-producer branch survives. It is reached only when the compact local
  numeric-pointer classifier rejects the raw resource's explicit scalar-layout spelling, then
  reuses `_getNVVMStructFieldAddress` rather than independently accepting an arbitrary pointer.
  Removing it restores the fixture's lane-address failure.
- `_getNVVMRawBufferAggregateElementType` survives as one source of truth for the aggregate
  declaration owned by accepted conventional and raw structured-buffer views. Both callers apply
  the existing CUDA/LLVM layout proof before retention. Removing the raw-parameter use makes the
  focused shader fail at its retained `struct`, proving that the resource producer owns the fix.
- Generic field diagnostics replace the former conventional/local labels because the resolver now
  serves conventional, local, physical-resource, and mutable-resource roles. No control flow,
  textual IR rewrite, custom equivalence, syntax reconstruction, graph walk, or silent fallback was
  added.

## Context and Current Pipeline

Consider the existing fixture:

    struct S
    {
        int4 a;
        int4 b;
    };

    outputBuffer[tid].b.y = outputBuffer[0].a.x;

After CUDA resource legalization and global collection, final IR loads the established raw resource
view from `GlobalParams`, produces `Ptr<S, ReadWrite, Generic, ScalarLayout>` twice, selects fields
`b` and `a` by semantic key, takes lane one of `b`, loads `a`, extracts lane zero, and stores the
scalar. `S` has CUDA offsets 0 and 16 and a 32-byte stride compatible with LLVM. Every provider
operation needed by this chain already exists.

## Scope and Non-Goals

In scope are direct keyed field addresses rooted in accepted writable copyable-struct
`RWStructuredBuffer` elements; selected numeric scalar/vector fields; sequential vector-lane
addresses derived from those writable fields; exact layout preflight; focused fake-provider
evidence; and runtime/PTX promotion of the existing fixture.

Out of scope are read-only structured-buffer mutation, fields of physical matrix wrappers, nested
aggregates, arrays as copyable-struct fields, Boolean storage, incompatible CUDA/LLVM layouts,
whole-aggregate construction, new resource types, new builder callbacks, and unrelated helper or
parameter-block ABI widening.

## Architecture and Invariants

- `_getNVVMStructFieldAddress` remains the sole semantic-key and pointer/result-type validator for
  every admitted struct field.
- A resource field is writable only when its base is the canonical accepted read-write element
  pointer and its pointee is the existing copyable-struct family. The physical-array wrapper keeps
  its prior immutable contract.
- `_getNVVMSequentialElementPointer` may reuse a field pointer only after resolving that exact
  producer and observing writable numeric-vector field type. It does not classify an arbitrary
  explicit-layout pointer as local.
- Pointer-consumer validation uses the resolved field's mutability. Stores to conventional
  parameter fields and immutable physical resource fields remain rejected before emission.
- The provider receives only existing typed struct-field and sequential-element pointer calls.

## Interfaces and Dependencies

No builder ABI or LLVM provider change is planned. The direct emitter's two structural resolvers,
the fake-provider shader/test, the existing compute fixture, durable design status, and this plan
are the expected committed files. CUDA 12.9 runtime and `ptxas` provide semantic and assembly
evidence.

## Milestones

1. Generalize field mutability and admit the exact writable copyable-resource field producer.
2. Reuse that producer in sequential vector-lane addressing and pointer-consumer validation.
3. Add focused fake traces plus an adjacent incompatible-layout regression, then promote the
   existing runtime/PTX fixture.
4. Format, build, run focused/full/runtime/PTX/`ptxas` validation, update durable status and this
   plan, self-review, and commit.

## Validation and Acceptance

Acceptance requires Release host builds; focused fake evidence for two resource element pointers,
semantic field indices, vector-lane addressing, typed load/extract/store, and zero new provider
features; continued rejection of the incompatible copyable layout before provider mutation; direct
runtime/PTX lanes for the existing fixture; CUDA 12.9 `ptxas -arch=sm_70`; the full
`slang-unit-test-tool/nvvm` prefix; pinned formatting; and `git diff --check`.

The self-review inventories every new helper, flag, branch, and diagnostic adjustment. Confirm the
accepted pointer spelling is produced by raw resource legalization, reuse the existing layout and
copyable-struct sources of truth, and remove any general explicit-pointer fallback or physical
matrix mutability leak.

## Failure and Recovery

If the next canonical stop requires a new provider operation or runtime semantics differ, preserve
the exact probe under ignored `build/slice117-*`, narrow this slice to the independently proven
field-address subset, and record the next boundary. Do not patch emitted LLVM text, infer source
syntax, reset unrelated work, or stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, PTX, cubin, and logs under ignored `build/slice117-*`. Distill the final mutable
resource-field contract, validation evidence, and next measured corpus boundary into
`docs/design/nvvm-backend.md`, then commit this plan with the implementation as explicitly
requested.
