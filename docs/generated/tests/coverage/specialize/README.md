---
generated: true
model: claude-opus-4-8[1m]
generated_at: 2026-06-11T14:00:00+00:00
source_commit: ef1068b5485e09b3a7afadba2e25f9541e29af42
watched_paths_digest: be9d6bdc63cfae3185a1605cf375e75f1caf240e9ddcf13318b656167b5b7905
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for coverage/specialize

## Intent

White-box characterization tests for the generic/existential specialization
passes `source/slang/slang-ir-typeflow-specialize.cpp` (~41% covered) and
`source/slang/slang-ir-specialize.cpp` (~34% covered). These pin the
**current observed behaviour** of CLI-reachable specialization and
dynamic-dispatch branches, not a spec.

Strategy: the two passes funnel a family of distinct interface / existential /
generic mistakes into late (IR-pass / target-codegen-stage) diagnostics that
front-end checking does not catch. Each error path is pinned with a
`DIAGNOSTIC_TEST` (which validates locally without FileCheck). Positive
specialization behaviour that resolves before back-end codegen
(associated-type-constrained generics, generic value parameters) is pinned
with `INTERPRET` (validates locally on `slangi`). The clean
multi-conformance dynamic-dispatch lowering shape is pinned with a CUDA
`SIMPLE` emit (FileCheck — ignored locally, validated in CI).

All diagnostics, codes, and values were copied verbatim from the local
`slangc` / `slangi`. The error-path diagnostics fire late (after lowering),
so they are pinned on the `-target cuda` back-end where the compiler prints
the full located span message; on some other targets the same passes print
only the short E-code form (sometimes repeated). No test carries
`characterization-unverified=true`: every pinned diagnostic / value is a
deterministic, reproduced output.

The second pass over this bundle (2026-08-05) re-measured the baseline
locally instead of trusting the shipped report, because the shipped report
under-counts: profiling `slangc` directly showed several regions the
hand-written suite does exercise. The working baseline was the union of the
nightly profile, the whole hand-written `tests/` suite and the generated
trees (999 uncovered lines in `slang-ir-specialize.cpp`, 947 in
`slang-ir-typeflow-specialize.cpp`), so every test added in that pass covers
code nothing in the repository covers today. That pass added three tests
worth 67 previously-uncovered lines, and — more usefully — established that
most of what remains is dead code, host-API-only, or defensive; those
findings are in `## Unreachable gaps` below.

## Functional coverage

| Test                                                                                                             | What it pins (current behaviour)                                                                                                                                                                                                                                                                                                                                                   | covers=                                       |
| ---------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------- |
| [`assoc-type-constrained-generic-spec.slang`](assoc-type-constrained-generic-spec.slang)                         | A generic constrained by an associated-type equality (`where C.Element == int`) specializes and calls the conformer's method, returning its value.                                                                                                                                                                                                                                 | source/slang/slang-ir-typeflow-specialize.cpp |
| [`bitcast-to-interface-type-diag.slang`](bitcast-to-interface-type-diag.slang)                                   | `bit_cast<IFoo>(x)` to an interface (existential) target type is rejected with E41204 "cannot bit-cast to existential (non-concrete) type".                                                                                                                                                                                                                                        | source/slang/slang-ir-typeflow-specialize.cpp |
| [`dynamic-dispatch-two-conformers-emit.slang`](dynamic-dispatch-two-conformers-emit.slang)                       | An interface value assigned one of two concrete conformers on different paths lowers cleanly to CUDA dynamic-dispatch code (a `switch` over the runtime tag).                                                                                                                                                                                                                      | source/slang/slang-ir-typeflow-specialize.cpp |
| [`generic-value-param-spec.slang`](generic-value-param-spec.slang)                                               | A generic value parameter (`let N : int`) specialized to `3` unrolls and sums the first three array elements (1+2+3 = 6).                                                                                                                                                                                                                                                          | source/slang/slang-ir-typeflow-specialize.cpp |
| [`global-type-param-shader-parameter-diag.slang`](global-type-param-shader-parameter-diag.slang)                 | A `type_param` used as the element type of a global shader parameter (`ConstantBuffer<TParam> cb;`) passes front-end checking and is rejected by the IR pass with E38207, reported once at the `type_param` declaration span even though the constrained parameter also lowers to a synthesized witness-table param.                                                               | source/slang/slang-ir-specialize.cpp          |
| [`shape-pack-late-transform-fold.slang`](shape-pack-late-transform-fold.slang)                                   | Symbolic `__shapeConcat` / `__shapePermute` / `__shapeSwap` / `__shapeReduce` that survive front-end folding are folded during IR specialization: concat adds the axis dims (16,32)+(16,8)@1 -> (16,40), permute (4,8,16) by (2,0,1) -> (16,4,8), swap 0/2 of (4,8,16) -> (16,8,4) with the middle axis copied through, same-axis swap is the identity, reduce sets the axis to 1. | source/slang/slang-ir-specialize.cpp          |
| [`structured-buffer-interface-no-conformance-diag.slang`](structured-buffer-interface-no-conformance-diag.slang) | A `lookup_witness_method` left unresolved in an entry point because no type conformance for the interface reached the linkage (`StructuredBuffer<IFoo>` with the conformer never referenced) is reported as E50100 at the `.get()` call site, naming the interface.                                                                                                                | source/slang/slang-ir-typeflow-specialize.cpp |
| [`ref-interface-param-dynamic-dispatch-diag.slang`](ref-interface-param-dynamic-dispatch-diag.slang)             | A `__ref` parameter of interface type called in a dynamic-dispatch context is rejected with E52010.                                                                                                                                                                                                                                                                                | source/slang/slang-ir-typeflow-specialize.cpp |
| [`specialize-generic-with-existential-diag.slang`](specialize-generic-with-existential-diag.slang)               | Explicitly specializing a generic with an interface type argument (`gen<IFoo>(f)`) is rejected with E33180.                                                                                                                                                                                                                                                                        | source/slang/slang-ir-specialize.cpp          |
| [`uninitialized-existential-dispatch-diag.slang`](uninitialized-existential-dispatch-diag.slang)                 | Dynamic dispatch on an interface value that is left uninitialized on one control-flow path is rejected with E50101.                                                                                                                                                                                                                                                                | source/slang/slang-ir-typeflow-specialize.cpp |

