# Slice 29: Centralize NVVM type legalization and caching

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user requires each
completed slice plan to ship with its implementation, so this plan will be committed with Slice 29.
It is queued behind Slice 28 and does not become the active ExecPlan until Slice 28 is committed.

## Purpose and Observable Result

After this slice, direct NVVM has one explicit owner for mapping canonical linked Slang IR types to
provider type handles. Function signatures, parameters, constants, phis, loads/stores, pointers,
arrays, and the raw resource ABI no longer each hardcode or cache `i32` independently inside
`slang-emit-nvvm.cpp`.

The accepted semantic subset remains exactly Slice 26: void entry results, signed `int`, comparison
Booleans where already produced, device pointers to `int`, fixed `int` arrays behind device
pointers, and raw `RWStructuredBuffer<int>` launch values. The observable result is structural:
all established tests pass through the centralized legalization/cache context, repeated canonical
IR types reuse one provider handle, and unsupported adjacent types still stop before module
mutation or provider discovery as their current contract requires.

## Progress

- [x] (2026-08-27) Audited the current `_isI32Type`, supported-pointer/array/resource classifiers,
  lazy `i32Type`, device-pointer singleton, per-array dictionaries, and raw-resource singleton.
- [x] (2026-08-27) Chose centralized lowering keyed by canonical `IRType*`, without creating a
  second semantic type tree.
- [x] (2026-08-27) Defined exact entry-result, helper-result, entry-parameter,
  helper-parameter, and value-use contracts and added module-local cache evidence.
- [x] (2026-08-27) Migrated signatures, constants, phis, and all value paths to one
  `NVVMTypeLoweringContext`; deleted the emitter's five ad hoc type caches/helpers.
- [x] (2026-08-27) Proved cache identity, adjacent rejection, full 193/193 focused/provider/PTX/
  `ptxas`/runtime behavior, Debug preservation 10/10, and documented the future type-extension
  contract.

## Surprises and Discoveries

- Observation: the provider API already has generic integer, pointer, and array constructors, but
  the Slang emitter only asks for one hand-managed `i32` graph.
  Evidence: `_getNVVMI32Type`, `_getNVVMDeviceArrayPointerType`, `i32Type`,
  `deviceI32PointerType`, `arrayTypeMap`, and `arrayPointerTypeMap` separately manage overlapping
  pieces of the same canonical type graph.
  Consequence: the first type-layer slice can preserve provider behavior and concentrate ownership
  before adding floating point, vectors, matrices, structs, or address spaces.

- Observation: LLVM integer types are signless while Slang integer operations are not.
  Evidence: the provider builds LLVM `i32`; signedness appears in comparisons and future division,
  shift, and extension policies.
  Consequence: the type layer maps representation width and shape. Operation legalization remains
  responsible for signed/unsigned semantics and must not encode signedness by inventing distinct
  LLVM integer types.

- Observation: canonical read and read-write Slang pointer types are distinct, but access is not
  part of LLVM pointer-type identity.
  Evidence: `kDirectNVVMCopyScalarSource` and `kDirectNVVMFixedDeviceArraySource` each contain both
  qualifiers while their established fake-provider contract observes one pointer construction.
  Consequence: the context keeps exact `IRType*` source-cache entries and a second representation
  cache keyed only by canonical pointee plus LLVM address space. It does not declare the Slang
  types equivalent.

- Observation: adding the focused source files required no hand-maintained build list.
  Evidence: the Release build reported a CMake glob mismatch, regenerated, and compiled
  `slang-emit-nvvm-type-lowering.cpp` automatically through `source/slang/CMakeLists.txt`.
  Consequence: Slice 29 has no CMake source-list change.

## Decision Log

