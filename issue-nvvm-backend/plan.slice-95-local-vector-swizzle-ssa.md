# Promote local vector swizzle updates to SSA

This ExecPlan follows `.agent/PLANS.md`. Keep it current as work proceeds. The user explicitly
requires each completed slice plan to be committed with its implementation, overriding the normal
working-log policy for this experimental backend.

## Purpose and Observable Result

After this slice, copyable local vectors whose only address-like operation is a canonical
`swizzledStore` are promoted to ordinary SSA values. The direct NVVM backend consumes the resulting
pure `swizzleSet` through its existing generic vector extraction/construction operations, with no
stack-allocation API and no Half-specific callback. The existing
`tests/compute/half-vector-calc.slang` fixture should pass a direct CUDA runtime lane and direct PTX
checking instead of stopping at `var`.

## Progress

- [x] (2026-08-29) Reproduced the post-Slice-94 boundary at both O0 and O3: one local Half4 remains
  as `var` solely because a partial `.xyz` assignment becomes `swizzledStore`.
- [x] (2026-08-29) Traced the producer: `constructSSA` deliberately rejects every partial store,
  even though final optimized IR has already forwarded every full store/load around this one use.
- [x] (2026-08-29) Identified the existing canonical value operation, `swizzleSet`, which represents
  exactly "copy the base vector and replace these lanes" without memory or pointer semantics.
- [x] (2026-08-29) Taught the generic SSA constructor to promote direct local-vector
  `swizzledStore` uses into
  canonical `swizzleSet` values while preserving CFG/phi behavior.
- [x] (2026-08-29) Flattened accepted constant-lane `swizzleSet` values through the direct
  backend's existing
  generic vector extraction/construction contract, without changing the builder ABI.
- [x] (2026-08-29) Added focused fake-boundary and existing file-backed runtime/PTX coverage,
  including negative
  shape/no-mutation evidence where appropriate.
- [x] (2026-08-29) Formatted, built, ran focused/full/CUDA validation, assembled PTX,
  self-reviewed, updated durable
  docs and this plan, and prepared the complete slice for commit.

## Surprises and Discoveries

- The local Half4 is not evidence that direct NVVM needs stack storage. After ordinary
  optimization, every full store/load has already been forwarded; only `v2.xyz = -v2.zwx` prevents
  promotion. Treating this as an alloca request would preserve a representation accident instead
  of fixing the producer's explicitly documented partial-assignment gap.
- `IRSwizzleSet` already has the exact pure-value semantics needed by SSA promotion and is emitted
  by other legalization passes. The direct backend can flatten it using callbacks it already has,
  so this slice need not advance ABI revision 10.
- `half-vector-compare.slang` is independent: it requires a scalar-struct helper result and a
  `BorrowInOutParam<Values>` stateful helper ABI. It should remain the measured next boundary rather
  than being mixed into this local-vector change.
- A plain `slangc` probe of the promoted shader succeeds at both its default level and O3, but the
  compute-test-harness form with `-render-features half -shaderobj` is rejected by CUDA 12.9
  libNVVM at its default level and succeeds at O3. The registered direct lanes record O3 rather
  than generalizing that module-specific result.
- Running the whole existing file prefix also exposed a pre-existing CUDA-source boundary: CUDA
  12.9 NVRTC rejects generated `__half4.xyz`. The Vulkan lane and both new direct lanes pass; this
  unrelated old CUDA lane was neither disabled nor patched as part of direct NVVM.

## Decision Log

- Decision: extend the generic `constructSSA` pass at the producer boundary instead of adding
  direct-NVVM local allocation.
  Rationale: the variable is copyable, has a known whole-vector value at every update, and does not
  escape. The existing pass comment names partial assignments as the missing promotion case;
  representing this update as a value restores the intended SSA invariant for every consumer.
  Date/author: 2026-08-29, Codex.
- Decision: use canonical `IRSwizzleSet` as the promoted value.
  Rationale: it is the established IR source of truth for a vector with selected lanes replaced.
  Rebuilding lane semantics in the NVVM emitter or introducing a target-only IR spelling would
  duplicate this representation.
  Date/author: 2026-08-29, Codex.
