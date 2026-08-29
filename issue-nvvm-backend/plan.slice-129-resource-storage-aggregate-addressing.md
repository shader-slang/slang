# Address resource-capable aggregates in global and parameter-group storage

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires the completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, direct NVVM compiles and executes three existing compute fixtures whose
harness-specialized final programs select resources or specialized values from aggregate storage:

- `array-existential-parameter.slang`
- `loop-unroll.slang`
- `parameter-block.slang`

The implementation must define one forward-only resource-capable storage-aggregate contract for
fixed arrays and parameter-group structs, then compose existing typed struct/array, pointer,
field/element addressing, load, and resource-view operations. It must not add source-specific
interface recovery, parameter-block callbacks, or array-of-resource callbacks to the provider.

## Progress

- [x] (2026-08-30) Completed Slice 128 as `2cfe46b8a`; the specialized generic fixture passed 3/3,
  its PTX assembled, Release builds passed, and the complete NVVM prefix remained 397/397.
- [x] (2026-08-30) Probed all remaining core compute fixtures through the actual harness and
  selected the three `struct field address` stops for a shared producer audit.
- [x] (2026-08-30) Captured the exact harness-specialized producer shapes and replaced the former
  parameter-group-only array/struct gates with one recursive aggregate-storage contract.
- [x] (2026-08-30) Implemented generic type/layout/address/load support with focused positive and adjacent-negative
  coverage.
- [x] (2026-08-30) Promoted all six direct runtime/PTX lanes, assembled PTX, ran Release/full gates, updated durable status,
  format, self-review, and commit.

## Surprises and Discoveries

- Bare source compilation is not authoritative for these fixtures because their runtime shader
  objects specialize resource and existential layout. All three compile when those bound inputs are
  absent, then stop at `struct field address` in the harness-produced program.
- `loop-unroll.slang` declares a fixed array of three `RWStructuredBuffer<Int>` globals and indexes
  that array before indexing a selected buffer. Its producer is conventional global storage.
- `parameter-block.slang` declares two `ParameterBlock<P>` values where `P` contains one
  `RWStructuredBuffer<Int>`. Its producer is parameter-group storage whose current classifier is
  deliberately numeric-only.
- `array-existential-parameter.slang` specializes two constant-buffer fields containing arrays of
  interface values to concrete `MyImpl { Int val; }` payloads. Its final storage shape is an
  ordinary fixed array in parameter-group storage; the optimized body contains no interface or
  witness operation.
- The existential module retains `global_hashed_string_literals` for reflection even after
  specialization. The C-family emitters intentionally emit nothing for this metadata, so direct
  NVVM now makes the same explicit non-executable classification.
- The fake provider already modeled arrays and raw-buffer views generically, but its array gate and
  resource-view load inference had deliberately excluded their composition. Removing those two
  test-only exclusions was sufficient; production needed no provider callback or ABI revision.

## Decision Log

- Decision: audit and implement the three producer shapes together, but require one generic storage
  algebra rather than three diagnostic-based special cases.
  Rationale: each final operation is keyed field selection followed by fixed aggregate addressing
  or a resource load. LLVM and the existing provider already express those operations generically;
  the compiler-side storage classifiers and layout proofs are the measured gap.
  Date/author: 2026-08-30, Codex.
- Decision: preserve the distinction between value/resource aggregates and CUDA storage layout.
  Rationale: Slice 127 proved first-class resource structs, but that does not automatically prove a
  host-bound constant/parameter-block or fixed resource-array ABI. This slice must validate CUDA
  offsets, stride, alignment, access, and exact element representation at the producer.
  Date/author: 2026-08-30, Codex.
- Decision: replace the parameter-group-only array/struct classifiers rather than add a second
  resource-array family.
  Rationale: conventional globals and parameter groups both lower to the same natural LLVM
  aggregate representation after their producer has established immutable storage. One
  cycle-safe algebra now admits selected numeric/raw-buffer leaves, natural fixed arrays, keyed
  structs, and the already-proven compact Float3 array, while the type-use role still controls
  physical lowering.
  Date/author: 2026-08-30, Codex.