- Decision: cache provider types by canonical linked `IRType*` and module lifetime.
  Rationale: the linked IR type is the semantic source of truth, provider handles are module-owned,
  and a per-emission context naturally prevents handles escaping their module.
  Date/author: 2026-08-27, Codex.
  Revisit when: canonical equivalent types are observed with distinct IR identities after final
  linking; audit/fix the producer before adding custom structural equivalence.

- Decision: do not create a parallel NVVM semantic type hierarchy.
  Rationale: it would duplicate Slang IR identity and invite equality/layout drift. Named
  classifiers may return the original IR type plus measured metadata, while the cache maps that
  exact type to an opaque provider handle.
  Date/author: 2026-08-27, Codex.
  Revisit when: one canonical Slang type intentionally has multiple ABI representations selected by
  an explicit use context; represent that context in the cache key rather than rebuilding syntax.

- Decision: add no new type capability in this slice.
  Rationale: behavior-preserving migration makes later type failures attributable to their own
  slices and proves that centralization did not widen preflight accidentally.
  Date/author: 2026-08-27, Codex.
  Revisit when: an existing accepted value cannot be expressed without a missing generic provider
  constructor; add only the constructor needed to preserve the baseline and test it explicitly.

- Decision: classify type use before consulting the cache.
  Rationale: a handle cached for one valid use must not accidentally admit that canonical type in a
  forbidden signature position. Entry/helper results and parameters therefore retain the exact
  preflight contract even when they map to an already-created LLVM type.
  Date/author: 2026-08-27, Codex.
  Revisit when: final linked IR intentionally permits one of the currently rejected type/use pairs;
  extend preflight and this classification together with adjacent-negative evidence.

- Decision: share LLVM pointer representations by exact pointee identity and address space, not by
  a structural comparison of Slang pointer types.
  Rationale: read/write access changes legality but not LLVM type identity. The representation key
  keeps one construction without recreating Slang type equality or flattening layout syntax.
  Date/author: 2026-08-27, Codex.
  Revisit when: a future ABI context intentionally maps the same pointee/address-space pair to
  different provider types; add that explicit context to the key.

## Outcomes and Retrospective

Slice 29 is complete. The new internal
`slang-emit-nvvm-type-lowering.{h,cpp}` files own the shared classifiers, use contract, exact
canonical-type cache, and pointee/address-space representation cache. `slang-emit-nvvm.cpp` no
longer contains `_getNVVMI32Type`, `_getNVVMDeviceArrayPointerType`, `i32Type`,
`deviceI32PointerType`, the two array dictionaries, or the raw-resource singleton. Full formatting,
provider/PTX/`ptxas`/runtime, Debug preservation, and final focused-prefix evidence all pass.

The strongest cache observations are one `getIntegerType(32)` across the three-function helper
graph and one pointer construction across read/read-write scalar or array entry parameters. The new
two-compile test observes exactly one void, i32, and pointer construction in each fresh provider
module, proving no handle crosses module lifetime. The complete Release prefix passes 193/193,
including all real differential PTX, `ptxas`, and RTX 5090 runtime lanes. Debug preservation passes
1/1 parser, 2/2 routing/hash, 1/1 unsupported boundary, 3/3 sampler, 2/2 CUDA compile/pass-through,
and 1/1 runtime dispatch. Pinned clang-format 17.0.6 reports no differences and `git diff --check`
is clean.

Future scalar/vector/aggregate slices now extend one use classifier and one representation switch,
add a provider constructor only when V3 lacks the necessary generic family, and supply positive,
adjacent-negative, cache, PTX, `ptxas`, and runtime evidence. They do not add another emitter-owned
type singleton or a parallel semantic type tree.

## Context and Current Pipeline

Before this slice, preflight in `source/slang/slang-emit-nvvm.cpp` classified accepted types with
local signed-i32, array, pointer, and resource helpers. Emission independently reconstructed the
provider graph through five lazy variables/dictionaries and passed `i32Type` through every value
path. That was correct for the bootstrap subset but made each richer type touch every emission
path.

