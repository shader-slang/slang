---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: ir-reference/differentiation.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 2
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for ir-reference/differentiation.md

## Summary

Two REOPENED gaps were re-decided: gaps recorded as `fixed` that a regenerated
bundle still reports. Both are `fixed` again, this time with the behaviour
stated explicitly in the section the gap anchors to. No gap was escalated; the
source agrees with what the tests observed. The earlier fixes had stated the
conclusion in prose (a parameter `no_diff` yields no `detachDerivative`; hoisted
requests dedupe) without the IR shape a reader could compare against, which is
what the bundle's tests were checking for.

## Actions

| Gap ID       | Action | Evidence                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                                         | Fix summary                                                                                                                                                                                                                                                                               |
| ------------ | ------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| f4b0906929c8 | fixed  | Confirmed in watched sources. `source/slang/slang-check-decl.cpp:7103-7135` (`_moveNoDiffFromTypeToParamDecl`) moves `NoDiffModifierVal` from the parameter type to a `NoDiffModifier` on the `ParamDecl`; `source/slang/slang-lower-to-ir.cpp:3075-3078` lowers `NoDiffModifierVal` to the `NoDiffAttr` that `IRAttributedType` wraps; `source/slang/slang-ir-autodiff.cpp:987` (`stripNoDiffTypeAttribute`) removes it at finalization. The expression forms are at `slang-lower-to-ir.cpp:5326` (`detach`) and `:5341-5400` (`no_diff`: load gets `detachDerivative`, call gets `TreatCallAsDifferentiableDecoration`). The step that re-attaches the modifier to the signature's parameter type is not in the watched paths and is stated as such. | rewrote the closing paragraphs of §`detachDerivative`: the parameter modifier appears as an `Attributed(T, no_diff)` parameter type, not an inst, is stripped by `stripNoDiffTypeAttribute`, and `no_diff` on a call is a decoration; corrected the stale line citations |
| 3a51a0977652 | fixed  | `source/slang/slang-ir-insts.lua:2853-2872` puts `ForwardDifferentiate` under hoistable `TranslateBase`; `source/slang/slang-ir.cpp:2036-2045` routes hoistable ops through `_findOrEmitHoistableInst`, which at `:2880` looks up a module-level global-value-numbering map keyed on the inst (opcode plus operands). So identity is the base function, not call site, function or entry point.                                                                                                                                                                                                                                                  | added the IR line `let %fwd : ... = ForwardDifferentiate(%f)` shared by both `call` insts under §Differentiation operators, and stated that the dedupe is module-wide and keyed on opcode plus operands                                                                                    |
