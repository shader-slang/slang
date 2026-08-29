# Add sampled texture dimension queries

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles and runs
`tests/compute/texture-get-dimensions-cuda.slang`. The existing typed texture-operation interface
covers width, height, and depth queries for the fixture's six scalar Float texture shapes. Exact
mutable UInt locals and `OutParam<UInt>` helper parameters carry query results without transporting
CUDA inline assembly through the LLVM boundary.

## Progress

- [x] (2026-08-29) Reproduced the Slice 102 boundary and captured all six finalized query helper
  signatures, GenericAsm bodies, call shapes, and ordinary CUDA/PTX reference output.
- [x] (2026-08-29) Built the provider and host, then measured the first direct fixture compile. The
  finalized entry point retains a canonical integer `switch`, so the previous assumption that its
  control flow was already supported was false.
- [x] (2026-08-29) Generalized exact local numeric pointer transport for canonical generic-space
  `var` and `OutParam` shapes, with producer checks that exclude identically spelled globals.
- [x] (2026-08-29) Extended the texture descriptor operation family with width, height, and depth
  queries and mapped them to LLVM's shared NVVM query intrinsics.
- [x] (2026-08-29) Recognized complete dimension-query helpers, persisted their query sequence,
  emitted stores to checked output pointers, and preserved CUDA's explicit zero for unavailable
  array size.
- [x] (2026-08-29) Added real/fake API coverage, a vector-texture negative, and direct runtime/PTX
  lanes to the existing fixture.
- [x] (2026-08-29) Formatted, built, validated focused/full/changed tests and `ptxas`, self-reviewed,
  and updated durable docs.

## Surprises and Discoveries

- Final IR retains three entry-local `Ptr<UInt>` variables initialized to zero. Query helpers take
  the corresponding `OutParam<UInt>` pointers and return Void, so this fixture measures a real
  pointer-ABI boundary in addition to texture queries.
- Non-array 1D queries width; 2D and cube query width/height; 3D queries
  width/height/depth. The finalized 2D-array and cube-array helpers also take a third output but
  deliberately store zero because CUDA's source implementation does not expose `txq.array_size`.
- LLVM 7.0.1 and LLVM 14.0.6 expose identical `llvm.nvvm.txq.width`, `height`, and `depth`
  signatures: i32 result from one i64 texture handle. The ordinary CUDA reference emits the same
  `txq.*.b32` PTX instructions.
- The first direct compile reaches `IRSwitch` before any query helper boundary. This is canonical
  source control flow with an integer selector, literal labels, ordinary target blocks, and no
  edge arguments; retaining it is more direct than introducing a texture-fixture-specific CFG
  rewrite.
- The first query-emission attempt reached the provider but reversed the generic store's value and
  pointer operands. Correcting the caller to the construction API's established
  `emitStore(value, pointer)` contract made the complete module verify.
- LLVM's generated intrinsic table assigns `txq.width/height/depth` exactly `nounwind readnone`,
  not the six modern optimization attributes used by scalar `sqrt`. LLVM 7 has the same query
  signature and attributes, so validation is sufficient and adding these declarations to the
  legacy attribute-rewrite count was incorrect.
- A pointer type alone cannot establish local storage ownership: a groupshared global reached
  validation with the same canonical `Ptr<Int>` spelling. The final classifier requires generic
  address space and the exact local `var` or helper-parameter producer. The existing group-shared
  negative consequently retains its earlier pointer diagnostic.
- A broad `tests/compute/texture` run also selected two unrelated WebGPU lanes that fail on this
  machine with invalid bind-group layouts. The exact changed texture lanes and the complete NVVM
  prefix pass independently; no WebGPU files or behavior changed in this slice.

## Decision Log

- Decision: add query operations to the existing texture descriptor rather than a new callback
  family or inline-assembly escape.
  Rationale: operation kind, texture shape, arrayness, and semantic texture element type remain the
  complete capability key. Query results have a fixed UInt32 type and use the same opaque handle.
  Date/author: 2026-08-29, Codex.