## Unreachable gaps

- The many `SLANG_UNEXPECTED(...)` sites in both passes (e.g. "Unhandled
  PropagationJudgment", "Unexpected witness table info type", "Invalid
  context for InstWithContext", "Unhandled interprocedural edge direction")
  assert on internal lattice / context invariants that valid front-end-checked
  input cannot reach; they are defensive and not targeted.
- `DynamicDispatchOnSpecializeOnlyInterface` (E52008) fires only when an
  interface carries the internal `[specialize]`-only decoration
  (`IRSpecializeDecoration`) yet a call still needs dynamic dispatch. No
  user-facing surface to attach that decoration to an interface from a single
  `.slang` source file was found, so this path is not given a test.
- An interface-typed local initialized with defaults (`IFoo f = {};`, only
  warned via E30521, not rejected) and then dynamically dispatched on aborts
  during SPIR-V emit with an internal error E99997. This is a crash on accepted
  input, so per the methodology it is filed as a finding
  (`_meta/findings/specialize-interface-defaults-init-spirv-emit-ice.yaml`,
  repro under `_repro/`) rather than pinned as a passing test.
- The duplicate emission of E50100 / E50101 on SPIR-V/HLSL/GLSL/Metal/WGSL/CPP
  targets (the same diagnostic printed 2–3 times with no source location)
  versus the single located form on CUDA is recorded as a doc/behaviour gap
  below rather than pinned, because the repeat count is target-incidental.

### Dead code — candidates for removal, not for tests

Verified by grepping the whole `source/` tree for each symbol: these have no
caller reachable from any entry point, so no input can cover them. They are
the single largest block of uncovered lines in `slang-ir-specialize.cpp`
(~430 of the file's remaining uncovered lines) and should be deleted rather
than tested.

| Symbol                                                                                                                                                                                                | File                                          | Why it is dead                                                                                                                                                                                                                                                                                                                                         |
| ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ |
| `createExistentialSpecializedFunc` (294 lines), `canSpecializeExistentialArg`, `isSimplifiedExistentialArg`, `isExistentialType`, `isExistentialReturnTypeSpecializable`, `isCompileTimeConstantType` | source/slang/slang-ir-specialize.cpp          | `maybeSpecializeExistentialsForCall` no longer calls any of them — it now only forwards to `maybeSpecializeBufferLoadCall` / `tryExpandParameterPack` and returns `false`. Existential specialization moved to the type-flow pass. `isCompileTimeConstantType` is called only from the other dead members.                                             |
| `addInstsToWorkListRec`                                                                                                                                                                               | source/slang/slang-ir-specialize.cpp          | Only self-recursive; no external caller.                                                                                                                                                                                                                                                                                                               |
| `analyzePtrType`                                                                                                                                                                                      | source/slang/slang-ir-typeflow-specialize.cpp | Its only dispatch site is commented out (`// case kIROp_PtrType:`).                                                                                                                                                                                                                                                                                    |
| `analyzeTupleType`, `isSingletonInfo`, `isTaggedUnionType`, `initializeFirstBlockParameters`, single-argument `getParamInfos(IRInst*)`                                                                | source/slang/slang-ir-typeflow-specialize.cpp | Defined but never referenced (the two-argument `getParamInfos(IRInst*, IRFuncType*)` is the one in use).                                                                                                                                                                                                                                               |
| `analyzeSwizzleSet`                                                                                                                                                                                   | source/slang/slang-ir-typeflow-specialize.cpp | Dispatched from `kIROp_SwizzleSet`, but its very first statement (`as<IRTupleType>(base->getDataType())`) is never executed across the entire suite: the only producer of `IRSwizzleSet` is vector swizzle-assignment lowering in `slang-lower-to-ir.cpp`, whose base is always a vector, and no `IRSwizzleSet` survives to the type-flow pass at all. |

No finding YAML was written for these: the finding schema requires an
`evidence.command` plus `evidence.source_slang` that reproduces a
misbehaviour, and unreachable code has neither.

### Reachable only through the host API, not the CLI

- The existential-slot cluster in `slang-ir-specialize.cpp`
  (`maybeSpecializeBindExistentialsType`, `calcExistentialBoxSlotCount`,
  `calcExistentialTypeParamSlotCount`, `maybeSpecializeFieldExtract`,
  `maybeSpecializeGetElement`, `maybeSpecializeBufferLoadCall`,
  `getNewSpecializedBufferLoadCallee`) consumes `IRBindGlobalExistentialSlots`,
  which `slang-ir-bind-existentials.cpp` only produces from link-time
  existential specialization arguments supplied by the host
  (`IComponentType::specialize`, or `slang-test`'s `TEST_INPUT: type` /
  `global_type` render-test directives). `slangc -specialize <typename>` binds
  entry-point specialization arguments only and reports E38025 "wrong number of
  specialization arguments" for a global `type_param`, so no single-file CLI
  invocation reaches these. Covering them needs a unit test against the public
  API or a GPU render test, not a `.slang` file.
- The successful-binding half of `specializeGlobalGenericParameters` has the
  same shape: this bundle's new test covers the _unbound_ reporting loop, but
  the bound path needs a host-supplied global type argument.

### Reachable in principle, no source construction found

- `ShapeConcatNoValidAxis`, the rank-0 `ShapePackNoValidAxis` variants for
  `__shapeConcat` / `__shapeSwap` / `__shapeReduce`, and
  `ShapePermuteDuplicateEquivalentIndex` in `maybeSpecializeShapePackTransformInst`
  all require a _non-constant_ axis (or non-constant order elements) at the
  moment the shape packs are already concrete. Every construction tried either
  keeps the packs symbolic too (so the pass folds nothing) or lets the front end
  constant-fold the axis first and report the equivalent early diagnostic
  (`UserPermuteSym<X>` with `permute<X, X>` reports E30423 at check time, not
  the late `ShapePermuteDuplicateEquivalentIndex`). Recorded rather than tested.
- `diagnoseGenericSpecializationCycle` needs re-entrant specialization of the
  _same_ `(generic, args)` key while that key is still in flight.
  Mutually-recursive generics with identical arguments memoize instead, and a
  growing argument list hits `diagnoseGenericSpecializationBudgetExceeded`
  (already covered by `tests/language-feature/generics/recursive-generic-specialization-budget.slang`)
  rather than the cycle path.
- `tryExpandParameterPack` in the type-flow pass never observes a parameter
  whose type is an `IRTypePack` (152k calls in the suite, zero hits): packs are
  already expanded before type-flow runs. It is not obviously dead — the guard
  is a legitimate defensive check — but no input reaches the body.
- The 27 non-`Constexpr*` case labels of `isUnsimplifiedArithmeticInst` are
  fall-through labels in one big switch; only the label actually matched is
  counted, so "covering" them is line-chasing with no behavioural claim
  attached. Deliberately not targeted.

## Doc gaps observed

| Anchor                                                                                                                                              | Kind                  | Gap                                                                                                                                                                                                                                                                                                                                                                                                                  | Suggested addition                                                                                                                                                                                                                                |
| --------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#dispatchers-and-existential-specialization](../../../design/ir-reference/generics-and-existentials.md#dispatchers-and-existential-specialization) | undocumented-behavior | When an entry point takes a `uniform` interface-typed parameter and the linkage has no conforming type, the compiler emits E50100 "no type conformances found", but on SPIR-V/HLSL/GLSL/Metal/WGSL/CPP it prints the short form with no source location and repeated 2–3 times, while CUDA prints a single located span. The doc does not describe the no-conformances requirement or this per-target inconsistency. | Add a note that interface-typed entry-point parameters require at least one conforming type in the linkage, document the E50100 diagnostic, and treat the no-location / repeated emission on non-CUDA targets as a diagnostic-quality bug to fix. |
| [#dispatchers-and-existential-specialization](../../../design/ir-reference/generics-and-existentials.md#dispatchers-and-existential-specialization) | undocumented-behavior | A potentially-uninitialized interface object used in dynamic dispatch reports E50101 twice on CUDA. The doc does not state that an interface local must be definitely-assigned on all paths before a dynamic-dispatch call, nor that the diagnostic may repeat.                                                                                                                                                      | Document the definite-assignment requirement for interface objects before dynamic dispatch and de-duplicate the repeated E50101 emission.                                                                                                         |