Now `slang-emit-nvvm-type-lowering.{h,cpp}` owns those exact classifiers and provider construction.
Preflight calls the shared policy, while emission passes the original canonical IR type and its
producer/consumer use to one module-local context. No type handle or cache outlives the provider
module, and no alternative syntax is accepted or repaired.

## Scope and Non-Goals

In scope are a private NVVM type-legalization/cache context, central supported-type predicates,
mapping of all established signature/value/pointer/array/resource types, cache identity tests,
removal of redundant `i32`/array/pointer caches, and documentation of use-context invariants.

Out of scope are accepting `uint`, other integer widths, `float` parameters or operations, bool
parameters/storage, vectors, matrices, struct values, conventional global parameter blocks, new
resource elements, new address spaces, layout rewrites, provider ABI version changes beyond a
strictly necessary constructor, and custom type equivalence.

## Architecture and Invariants

The internal `NVVMTypeLoweringContext` lives in focused
`source/slang/slang-emit-nvvm-type-lowering.{h,cpp}` files. It owns references to the codegen
context, selected `NVVMIRBuilder`, live module, and a dictionary from canonical `IRType*` to
module-owned `SlangNVVMTypeHandle_1`. A second dictionary keys LLVM pointer representations by
exact canonical pointee plus address space so access-only Slang type differences do not cause
duplicate provider construction. Its public-to-the-emitter operation is:

```text
lowerType(canonicalIRType, explicitUse, outProviderType)
```

The explicit use distinguishes genuinely different ABI contracts: entry and helper results,
entry and helper parameters, and internal values. It does not accept alternative spellings and is
checked before the cache. Unsupported input returns the existing deterministic diagnostic.
Provider construction failure retains E52018 and does not expose a handle.

Preflight and emission share named classification helpers or a small legalization policy module so
they cannot disagree about accepted shapes. They continue to operate on the original IR types.
Access qualifiers remain legality metadata, not LLVM pointer-type identity. Address-space mapping
is explicit and centralized. Raw resource mapping remains a measured ABI-specific constructor;
the type layer must not flatten it to pointer-plus-count parameters in the host.

## Interfaces and Dependencies

The change adds the internal type-lowering header/source, migrates
`source/slang/slang-emit-nvvm.cpp`, extends focused direct-emitter tests, and updates the design and
ledger. The V3 facade from Slice 28 already provides the required integer/pointer/array and
dedicated raw-resource constructors. No public Slang API, provider ABI, or artifact format changes.
Provider handles remain opaque and module-owned. The owning Slang target's configured source glob
discovered the new files automatically, so no explicit build declaration changed.

## Milestones

1. Inventory every accepted IR type and every current construction/cache site. Add focused fake
   observations proving current provider call counts and exact adjacent-type rejection before
   refactoring.
2. Implement the legalization/cache context for void, signed `int`, accepted device pointers,
   accepted fixed arrays/pointers, and raw `RWStructuredBuffer<int>`. Test same-module reuse and
   different-module isolation.
3. Route function result/parameter declaration through the context. Remove `i32Type`,
   `deviceI32PointerType`, `arrayTypeMap`, `arrayPointerTypeMap`, and
   `rawRWStructuredBufferI32Type` only as their consumers migrate.
4. Route constant, phi, load/store, call/return, addressing, comparison result, atomic, and resource
   emission through centralized type queries or already-typed values. Preserve producer validation
   and all no-mutation boundaries.
5. Perform the self-review inventory: every surviving classifier must describe a valid canonical
   shape and name its producer; no fallback, structural equivalence, syntax reconstruction, or
   arbitrary operand walk may survive.
6. Run focused, preservation, real-provider, PTX, `ptxas`, and runtime evidence; update design and
   ledger with the future extension procedure.

## Self-Review Inventory