- Decision: generalize local numeric pointer transport at the canonical pointer producer.
  Rationale: `var UInt` and `OutParam<UInt>` are valid checked IR shapes used for many output-style
  helpers. The provider already owns typed local storage, load, store, and generic pointers; a
  texture-specific temporary representation would duplicate those primitives.
  Date/author: 2026-08-29, Codex.
- Decision: preserve the array helper's explicit zero output instead of emitting array-size query.
  Rationale: the finalized CUDA GenericAsm body is the semantic source for this fixture and states
  that array size is unavailable. Substituting a different intrinsic would change observable CUDA
  behavior and overclaim the selected helper mapping.
  Date/author: 2026-08-29, Codex.
- Decision: advance the forward-only builder ABI and add one generic integer-switch construction
  callback.
  Rationale: LLVM already represents this exact canonical terminator. A callback taking one typed
  integer selector, parallel constant/target arrays, and a default target keeps structural IR in
  the shielded provider and avoids rebuilding a chain of synthetic compare blocks in the compiler.
  Date/author: 2026-08-29, Codex.

## Context and Current Pipeline

Slice 102 admits scalar Float sampled texture handles and one descriptor-driven SampleLevel
operation. The next fixture reaches six retained `_Texture.GetDimensions` helpers. Their exact
bodies use `txq.width.b32`, `txq.height.b32`, and `txq.depth.b32` inline PTX, followed by stores
through output parameters. Array helpers query only width and height and write zero to the trailing
element-count output.

The entry function creates width, height, and depth UInt locals, initializes them to zero, passes
their pointers to helpers, reloads the selected results, packs them into the output buffer, and
joins six switch cases. All scalar arithmetic, control flow, raw output buffer access, sampled
texture storage, and texture handle transport are already supported.

## Scope and Non-Goals

In scope:

- exact canonical local pointers to selected numeric values and matching helper `OutParam` types;
- canonical integer switches with literal case values and non-parameterized case/default targets;
- scalar UInt local allocation/load/store and pointer call transport;
- Float texture width/height/depth queries for the six fixture shapes;
- exact helper text/signature matching, typed capability preflight, explicit array-output zero;
- direct runtime, PTX FileCheck, LLVM verification, libNVVM, and `ptxas` evidence.

Out of scope:

- array size, mip count, sample count, channel order/type, surface queries, or mip-level overloads;
- query return vectors or user-authored arbitrary output-pointer helpers;
- comparison, multisample, shadow, combined, feedback, integer, vector-element, or Half textures;
- Boolean, vector, dynamic-label, parameterized-target, or edge-argument switches;
- pointer escape, pointer SSA/phi, pointer arithmetic on locals, aliasing between query outputs,
  nested pointees, or backward compatibility.

## Architecture and Invariants

- The local numeric pointer classifier accepts only canonical one-operand `Ptr<T>` and
  `OutParam<T>` shapes where `T` is an already selected numeric value.
- Entry locals are allocated from the `var` producer with the established type/alignment helpers.
  Helper pointer parameters lower to typed generic LLVM pointers. A call accepts `Ptr<T>` for
  `OutParam<T>` only when the canonical pointee types agree.
- Dimension helper resolution checks exact Void result, texture type/shape/arrayness, output count,
  UInt pointees, and the complete GenericAsm string. Preflight persists the ordered query list and
  whether the final output is the source-defined zero.
- Canonical `IRSwitch` selectors and labels lower directly to one provider switch terminator. The
  compiler validates exact integer types, literal labels, target ownership, and target parameters
  before module mutation; the provider repeats LLVM ownership/type checks.
- Provider query callbacks accept one usable i64 handle and return i32. Descriptor validation and
  intrinsic selection happen before LLVM mutation.
- Emission consumes only the persisted requirement, stores each query result to its corresponding
  checked output pointer, emits the explicit zero when required, and returns Void.

## Interfaces and Dependencies

