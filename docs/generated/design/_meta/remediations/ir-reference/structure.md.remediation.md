---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T08:33:38Z
target_doc: ir-reference/structure.md
review_report: ../../reviews/ir-reference/structure.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 4
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/structure.md

## Summary

All four findings were verified against the current source and fixed in the document. Four findings were fixed; none were rejected, deferred, or escalated.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-lower-to-ir.cpp:1849-1855` keys by declaration with a `getMangledName` linkage name; same spelling does not imply same key. | `### key / StructKey`: unification is by same declaration's mangled name; same-spelling declarations stay distinct. |
| F-002 | fixed | Producers confirmed at `slang-lower-to-ir.cpp` lines 15548, 10922, 12204, 12215, 2433, 11006, 12530, 12675, 12704, 14047. | Replaced all `(synthesized)` cells with named producers; added lowering functions for `param`, `field`, `SymbolAlias`. |
| F-003 | fixed | 19 of 23 tabulated wrappers are hand-written, per the Source section. | Added a dagger marker on 19 wrapper cells plus a legend stating the count under `## Opcodes`. |
| F-004 | fixed | `slang-lower-to-ir.cpp:1432-1458` and `slang-ir-link.cpp:2552-2574` show import/export decorations sit on individual symbols. | `### module / ModuleInst`: decorations described as on contained symbols; serialization kept separate. |
