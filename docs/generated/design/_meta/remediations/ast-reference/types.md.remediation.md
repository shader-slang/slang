---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T14:05:00Z
target_doc: ast-reference/types.md
review_report: ../../reviews/ast-reference/types.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/types.md

## Summary

Seven findings were fixed and one was rejected as out of scope. The
stale "parser is not watched" sentence was deleted, the
`BorrowInParamType` row was corrected (it is the `borrow` /
`__constref` mode, not `in`), the `ThisType` row now uses `(none)` as
the contract requires, the `FuncType` callout now explains
parameter-mode wrapper types and the always-present error slot, the
resource callout now cites the `__magic_type` intrinsics, the missing
parsing-pipeline cross-reference was added, and the emit/layout pass
detail in the descriptor-heap callout was reduced to a cross-reference.
F-001 concerns the front-matter digest, which belongs to the operator.

## Actions

| Finding ID | Action                | Rationale                                                                                                                                                                                                                                                                                                                                                                                            | Fix summary                                                                                                                                                                                                                                                                            |
| ---------- | --------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | rejected-out-of-scope | `docs/generated/design/_meta/prompts/_remediate.md:97-100` reserves `watched_paths_digest` for the operator's `regenerate.py mark-fresh` run and forbids the remediator from editing it. The document was edited under other findings, so the operator's `mark-fresh` will record the current digest.                                                                                                | —                                                                                                                                                                                                                                                                                      |
| F-002      | fixed                 | Confirmed: `regenerate.py show ast-reference/types.md` resolves `source/slang/slang-parser.cpp`, so the claim that it is unwatched and the recommendation to add it were both stale.                                                                                                                                                                                                                 | `## Source`: dropped the trailing "That file is not among this page's watched paths either..." sentence, keeping the parser entry-point list                                                                                                                                           |
| F-003      | fixed                 | Confirmed: `source/slang/slang-ast-type.h:890-895` documents `BorrowInParamType` as "immutable borrow input parameter, e.g., `borrow T`" and explicitly says it is an input-only equivalent of `inout`, not comparable to `ref`. `source/slang/slang-parser.cpp:10750` maps the surface spelling `__constref` to `BorrowModifier`, so there is no public grammar production for it.                  | `## Nodes`: `BorrowInParamType` grammar cell `[in param](...)` -> `(none)`; summary now describes the immutable borrow input parameter, its `__constref` spelling, and its relation to `inout`                                                                                         |
| F-004      | fixed                 | The per-page contract at `docs/generated/design/_meta/prompts/ast-reference-types.md:34-36` names `ThisType` among the checking-only types that must use `(none)` with a Summary note; the row linked a `This` grammar anchor instead.                                                                                                                                                               | `## Nodes`: `ThisType` grammar cell -> `(none)`; summary now says checking synthesizes it for the self type of an interface or extension                                                                                                                                               |
| F-005      | fixed                 | Confirmed: `docs/generated/design/_meta/prompts/_common.md:119-122` requires the parsing page for a parsed family, and the list had no `pipeline/` entry.                                                                                                                                                                                                                                            | `## See also`: added a `../pipeline/02-parse-ast.md` bullet referring to the parser entry points already named in `## Source`                                                                                                                                                          |
| F-006      | fixed                 | Confirmed against `source/slang/slang-ast-type.h:994-1007` (parameter operands carry mode wrappers such as `OutParam<int>`) and `:1064-1077` (a non-failing function stores the bottom type `Never` as its error type, so the slot is never absent). The old callout called the error type "optional" and said nothing about qualifiers.                                                             | `### FuncType`: first sentence replaced by an operand-order sentence plus two sentences on parameter-mode wrapper types and the always-present error slot                                                                                                                              |
| F-007      | fixed                 | `docs/generated/design/_meta/prompts/ast-reference-types.md:57-59` requires the group note to cite the relevant core-module intrinsic definitions, and it cited none. The bindings were re-verified at HEAD in `source/slang/hlsl.meta.slang` and `source/slang/core.meta.slang`; neither file is in this page's `watched_paths`, so they are linked without line numbers.                           | `### Resource and texture type families`: added a sentence naming the `__magic_type(TextureType)` / `__magic_type(HLSLByteAddressBufferType)` intrinsics in `hlsl.meta.slang` and the `ConstantBufferType` / `ParameterBlockType` / `SamplerStateType` intrinsics in `core.meta.slang` |
| F-008      | fixed                 | The second cited range really did carry pass/layout/emit detail (a named IR pass plus internal-error behavior), which `docs/generated/design/_meta/prompts/_common.md:131-135` excludes from AST pages. The first cited range is a different matter: its only IR sentence is already the short pointer to `cross-cutting/ir-instructions.md` that the recommendation asks for, so it was left alone. | `### UntypedResourceHandleType and UntypedSamplerHandleType`: final sentence replaced with a one-clause statement that IR lowering reduces the handle to its `uint` index, linking `../cross-cutting/ir-instructions.md`                                                               |
