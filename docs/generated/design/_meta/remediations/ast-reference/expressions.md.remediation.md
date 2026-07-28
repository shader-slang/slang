---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:58:11Z
target_doc: ast-reference/expressions.md
review_report: ../../reviews/ast-reference/expressions.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/expressions.md

## Summary

Acted on 3 findings, all fixed by minimal body-prose edits to the three
`## Notable nodes` callouts the review flagged. F-001 reattributes
`PartiallyAppliedGenericExpr` to overload checking; F-002 lists the three
parser-produced member-expression forms before noting checker
reinterpretation; F-003 separates `DispatchKernelExpr` from the autodiff
group as a host kernel-dispatch primitive. Each edit matches the
reviewer's recommendation and was verified against the watched source. No
watched source changed, so `source_commit` is unchanged across the edits.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Callout framed `PartiallyAppliedGenericExpr` as a parser two-stage-parsing result; the node is created in overload checking (`source/slang/slang-check-overload.cpp:1785`, gated on `IsPartiallyAppliedGeneric`), while the parser builds `GenericAppExpr` (`parseGenericApp`, `source/slang/slang-parser.cpp:2868`). | Rewrote callout lead to attribute the node to overload checking and cite `parseGenericApp` for the `GenericAppExpr` the parser actually builds. |
| F-002 | fixed | Callout said `the parser emits MemberExpr by default and the checker may convert it`; the parser emits all three forms directly — `StaticMemberExpr` for `::`, `MemberExpr` for `.`, `DerefMemberExpr` for `->` (`source/slang/slang-parser.cpp:9384,9407,9408`). | Replaced the "MemberExpr by default" sentence with the three parser-produced forms, then noted checker synthesis/reinterpretation for type-valued bases. |
| F-003 | fixed | Callout grouped `DispatchKernelExpr` with the differentiate expressions as autodiff entry points; the header declares it as a host `__dispatch_kernel(fn, threadGroupSize, dispatchSize)` primitive (`source/slang/slang-ast-expr.h:842-851`). | Split `DispatchKernelExpr` out of the autodiff sentence as a `HigherOrderInvokeExpr` host compute-kernel dispatch primitive unrelated to autodiff. |