- Decision: flatten `swizzleSet` to vector extraction plus construction at the direct-emitter
  boundary.
  Rationale: the provider already exposes exact generic vector extraction/construction. A new
  callback or operation enum would add surface area for a composition the current interface can
  express economically.
  Date/author: 2026-08-29, Codex.

## Outcomes and Retrospective

The generic SSA constructor now promotes a non-escaping vector local whose partial assignments are
direct `swizzledStore` operations. It reads the complete value already tracked for the variable,
creates canonical `swizzleSet`, and records that value through the existing `writeVar` path. The
normal block sealing and phi machinery remains the sole CFG implementation. A dynamic
element-address store stays unpromoted and reaches the established E52017 `var` diagnostic before
builder discovery.

Direct NVVM classifies constant-lane `swizzleSet` as one complete selected-vector construction.
It validates result/base/source types, source lane count, unique bounded destinations, dominance,
and availability, then composes the provider's existing vector extractions and construction.
Builder ABI remains revision 10; the facade, LLVM provider, and fake interface gain no alloca or
swizzle callback.

The motivating final linked IR has one Half4 `swizzleSet(%old, %replacement, 0, 1, 2)` and no
Half4 pointer, load, store, or `swizzledStore`. The focused fake test records at least two generic
vector constructions and seven element extractions. The existing shader's new direct runtime lane
produces `75, 220.5, 565, 1108`; its 3,495-byte PTX contains native f16/f16x2 arithmetic, and
`ptxas -arch=sm_70` emits a 3,688-byte cubin. The next direct probe remains E52017
`helper function result type` in `half-vector-compare.slang`.

## Context and Current Pipeline

Consider the retained source assignment:

    half4 v2 = half4(...);
    v2 = +v2.yxwz;
    v2.xyz = -v2.zwx;

The first whole-vector assignment is forwarded as a value. The second becomes
`swizzledStore(%v2, %negated, 0, 1, 2)`. `constructSSA::isPromotableVar` accepts only full stores
and address chains ending in loads, so `%v2` remains a pointer even at O3. Direct preflight then
correctly rejects `var` before the provider is mutated.

`swizzleSet(base, source, 0, 1, 2)` is the canonical value-form equivalent. Promotion can read the
current SSA value exactly as a load would, build that pure update, and record it with `writeVar`.
The normal phi construction algorithm then handles branches and loops without any target-specific
logic.

## Scope and Non-Goals

In scope:

- direct `swizzledStore` uses of copyable local vector variables;
- scalar or same-element vector replacement sources and canonical lane operands;
- SSA/phi preservation through the existing `readVar`/`writeVar` algorithm;
- selected two- through four-lane direct-NVVM `swizzleSet` flattening;
- the complete existing Half vector calculation as runtime/PTX evidence.

Out of scope:

- escaping variables, pointer/reference helper parameters, local allocas, volatile or noncopyable
  values, address-chain partial stores, matrices, arrays, structs, dynamic l-value indexing, or
  switch-fallthrough variables;
- scalar-struct helper results or `BorrowInOutParam<Values>` from `half-vector-compare.slang`;
- a new builder callback, operation enum, or ABI revision.

## Architecture and Invariants

- Promotion applies only when the variable itself is the `swizzledStore` destination and every
  other use already satisfies the existing promotability contract. An escaping pointer remains
  memory.
- The current SSA value is read before the replacement value is constructed. Each promoted update
  therefore observes exactly the preceding full/partial assignments on that CFG path.
- `swizzleSet` owns replacement ordering and lane operands. Direct lowering validates exact result,
  base, source, element, and bounded constant-lane types before provider mutation.
- The direct provider sees only ordinary scalar/vector values. It does not learn about Slang local
  variables, l-values, swizzles, or Half-specific syntax.

## Interfaces and Dependencies

Update `source/slang/slang-ir-ssa.cpp` to classify and process canonical local-vector
`IRSwizzledStore`. Extend the existing direct vector-construction resolver and its preflight,
availability, and emission switches to accept `kIROp_SwizzleSet` by composing the existing provider
operations. Add focused fake instrumentation and registered shader directives. Builder ABI remains
revision 10.

