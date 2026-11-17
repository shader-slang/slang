---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:03:16Z
target_doc: ast-reference/modifiers.md
review_report: ../../reviews/ast-reference/modifiers.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 7
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ast-reference/modifiers.md

## Summary

All seven findings were verified against the current source and fixed in the document. The 26 per-topic tables were merged into one `## Nodes` table to pay down the size cap, and long semantic-checking prose was cut, leaving the page at about 65.1 KB, under the 65,536-byte cap.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/hlsl.meta.slang:15939` has target-switch bodies for `sin`; `source/slang/core.meta.slang:1117` has `__intrinsic_op(select) T select(...)`. | Replaced the `sin` example with `select`, and noted `sin` is not opcode-bound. |
| F-002 | fixed | `source/slang/slang-ast-modifier.h:463` is an empty class; `source/slang/slang-parser.cpp:1101` and `:10874` create and diagnose it. | `GLSLUnparsedLayoutModifier` summary now says it is a diagnosed marker. |
| F-003 | fixed | `core.meta.slang:489,498,517` declare the marker attributes; `diff.meta.slang:44,83` the derivative bindings. | `## Source` and the work-graph section now split the family across both files. |
| F-004 | fixed | Keyword registrations at `slang-parser.cpp:11155-11245` and `attribute_syntax` declarations confirm these are parsed. | Relinked 137 plus about 25 parsed rows; `(none)` kept only for synthesized or unspelled nodes. |
| F-005 | fixed | `_common.md:133-134,158-161` require one table of concrete classes; `slang-ast-modifier.h:1457` marks the base abstract. | Merged the tables into one; dropped the abstract row (still in the hierarchy). |
| F-006 | fixed | `_common.md:153-156` requires the parsing page in `See also`. | Added `../pipeline/02-parse-ast.md` and `../pipeline/03-semantic-check.md`. |
| F-007 | fixed | The prompt excludes modifier semantics; `_common.md:142` requires short clauses with only inline-code markup. | Trimmed about 20 summaries, removed bold and links, and condensed two checking sections to a pointer. |
