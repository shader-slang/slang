---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:00:00Z
target_doc: target-pipelines/spirv.md
review_report: ../../reviews/target-pipelines/spirv.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for target-pipelines/spirv.md

## Summary

Five of six findings were verified against the current source and fixed in the document: F-001, F-002, F-003, F-005 (the three cited labels only) and F-006. F-004 is valid but requires restructuring the diagrams and merging five gate tables, so it is deferred. Source commit is unchanged; the watched-paths digest is unchanged.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-eliminate-phis.h:13-14` defaults equal the SPIR-V assignments at `slang-emit.cpp:2788-2794`; the "only backend" and default-mode `OpPhi` claims were wrong. | Rewrote the `eliminatePhis` paragraph to say the assignments restate the defaults; removed the unique-knob and OpPhi comparison. |
| F-002 | fixed | `slang-emit-spirv.cpp:2609-2617` and `12638-12655`: the loop moves recorded `OpTypePointer` definitions to the end of the Types & Constants section; forward declarations are emitted earlier. | Reworded the fixup-loop bullet accordingly. |
| F-003 | fixed | No "Embedding source" heading exists; the section is `### Debug levels and -debug-info-include-source`. | Retargeted the link and label to `#debug-levels-and--debug-info-include-source`. |
| F-004 | deferred | Valid (`_common.md` requires one row per diagram node and one gates table), but the fix means adding many rows across Phases A, B and D and merging five gate tables, a broader rewrite than the minimal-edit rule allows. | — Follow-up: regenerate the doc, or a dedicated restructuring cycle. |
| F-005 | fixed | Predicates verified at `slang-emit.cpp:1099-1101`, `1501`, `1555`. Only the three cited labels were changed; other abbreviated labels remain. | Replaced the three labels with the verbatim C++ predicates. |
| F-006 | fixed | `slang-emit.cpp:1757-1761` sets `reinterpret` after the second scan. | Qualified the safety argument and named `lowerTaggedUnionTypes` as the exception. |
