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

## Functional coverage

| Test                                                                                                 | What it pins (current behaviour)                                                                                                                              | covers=                                       |
| ---------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------- |
| [`assoc-type-constrained-generic-spec.slang`](assoc-type-constrained-generic-spec.slang)             | A generic constrained by an associated-type equality (`where C.Element == int`) specializes and calls the conformer's method, returning its value.            | source/slang/slang-ir-typeflow-specialize.cpp |
| [`bitcast-to-interface-type-diag.slang`](bitcast-to-interface-type-diag.slang)                       | `bit_cast<IFoo>(x)` to an interface (existential) target type is rejected with E41204 "cannot bit-cast to existential (non-concrete) type".                   | source/slang/slang-ir-typeflow-specialize.cpp |
| [`dynamic-dispatch-two-conformers-emit.slang`](dynamic-dispatch-two-conformers-emit.slang)           | An interface value assigned one of two concrete conformers on different paths lowers cleanly to CUDA dynamic-dispatch code (a `switch` over the runtime tag). | source/slang/slang-ir-typeflow-specialize.cpp |
| [`generic-value-param-spec.slang`](generic-value-param-spec.slang)                                   | A generic value parameter (`let N : int`) specialized to `3` unrolls and sums the first three array elements (1+2+3 = 6).                                     | source/slang/slang-ir-typeflow-specialize.cpp |
| [`ref-interface-param-dynamic-dispatch-diag.slang`](ref-interface-param-dynamic-dispatch-diag.slang) | A `__ref` parameter of interface type called in a dynamic-dispatch context is rejected with E52010.                                                           | source/slang/slang-ir-typeflow-specialize.cpp |
| [`specialize-generic-with-existential-diag.slang`](specialize-generic-with-existential-diag.slang)   | Explicitly specializing a generic with an interface type argument (`gen<IFoo>(f)`) is rejected with E33180.                                                   | source/slang/slang-ir-specialize.cpp          |
| [`uninitialized-existential-dispatch-diag.slang`](uninitialized-existential-dispatch-diag.slang)     | Dynamic dispatch on an interface value that is left uninitialized on one control-flow path is rejected with E50101.                                           | source/slang/slang-ir-typeflow-specialize.cpp |

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

## Doc gaps observed

| Anchor                                                                                                                                              | Kind                  | Gap                                                                                                                                                                                                                                                                                                                                                                                                                  | Suggested addition                                                                                                                                                                                                                                |
| --------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#dispatchers-and-existential-specialization](../../../design/ir-reference/generics-and-existentials.md#dispatchers-and-existential-specialization) | undocumented-behavior | When an entry point takes a `uniform` interface-typed parameter and the linkage has no conforming type, the compiler emits E50100 "no type conformances found", but on SPIR-V/HLSL/GLSL/Metal/WGSL/CPP it prints the short form with no source location and repeated 2–3 times, while CUDA prints a single located span. The doc does not describe the no-conformances requirement or this per-target inconsistency. | Add a note that interface-typed entry-point parameters require at least one conforming type in the linkage, document the E50100 diagnostic, and treat the no-location / repeated emission on non-CUDA targets as a diagnostic-quality bug to fix. |
| [#dispatchers-and-existential-specialization](../../../design/ir-reference/generics-and-existentials.md#dispatchers-and-existential-specialization) | undocumented-behavior | A potentially-uninitialized interface object used in dynamic dispatch reports E50101 twice on CUDA. The doc does not state that an interface local must be definitely-assigned on all paths before a dynamic-dispatch call, nor that the diagnostic may repeat.                                                                                                                                                      | Document the definite-assignment requirement for interface objects before dynamic dispatch and de-duplicate the repeated E50101 emission.                                                                                                         |
