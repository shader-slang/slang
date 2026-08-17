---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T13:36:00Z
target_doc: cross-cutting/serialization.md
review_report: ../../reviews/cross-cutting/serialization.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/serialization.md

## Summary

Three of the four findings were verified and fixed: the stale
`## Manifest coverage` section, the missing `slang-ir-insts-stable-names.cpp`
citation, and the incorrect claim that `-load-repro` is deprecated. The
remaining finding asks for a front-matter digest refresh, which the remediation
contract reserves for the operator, so it is recorded as out of scope. Nothing
was rejected as bogus, deferred, or escalated. The per-document prompt contains
a defect that will regress the F-004 fix on the next regeneration; see the note
at the end.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-out-of-scope | `docs/generated/design/_meta/prompts/_remediate.md` lines 97-100 reserve `generated_at`, `source_commit`, and `watched_paths_digest` for the operator's `mark-fresh` run and forbid the remediator from editing them. The digest will be refreshed automatically because this cycle did edit the document. | — |
| F-002 | fixed | Confirmed: `regenerate.py show cross-cutting/serialization.md` resolves `slang-serialize-types.{h,cpp}` and all three `slang-ir-insts-stable-names.{h,cpp,lua}` files, so all four named paths are already watched, and the `.cpp` was omitted from the page's own list. | `## Manifest coverage`: replaced the "outside `watched_paths`" framing and the add-path recommendation with a statement that the cited types, stable-name (including the `.cpp`), and core RIFF files are all covered. |
| F-003 | fixed | Confirmed: `source/slang/slang-ir-insts-stable-names.cpp` declares `kStableNameToOpcode` and defines both `getOpcodeStableName` and `getStableNameOpcode`; the header only declares them. | `## Versioning and backwards compatibility`: added the `.cpp` link and identified it as the owner of both mapping tables. |
| F-004 | fixed | Confirmed against the source of truth the page itself cites: `CLAUDE.md` lists only `-dump-repro` under "AVOID These Debugging Options", and its "Repro Tooling" section states that `-load-repro` and `-extract-repro` are specialized tools to use when working on repro handling. | `## Round-trip and repro files`: split the two cases — `-dump-repro` discouraged, `-load-repro` / `-extract-repro` retained as specialized tools — and kept the existing out-of-scope note about the unwatched implementation. |

## Note for the operator: prompt defect

`docs/generated/design/_meta/prompts/cross-cutting-serialization.md` lines 44-48
instruct the generator to write that "the historical `-dump-repro` /
`-load-repro` machinery is deprecated (per `CLAUDE.md`)". That premise is false
for `-load-repro` and `-extract-repro`, and it is the direct cause of F-004. The
next regeneration will reintroduce the error unless the prompt is corrected to
scope the deprecation to `-dump-repro`. Remediation may not edit prompt files,
so this is reported rather than fixed.
