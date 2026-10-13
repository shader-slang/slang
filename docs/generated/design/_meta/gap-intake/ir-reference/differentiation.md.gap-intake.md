---
gap_intake_report: true
intake_model: claude-opus-5[1m]
intake_at: 2026-08-11T15:10:00Z
target_doc: ir-reference/differentiation.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: ec47ea72b6aa5fefc3b36f8a780dbd3ecf5b1f6e
gap_count: 8
actions:
  fixed: 4
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 3
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/differentiation.md

## Summary

Eight gaps, reported by `design/ir-reference/differentiation` (6) and
`coverage/autodiff` (2). Four were confirmed in the document's watched paths
and fixed: the `no_diff`-as-parameter-modifier distinction, an example of the
hoistable dedupe, the `s_bwdProp_` / `s_bwdCallableCtx_` / `s_fwd_` emitted
names, and the `-experimental-feature` gate on `__func_extension`. One was
rejected as bogus — the Forward-mode table already carries the user surface the
gap says is missing. Three are deferred, and two of those share one cause worth
the operator's attention: the behaviour they ask about is real and I confirmed
it in the compiler, but in files this page does not watch, so documenting it
would create a claim the staleness machinery cannot protect. No gap was
escalated; none of the eight turned out to be a compiler defect.

## Deferred — manifest follow-up

`5ba741954afb` and `0a69b5e6dec9` both need `watched_paths` widened before
they can be fixed:

- `source/slang/slang-check-expr.cpp` — owns `convertHigherOrderExprToLookup`,
  which is the actual answer to "what does `__bwd_diff(f)` become?". The page
  already makes claims about semantic-checking behaviour (§`BackwardDifferentiate`,
  "semantic checking resolves `BackwardDifferentiateExpr` earlier") without
  watching this file, so the gap is pre-existing, not introduced by the gap.
- `source/slang/slang-ir-check-differentiability.cpp` — emits E30510, the
  loop-bound requirement.

Per `regenerate.md` § "Apply manifest changes before recording reviews, not
after", that edit is an operator step and belongs before the next review, not
inside this intake pass.

## Actions

| Gap ID       | Action         | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                                                              | Fix summary                                                                                                                           |
| ------------ | -------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------- |
| f4b0906929c8 | fixed          | Confirmed in watched `source/slang/slang-check-decl.cpp`: lines 6736-6751 move a `NoDiffModifierVal` off a parameter's type onto the `ParamDecl` as a `NoDiffModifier`, lines 2872-2878 do the same for var decls, and `doesTypeHaveNoDiffModifier` (5333, 5338) reads it from either place. No `detachDerivative` inst is involved on that path.                                                                                                                                                     | added a paragraph to §`detachDerivative` distinguishing the expression modifier from the parameter modifier, cross-linked to types.md |
| 3a51a0977652 | fixed          | The claim itself is already in the doc and already tested; the example only had to be accurate. `ForwardDifferentiate` sits in the hoistable `TranslateBase` group at `source/slang/slang-ir-insts.lua:2835` (watched), and `docs/generated/tests/design/ir-reference/differentiation/forward-differentiate-hoisted-dedupe.slang` asserts the shape and is not in expected-failures.                                                                                                                  | added a two-line Slang example plus the resulting single `ForwardDifferentiate` to §Differentiation operators                         |
| 6da449d93530 | fixed          | Confirmed in watched sources: `slang-ir-autodiff-rev.cpp:405,726` generate the `s_bwdProp_` prefix, `:314,727` the `s_bwdCallableCtx_` prefix, and `slang-ir-autodiff-fwd.cpp:2271` builds `s_fwd_<orig>`.                                                                                                                                                                                                                                                                                            | added an emitted-names paragraph after the Reverse-mode table                                                                         |
| 5f9684f655d5 | fixed          | Confirmed in watched `source/slang/slang-check-decl.cpp:16258-16264`. Note the source is broader than the gap claimed: the gate is on `visitFuncExtensionDecl`, so it covers every `__func_extension` form rather than `__apply` alone, and it exempts core-module source via `isFromCoreModule`. Documented what the source says, not the suggestion.                                                                                                                                                | added a gate note after the synthesized-derivative-witnesses table, covering all `__func_extension` forms                             |
| 56dc50d6befd | rejected-bogus | The premise does not hold for the anchored section. Every row of the Forward-mode table already names its user surface: `ForwardDifferentiate` gives `__fwd_diff(...)`, `TrivialForwardDifferentiate` gives `[TreatAsDifferentiable]` / `[HasTrivialForwardDerivative]`, and `ForwardDifferentiatePropagate` correctly states it has no AST origin.                                                                                                                                                   | —                                                                                                                                     |
| 5ba741954afb | deferred       | Behaviour confirmed — `convertHigherOrderExprToLookup` (`source/slang/slang-check-expr.cpp:3965-3979`) rewrites `bwd_diff(fn)` into a lookup of the `fn.bwd_diff` member — but that file is not in this page's `watched_paths`. The gap's suggested answer (a `LegacyBackwardDifferentiate` value) is also narrower than the source: the lookup resolves to whichever synthesized member the callable has. Needs the manifest widened first.                                                          | —                                                                                                                                     |
| 0a69b5e6dec9 | deferred       | E30510 is real: defined at `source/slang/slang-diagnostics.lua:3376-3381` and emitted at `source/slang/slang-ir-check-differentiability.cpp:781`. Neither file is watched by this page. The watched sources corroborate only the passes' dependence on a loop bound (`slang-ir-autodiff-primal-hoist.cpp:2322`, `slang-ir-autodiff-unzip.cpp:553-556`), not the user-facing requirement or its diagnostic. Needs the manifest widened first.                                                          | —                                                                                                                                     |
| cf11c291dff6 | deferred       | Could not confirm the proposed rule. The gap asserts a `DiffPair_*` struct survives into target text only when a differentiated call passes the pair across a function boundary, and that a purely local pair is "scalarized away before emit"; the pass that would do that scalarization is in neither the watched paths nor `slang-ir-autodiff-pairs.cpp`, and no test in the reporting bundle pins the negative case. Documenting it would mean asserting an unverified rule about emitted output. | —                                                                                                                                     |