Advance the builder ABI to revision 15 for one required generic integer-switch construction
callback. New texture operation enum values continue to use the existing descriptor and query/emit
callbacks. Update facade operand-count validation, real and fake providers, API tests, texture
requirements, local pointer type lowering, helper/call/CFG validation, emission, the existing
GetDimensions fixture, a focused negative fixture, and
`docs/design/nvvm-backend.md`.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. CMake builds, tests, and native formatting run outside the sandbox per
repository instructions.

## Validation and Acceptance

Acceptance requires focused pointer/query API and compiler tests; the complete
`slang-unit-test-tool/nvvm` prefix; ordinary and direct CUDA runtime lanes for
`texture-get-dimensions-cuda.slang`; direct PTX checks for width, height, and depth; a negative
adjacent query shape; LLVM verification; CUDA 12.9 `ptxas`; pinned clang-format 17; and
`git diff --check`.

Record exact counts, expected runtime words, PTX/cubin sizes, query instructions, self-review, and
the next measured fixture boundary as work completes.

## Self-Review and Input-Shape Audit

Inventory the local numeric pointer classifier, helper argument relation, query resolver,
persisted ordered operation sequence, provider intrinsic selector, output stores, and explicit
zero. For each, confirm the exact producer, canonicality, semantic source of truth, and failing
test. Reject arbitrary pointer recovery, alias guessing, GenericAsm forwarding, or use of a query
not present in the finalized source helper.

The final inventory keeps all of those additions. The pointer classifier initially overclaimed a
groupshared global from type alone; it now survives only with generic address space plus a local
`var` or helper-parameter producer, as proven by the restored group-shared negative. The helper
argument relation is the language-defined `Ptr<T>` to `OutParam<T>` relation with identical
canonical pointees. Switch validation consumes the canonical terminator and literal labels
directly. Query resolution requires the complete finalized helper signature and body, then emission
uses only the persisted descriptors, checked parameters, and source-defined trailing zero. No
syntax reconstruction, arbitrary operand walk, alias inference, GenericAsm forwarding, or fallback
was retained.

## Failure and Recovery

If LLVM verification, libNVVM, runtime comparison, or `ptxas` rejects a query mapping, preserve the
exact IR/PTX/diagnostic and stop the loop. Generated dumps, CUDA, PTX, and cubins stay under ignored
`build/`. Never reset unrelated work or stage `external/slang-binaries/`.

## Outcomes and Retrospective

Slice 103 advances the forward-only builder ABI to 15 and adds one generic integer-switch
construction callback. The real provider validates type, ownership, dominance, literal labels,
unique cases, and targets before mutation. Normal LLVM 14 and LLVM 7-compatible assembly both
retain the switch and the three exact query declarations.

Direct NVVM now accepts selected numeric local `Ptr<T>` storage and exact helper `OutParam<T>`
parameters, while rejecting nonlocal producers with the same type spelling. Six complete scalar
Float `GetDimensions` helper shapes map to ordered width/height/depth descriptors. Array helpers
write the CUDA source-defined trailing zero rather than claiming array-size support.

Validation evidence:

- standalone Release provider and Release `slangc`/`slang-unit-test` builds pass;
- focused real/fake builder, runtime, PTX, negative, and unsupported-shape tests pass 6/6, with the
  pointer-regression rerun passing 5/5;
- the complete `slang-unit-test-tool/nvvm` prefix passes 375/375;
- ordinary and direct runtime produce `4, 2056, 131586, 4112, 1028, 4112, 0`;
- optimized direct PTX is 3,165 bytes and contains six `txq.width.b32`, five
  `txq.height.b32`, and one `txq.depth.b32`;
- CUDA 12.9.86 `ptxas -arch=sm_70` accepts that PTX and emits a 4,328-byte cubin;
- pinned clang-format 17 and `git diff --check` pass.

The next slice should measure the adjacent existing sampled-texture subscript fixture. It will
reuse the now-established integer switch and local numeric pointer paths if present, while treating
the retained texture-fetch helper as a separate semantic operation family.
