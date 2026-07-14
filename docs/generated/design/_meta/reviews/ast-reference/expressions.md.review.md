---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:26:21+00:00
target_doc: ast-reference/expressions.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: bb183348b2c07246bd0706641ccbb5c4867597747399350a43dc2a5e89db9fdd
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 1
  minor: 2
  nit: 0
---

# Review report for ast-reference/expressions.md

## Summary

The document has the required AST-reference structure, valid front matter, resolving generated-doc links, and a complete `## Nodes` table for the concrete `Expr` classes. The main issue is that one notable-node explanation assigns `PartiallyAppliedGenericExpr` to parser two-stage parsing, but the source creates that node during overload checking after a `GenericAppExpr` has already been parsed.

## Items checked

- Read the target document, `_common.md`, the per-doc prompt, and the dependency docs `ast-reference/base.md` and `syntax-reference/grammar.md`.
- Verified `source_commit` against `git rev-parse HEAD`; it is `c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8`.
- Compared the `## Nodes` table against `slang-ast-expr.h`; the table covers all 92 concrete expression classes, with abstract intermediates excluded.
- Spot-checked claims for `DeclRefExpr`, `LiteralExpr`, adjacent string literals, `BuiltinOperatorExpr`, `InvokeExpr`, `IsTypeExpr`, `AsTypeExpr`, `SizeOfLikeExpr`, pack-query expressions, member expressions, generic application, `PartiallyAppliedGenericExpr`, lambda parsing, higher-order expressions, and `SPIRVAsmExpr`.
- Resolved the relative generated-doc links used by the target document, including peer AST pages, syntax-reference pages, pipeline pages, and `cross-cutting/ir-instructions.md`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `### PartiallyAppliedGenericExpr`, lines 243-251 | The prose says `When the parser commits to "generic application" but only some parameters are explicitly supplied, the result is a PartiallyAppliedGenericExpr`. The parser's generic-application path creates `GenericAppExpr`; `PartiallyAppliedGenericExpr` is produced later by overload checking when a generic candidate is flagged as partially applied. | `source/slang/slang-parser.cpp:2868` creates `GenericAppExpr` in `parseGenericApp`; `source/slang/slang-check-overload.cpp:1778`-`1797` creates `PartiallyAppliedGenericExpr` for `OverloadCandidate::Flag::IsPartiallyAppliedGeneric`. | Rewrite the callout to say the parser builds `GenericAppExpr` for `<...>` syntax, and overload resolution/checking rewrites the partially supplied generic candidate into `PartiallyAppliedGenericExpr` while preserving the provided ordinary argument prefix. |
| F-002 | minor | `### MemberExpr / StaticMemberExpr / DerefMemberExpr`, lines 217-222 | The paragraph says the checker chooses between the three member-expression nodes and that `the parser emits MemberExpr by default`. For surface syntax, the parser already emits `StaticMemberExpr` for `::`, `MemberExpr` for `.`, and `DerefMemberExpr` for `->`; checker logic may add further static/member forms, but it is not the sole chooser. | `source/slang/slang-parser.cpp:9381`-`9397` creates `StaticMemberExpr` for scope access, and `source/slang/slang-parser.cpp:9401`-`9421` selects `MemberExpr` or `DerefMemberExpr` based on `.` versus `->`. | Split the explanation into parser behavior and checker behavior: describe the three parser-produced forms first, then note that checking can synthesize or reinterpret member lookup for type-valued bases. |
| F-003 | minor | `### Differentiate-family expressions`, lines 253-260 | The callout groups `DispatchKernelExpr` with autodiff expressions and says they are `entry points for the autodiff machinery`. `DispatchKernelExpr` is a higher-order expression, but the header describes it as host-side compute-kernel dispatch, not an autodiff entry point. | `source/slang/slang-ast-expr.h:842`-`850` declares `DispatchKernelExpr` as `__dispatch_kernel(fn, threadGroupSize, dispatchSize)` to dispatch a compute kernel from host. | Remove `DispatchKernelExpr` from the autodiff callout or give it a separate sentence as a higher-order host kernel dispatch expression rather than an autodiff expression. |

## No-issues notes

- The target front matter contains all required generated-doc keys and the requested `source_commit` / `watched_paths_digest` values.
- The required `## Source`, `## Family hierarchy`, `## Nodes`, `## Notable nodes`, and `## See also` sections are present in order.
- The literal-family description matches the header and parser behavior, including `StringLiteralExpr` concatenation and `CharLiteral` mapping to `IntegerLiteralExpr`.
- The `BuiltinOperatorExpr` description is well aligned with the header, checker, lowering, and constant-folding source comments.