- Decision: keep nested immutable struct address chains and fixed sampler arrays rejected.
  Rationale: the selected fixtures require direct parameter-group resource fields and fixed-array
  selection, not recursive field-address chains or opaque sampler arrays. Existing focused
  negative tests still prove those adjacent boundaries before provider discovery.
  Date/author: 2026-08-30, Codex.

## Context and Current Pipeline

The conventional CUDA global block stores resource views in their physical two-field form: a typed
global data pointer and UInt64 count. Parameter groups are represented as global pointers to their
selected storage struct. Existing direct lowering supports keyed fields in the conventional global
block, numeric parameter-group structs and compact Float3 arrays, fixed numeric/copyable local
arrays, raw resource views, generic LLVM arrays/structs, and typed sequential element pointers.

The current narrow gates do not admit fixed arrays whose elements are resource views or copyable
specialized payloads in CUDA storage, nor parameter-group structs whose fields are resource values.
`_getNVVMStructFieldAddress` therefore reports the common consumer diagnostic before generic
provider mutation even though the physical operations are already representable.

## Scope and Non-Goals

In scope are the exact harness-produced fixed arrays and parameter-group structs in the three named
fixtures; recursive cycle-safe element/field classification; CUDA/provider layout and stride
checks; keyed field addresses; fixed element addressing; immutable versus mutable access; loads of
selected resource/copyable values; focused fake/real-provider coverage if a generic provider
contract changes; direct runtime/PTX lanes; PTX assembly; durable status; Release builds; the full
NVVM prefix; and this plan.

Out of scope are unsized resource arrays, dynamic array lengths, arbitrary interface/witness
recovery, entry-point aggregate parameters, new provider callbacks for parameter blocks or resource
arrays, resource-bearing helper results, general constant-buffer layouts beyond captured shapes,
compatibility aliases, source-name matching, and the unrelated helper-parameter/makeArray fixtures.

## Architecture and Invariants

- Classification begins from the canonical final IR type and is recursive/cycle-safe. It never
  consults a source declaration name, test input string, interface, or witness table.
- Every accepted storage aggregate has an exact CUDA size/alignment/offset/stride proof matching
  the provider representation. A value classifier alone cannot authorize host-bound storage.
- Key identity selects struct fields; declared position is translated once to the exact physical
  index. Conceptually unordered field data is never addressed by assumed source position.
- Fixed arrays reuse the builder's generic LLVM array and sequential-pointer operations. Resource
  views reuse their established typed handle representation and load path.
- Immutability follows constant/parameter-group producers through nested field/element addresses;
  only the fixture's writable resource view permits the final element store.
- Unsupported element kinds, layouts, recursive graphs, and pointer forms stop before provider
  module creation.

## Interfaces and Dependencies

Expected committed areas are direct NVVM type classification/lowering and validation/emission,
focused fake-emitter coverage, the three existing fixtures, `docs/design/nvvm-backend.md`, the
capability ledger, and this plan. The generic builder ABI is expected to remain revision 24 because
its existing type, pointer, aggregate, load/store, and extraction operations express the physical
IR.

## Milestones

1. Capture the three exact final-IR producer/address chains and define one recursive storage-
   aggregate descriptor or classifier with natural layout evidence.
2. Route conventional-global/parameter-group lowering, keyed fields, fixed element pointers, loads,
   reachability, and layout validation through that shared contract.
3. Add focused positive topology and adjacent rejection coverage, then promote all three fixture
   runtime and static PTX lanes.
4. Inspect and assemble every PTX module, run Release builds/full NVVM gates, update docs/log,
   format, perform the input-shape audit, and commit.

## Validation and Acceptance

