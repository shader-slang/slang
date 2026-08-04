---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:58:13Z
target_doc: ast-reference/declarations.md
review_report: ../../reviews/ast-reference/declarations.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/declarations.md

## Summary

All three findings are valid against the reviewed (committed-HEAD) version
of the page and all three are correct against the watched sources. The
target document carries working-tree edits that resolve every finding: the
forbidden IR-lowering sentence is removed (F-001), the parser symbol is
corrected to `ParseDecl` (F-002), and the accessor-synthesis attribution is
moved from the parser to semantic checking (F-003). I verified each fix
against `source/slang/slang-parser.cpp` before recording it. The document is
edited relative to the reviewed version; its front-matter `source_commit`
is already `c21ead2...`, so `_after` equals `_before`. Action breakdown:
3 fixed, 0 rejected-bogus, 0 rejected-out-of-scope, 0 deferred, 0 escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Per-doc prompt lines 59-64 forbid IR-level declaration lowering, and `slang-lower-to-ir.cpp` is not a watched path for this page; the IR-lowering sentence had to go. | `### FuncExtensionDecl`: removed the `IGNORED_CASE` / `slang-lower-to-ir.cpp` sentence so the callout ends at "rest of the pipeline never sees the shorthand." |
| F-002 | fixed | Parser symbol is `ParseDecl` (capital P) per `source/slang/slang-parser.cpp:298`. | `## Source` line 26 and `## See also` line 285: `parseDecl` -> `ParseDecl`. |
| F-003 | fixed | `parseAccessorDecl` / `parseStorageDeclBody` (`source/slang/slang-parser.cpp:4685-4773`) only leave a "treated like `{ get; }`" comment; the implicit `GetterDecl` is materialized later in semantic checking, not by the parser. | `### AccessorDecl family`: reworded to attribute implicit `GetterDecl` synthesis to semantic checking rather than the parser. |
