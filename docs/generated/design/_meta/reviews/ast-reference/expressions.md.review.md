---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:18:48+00:00
target_doc: ast-reference/expressions.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 8e2a1a1b289d2a99a65d0dbd1eee944a77ca8b0eb9415e18a3e6fc23cb52c0bf
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 6
severity_breakdown:
  critical: 0
  major: 1
  minor: 5
  nit: 0
---

# Review report for ast-reference/expressions.md

## Summary

The document has valid front matter, resolving links, and complete concrete-node coverage, but it needs targeted corrections in four checklist dimensions. Most importantly, the `CastOptionalExpr` callout contains detailed IR-lowering behavior that the per-document prompt explicitly forbids on this AST-reference page.

## Items checked

- Read the target, `_common.md`, its per-document prompt, and both dependency documents.
- Verified the document against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`, which was also review-time `HEAD`.
- Compared all 93 table rows with the 93 concrete expression classes in `slang-ast-expr.h`; all nine `FIDDLE(abstract)` intermediates were excluded from the table.
- Spot-checked more than 20 claims covering AST inheritance and fields, parser entry points, keyword dispatch, literals, calls, casts, member access, generic application, lambdas, pack queries, autodiff expressions, and inline SPIR-V.
- Verified all relative links and anchors with the generated-doc linter. The body contains zero line-number citations, so there were no citation line numbers to re-derive.
- Confirmed the recorded watched-path digest matches `regenerate.py digest ast-reference/expressions.md`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | Opening, lines 12-18 | The first paragraph states what the page covers, but the intended reader appears only in a separate `Audience:` paragraph. The universal contract requires both facts in the first paragraph. | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first paragraph to state both coverage and intended reader. | Fold the audience sentence into the opening paragraph so that paragraph names both the `Expr` catalog and its developer audience. |
| F-002 | minor | `## Family hierarchy`, lines 38-80 | The hierarchy omits the `OverloadedExpr` / `OverloadedExpr2` family even though the per-document prompt explicitly requests it among the families shown. Both classes appear later in the table and callout, so only the required hierarchy view is incomplete. | `docs/generated/design/_meta/prompts/ast-reference-expressions.md:14-17`; declarations at `source/slang/slang-ast-expr.h:54-85`. | Add `OverloadedExpr` and `OverloadedExpr2` as sibling leaves under `Expr` in the hierarchy diagram. |
| F-003 | minor | `### VarExpr and DeclRefExpr`, lines 251-258 | The statement `every identifier use starts life as a VarExpr` is too broad. Member identifiers after `.`, `->`, and `::` are parsed directly into `MemberExpr`, `DerefMemberExpr`, or `StaticMemberExpr`, not first represented by a `VarExpr`. | `source/slang/slang-parser.cpp:9172-9204` constructs those member nodes and assigns their member names directly. | Change this to “every bare/unqualified name expression starts as a `VarExpr`,” then explicitly exclude member-name positions. |
| F-004 | minor | `### LiteralExpr family`, lines 260-268 | The prose implies that every literal's inherited `Token` stores its originating token and makes source text recoverable. The keyword-literal callbacks for `true`, `false`, `nullptr`, and `none` create their nodes without assigning `LiteralExpr::token`; keyword dispatch preserves only `loc`. | `source/slang/slang-parser.cpp:1146-1174` consumes the keyword and fills location only; `source/slang/slang-parser.cpp:7986-8008` creates the four keyword-literal nodes without setting `token`. | Qualify the statement: numeric, character, and string literal paths populate `token`; keyword literals retain source location but do not populate the inherited token field. |
| F-005 | minor | `### OverloadedExpr and OverloadedExpr2`, lines 222-231 | The claim `if resolution fails, the checker reports an "ambiguous" diagnostic` conflates two distinct failure modes. Ambiguous best candidates produce an ambiguous-overload diagnostic, while zero applicable candidates produce a no-applicable-overload diagnostic. | `source/slang/slang-check-overload.cpp:3507-3518` diagnoses no applicable overload; `source/slang/slang-check-overload.cpp:3599-3659` diagnoses ambiguity. | Say that checking diagnoses either no applicable candidate or ambiguity, depending on why overload resolution failed. |
| F-006 | major | `### CastOptionalExpr`, lines 322-334 | The final sentences describe the lowering algorithm in detail: binding `innerVarDecl`, lowering `innerCoercedExpr`, and emitting a has-value branch. The per-document prompt explicitly forbids IR lowering of expressions on this page. | `docs/generated/design/_meta/prompts/ast-reference-expressions.md:54-59`; the out-of-scope prose is at target lines 331-334. | Remove the lowering sequence. Keep the AST-field explanation through `innerCoercedExpr`, then link `../pipeline/04-ast-to-ir.md` for lowering behavior. |

## No-issues notes

- Front matter contains every required key, uses a full SHA and hex digest, and reproduces the mandatory warning exactly.
- All required top-level sections appear in the prescribed order, and every grammar/source link resolves.
- Parser claims for expression precedence entry points, member syntax, lambda bodies, adjacent-string concatenation, and `ThisInterfaceExpr` construction match the recorded source.