Acceptance requires focused compiler/provider evidence for the exact aggregate topology and
adjacent invalid layouts; all existing plus new lanes of the three fixtures; inspectable PTX with
the expected resource loads/stores and specialized arithmetic; CUDA 12.9 `ptxas -arch=sm_70`;
Release provider/compiler/unit-test builds; the complete `slang-unit-test-tool/nvvm` prefix; pinned
formatting; and `git diff --check`.

The self-review inventories every new classifier, descriptor, recursion guard, layout helper,
widened consumer, fallback, and special case. Remove any duplicated resource-type list, source-
specific branch, interface reconstruction, syntax recovery, positional field assumption,
unvalidated ABI widening, provider fallback, compatibility shim, or change without a failing
selected fixture/focused test.

## Failure and Recovery

If one producer cannot share the principled storage algebra, preserve its final IR and diagnostic
under ignored `build/slice129-*`, remove only its new directive, and complete the coherent shared
subset. Do not flatten resources in the compiler, copy host storage without a layout proof, weaken
fixture inputs/results, silently use NVRTC, reset unrelated work, or stage
`external/slang-binaries/`.

## Artifacts and Hand-Off

Keep generated IR, LLVM, PTX, cubin, and logs under ignored `build/slice129-*`. Distill the storage-
aggregate contract, exact CUDA evidence, exclusions, and next measured boundary into
`docs/design/nvvm-backend.md`, then commit this plan with the implementation as explicitly
requested.

## Outcomes and Retrospective

Slice 129 is complete. The former numeric-only parameter-group array/struct classifiers are gone.
One cycle-safe aggregate-storage algebra now recognizes selected integer/Float32/32-bit-vector and
raw-buffer leaves, natural fixed arrays, recursively keyed structs, and the established explicit
12-byte Float3 array. Type lowering composes the existing generic array/struct/pointer
representations, while an independent recursive layout calculation proves provider size,
alignment, offsets, and strides against CUDA before module creation.

The three measured producer chains now execute without source-specific recovery:

- the specialized existential constant buffers contain `MyImpl { Int val; }[2]` storage;
- the loop fixture indexes a conventional-global `RWStructuredBuffer<Int>[3]` and loads the
  selected two-word view;
- the parameter-block fixture loads a global pointer to `P`, then the `RWStructuredBuffer<Int>`
  field inside `P`.

The field and sequential-pointer resolvers preserve immutable provenance throughout these chains.
The fake-emitter test observes one generic resource-view array type, a keyed field pointer, a
sequential element pointer, an aggregate load, and existing resource field extraction. Fixed
sampler arrays plus nested immutable parameter-group/constant-buffer field chains remain the
adjacent E52017 negative evidence. Builder ABI remains revision 24.

All six new direct lanes pass: one runtime comparison and one PTX/FileCheck lane for each fixture.
The existential, resource-array, and parameter-block PTX modules are 863, 960, and 793 bytes; CUDA
12.9.86 `ptxas -arch=sm_70` assembles them to 2,920-, 2,792-, and 2,920-byte cubins. The pinned
formatter, Release provider build, Release `slangc`/`slang-test`/`slang-unit-test` builds, focused
tests, `git diff --check`, and complete NVVM prefix all pass; the final prefix result is 398/398.

Self-review inventory:

- The new recursive classifier survives because removing it restores the three measured field-
  address failures; its active set rejects recursive graphs and it accepts no source names,
  interfaces, witnesses, or arbitrary explicit stride.
- The aggregate layout helper survives because LLVM arrays cannot represent arbitrary explicit
  padding. It independently derives the provider representation and compares it with CUDA before
  builder mutation.
- The widened conventional field, parameter-group field, sequential-element, reachable-type, and
  fake-provider branches each correspond to one producer or generic consumer in the three traces.
- The hashed-string metadata case survives because the frontend deliberately retains reflection
  data that every C-family emitter ignores for executable output; it is not an IR repair or runtime
  fallback.
- No builder callback, ABI change, compatibility alias, source-specific branch, positional field
  assumption, interface reconstruction, or syntax recovery remains.
