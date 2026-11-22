---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:12:00Z
target_doc: target-pipelines/hlsl.md
review_report: ../../reviews/target-pipelines/hlsl.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for target-pipelines/hlsl.md

## Summary

Six findings were verified against current source and fixed; F-002 (diagram gate restructuring) was deferred. The doc sits at its 98304-byte cap, so each addition was paid for by trimming prose elsewhere (embed-downstream-ir note, Phase D bullets, See-also text, loops section); it ends at 98279 bytes.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-legalize-composite-select.cpp:24-47,70-72` confirms result-type test, matrix exclusion, if/else plus merge param | Rewrote the Notable-passes paragraph and Phase B row 20 note; dropped the vector-only justification |
| F-002 | deferred | Valid, but adding gate diamonds and bypass edges to the Phase A-C diagrams plus a consolidated gate table is a broad rewrite that cannot fit within the size cap without removing substantial content; needs a cap raise or a split of the page | — |
| F-003 | fixed | `source/slang/slang-emit.cpp:1693-1705` nests the `missingReturn` test inside `shouldRunNonEssentialValidation()` | Row 26 gate made a conjunction; added `checkForMissingReturns` to the option-set row |
| F-004 | fixed | `source/slang/slang-emit.cpp:673-688` sets `generics` and `existentialTypeLayout` for interface types after pointer unwrapping | Added the fallback to both "Set by" cells |
| F-005 | fixed | `source/slang/slang-code-gen.cpp:1111-1116,1181` and `:538`: dispatch and recursion are outer, source emission nested in `emitWithDownstreamForEntryPoints` | Redrew the Phase D diagram starting at `_emitEntryPoints` dispatch; adjusted notes of rows 8 and 9 |
| F-006 | fixed | `source/slang/slang-ir-explicit-global-init.cpp:133-149` extracts resource-global replacement candidates and preserves `ActualGlobal` | Row 65 now states the extra rules alongside the target policy |
| F-007 | fixed | `source/compiler-core/slang-artifact-util.cpp:45` defines the function | Row 7 File cell relinked; call site kept in Notes |