Validation uses the configured Release host build, standalone LLVM provider, CUDA 12.9, and
`ptxas -arch=sm_70`. Builds and tests run outside the sandbox per repository instructions.

## Milestones

1. Promote direct local-vector swizzled stores through the generic SSA algorithm and verify the
   final linked IR contains no local `var`, load, store, or `swizzledStore` for the motivating
   value.
2. Admit canonical selected-vector `swizzleSet` in direct preflight and flatten its complete result
   lanes through generic extraction/construction.
3. Add focused fake coverage that records exact source/base lane transport and proves dynamically
   addressed local stores remain conservatively rejected before provider discovery.
4. Register the existing Half vector calculation for direct runtime/PTX, compile and assemble the
   resulting PTX, and re-probe the next suite boundary.
5. Format, build, run the complete NVVM prefix and changed shader prefix, perform the
   input-shape/special-case audit, update durable documents and this plan, and commit.

## Validation and Acceptance

Acceptance requires a final-IR probe proving producer-side variable elimination; focused fake
descriptor/value traces; the complete `slang-unit-test-tool/nvvm` prefix; direct runtime and PTX
lanes for `half-vector-calc.slang`; CUDA 12.9 PTX assembly; pinned clang-format 17; and
`git diff --check`.

Completed evidence on 2026-08-29:

- Release `slangc`, `slang-unit-test`, and `slang-test` host targets built successfully; the
  standalone `slang-llvm-nvvm` Release provider also built successfully.
- `slang-unit-test-tool/nvvm`: 367/367 passed with the standalone real provider configured.
- `nvvmSlangLocalVectorSwizzlePromotesToGenericValues` and
  `nvvmSlangUnsupportedIRStopsBeforeEmission`: 1/1 each.
- `tests/compute/half-vector-calc.slang.1`, `.3`, and `.4`: the unaffected Vulkan lane and both new
  direct lanes each passed 1/1. The pre-existing `.2` NVRTC lane remains the independently
  documented `__half4.xyz` failure.
- Final linked IR contains the expected value-form `swizzleSet` and no local Half4 pointer or
  `swizzledStore`.
- The final PTX is 3,495 bytes, and CUDA 12.9 `ptxas -arch=sm_70` produced a 3,688-byte cubin.
- Pinned clang-format 17 completed on every changed C++/header file, and `git diff --check` reported
  no errors.

## Self-Review and Input-Shape Audit

No new production helper or fallback was introduced. The one new producer case handles an exact
canonical shape: `slang-lower-to-ir.cpp` creates `IRSwizzledStore` for a source l-value swizzle,
the destination use is the local vector variable itself, and every other use must already satisfy
`isPromotableVar`. The shape is intentionally valid IR, and the existing `IRSwizzleSet` is its
semantic value-form source of truth. Removing this case reproduces E52017 `var` in the motivating
shader, which proves SSA construction owns the fix.

The direct resolver's `IRSwizzleSet` branch is the selected physical-value consumer. It accepts
only exact two- through four-lane values and lowers them by composing already-negotiated generic
vector operations. It neither reconstructs syntax nor searches operands for a hidden base. The
dynamic local-element negative test proves that the change does not infer value semantics for an
address chain. No checked semantic value is duplicated, no malformed IR is patched, and no silent
default, alternate equivalence relation, alloca fallback, or target-specific producer rewrite
remains in the diff.

## Failure and Recovery

If canonical `swizzleSet` survives in a shape the selected direct vector contract cannot express,
record that exact shape and keep the variable rejected rather than introducing an alloca as a
shortcut. Generated dumps/PTX/cubins stay under ignored `build/`. Never reset unrelated work or
stage `external/slang-binaries/`.

## Artifacts and Hand-Off

Record the before/after final IR, focused fake counts, runtime output, PTX/cubin sizes, full/focused
test counts, the next exact fixture stop, and the self-review inventory here. Distill the generic
SSA and direct vector policy into `docs/design/nvvm-backend.md` and durable evidence into the
capability ledger.