- The seven moved classifiers survive. They accept only the exact linked-IR shapes already proved
  by Slices 7, 10, 11, and 26: canonical `Int`/comparison `Bool`; canonical `PtrType` with the
  audited access/address-space/layout operands; canonical nonempty `ArrayType(Int, count)`; exact
  `HLSLRWStructuredBufferType(Int, DefaultBufferLayout)`; and the scalar-layout element pointer
  produced by `RWStructuredBufferGetElementPtr`. They do not walk arbitrary operands, rebuild
  syntax, or accept an alternative spelling.
- `NVVMTypeUse` and the legality check survive. `_validateNVVMFunction` remains the preflight
  producer of the accepted role/type pairs; checking the same role before a cache lookup prevents a
  cached value type from widening a helper or entry ABI. `_reportUnsupportedType` is an invariant
  boundary with the existing E52017 vocabulary, not a fallback that repairs IR.
- `_lowerArrayType` survives. It receives only the already-classified canonical array pointee,
  extracts the exact positive `uint32_t` count checked by preflight, recursively lowers the original
  canonical element type, and caches the provider result under that exact `IRArrayType*`.
- `_lowerPointerType` and `PointerTypeKey` survive. The key compares the original pointee pointer
  and explicit LLVM address space exactly. It does not compare Slang types structurally. The second
  cache exists because access qualifiers intentionally affect legality but not LLVM pointer type;
  the copy and fixed-array tests fail their one-construction assertions without it.
- `lowerType` survives as the single construction boundary. It maps the original linked-IR type
  directly and owns no second AST/IR/type hierarchy. The resource element pointer's canonical
  Generic/ScalarLayout spelling is valid producer output, while its provider representation is AS1
  because the dedicated raw resource stores an AS1 data pointer; the emitter does not patch that
  source shape or flatten the resource.
- No new equivalence relation over AST, IR, `Val`, or witness data; substitution/resolution
  fallback; syntax reconstruction; arbitrary graph walk; hardcoded source name; or silent default
  survives. Removing the context restores the five ad hoc caches and manual `i32Type` threading;
  the cache-count tests identify that producer/consumer break directly.

## Validation and Acceptance

Build the isolated provider if touched and the Release/Debug Slang test targets outside the
sandbox. Run the full focused NVVM prefix and Debug preservation 10/10. Run real direct/NVRTC PTX,
matching-toolkit `ptxas`, and GPU runtime coverage for scalar pointers, fixed arrays, atomic add,
and the raw resource so each established type family crosses the new layer.

Focused fake tests must prove: repeated use of the same canonical type does not reconstruct it;
module boundaries do not share handles; signless provider `i32` represents only the currently
accepted signed Slang `int`; pointer address spaces and access legality remain correct; arrays keep
exact count; the raw resource keeps `{AS1 i32 pointer, i64 count}`; and adjacent unsigned, wide,
floating, read-only-resource, conventional-global, and unsupported layout shapes retain E52017
before provider discovery where currently promised.

Acceptance requires removal of all ad hoc emission-owned type caches superseded by the context,
`git diff --check`, pinned formatting, and no new semantic support claim.

## Failure and Recovery

Migrate one consumer family at a time while the old helpers remain available, then delete the old
path only after its tests use the new owner. If preflight and emission disagree, fix the shared
classification policy or the upstream producer; do not add an emitter-only fallback. If canonical
identity appears unstable, dump final linked IR and trace its type builder before implementing any
equivalence relation.

Do not delete or stage `external/slang-binaries/`. Remove temporary IR dumps and call-count probes
before committing.

## Artifacts and Hand-Off

Retain the accepted-type inventory, producer/consumer traces, cache-call counts, module-isolation
evidence, adjacent-negative matrix, focused/preservation/PTX/`ptxas`/runtime results, and helper
self-review in this plan. Distill the type ownership and future extension contract into the backend
design and capability ledger. Commit this completed plan with Slice 29; leave Slice 30's plan
uncommitted until its implementation is complete.
