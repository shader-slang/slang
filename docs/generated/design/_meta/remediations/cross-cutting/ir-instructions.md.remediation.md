---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: cross-cutting/ir-instructions.md
review_report: ../../reviews/cross-cutting/ir-instructions.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/ir-instructions.md

## Summary

The review reported two major findings, both fixed. F-001: the required top-level `## Decorations` section was missing; a short, conservative section was added after `## Hoistable / global / deduplicated values`, grounded in the `Decoration` family of `slang-ir-insts.lua` and `IRDecoration` in `slang-ir.h`, without over-claiming a migration roadmap. F-002: four summary-table cells used display names that are not Lua opcode entries; they were corrected to the real entry names.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Prompt `cross-cutting-ir-instructions.md` lines 56-60 require a top-level `## Decorations` section after the hoistable section; only a `### Decorations` subtable existed. | Added a `## Decorations` section citing the `Decoration` family in `slang-ir-insts.lua` and `IRDecoration`/`getFirstDecoration` in `slang-ir.h`; links to `ir-reference/decorations.md`. |
| F-002 | fixed | `slang-ir-insts.lua` declares `Vec`/`Mat` (lines 103,110) and `unconditionalBranch`/`conditionalBranch` (lines 1330,1347), not `vector`/`matrix`/`branch`/`condBranch`. | Type table: `vector`->`Vec`, `matrix`->`Mat`. Control-flow table: `branch`->`unconditionalBranch`, `condBranch`->`conditionalBranch`. |
