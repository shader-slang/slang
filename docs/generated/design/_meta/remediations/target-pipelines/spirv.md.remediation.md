---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T14:09:52Z
target_doc: target-pipelines/spirv.md
review_report: ../../reviews/target-pipelines/spirv.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/spirv.md

## Summary
All three findings reported by the review are resolved by edits that are now
present in the target document. F-001 (major) and F-002, F-003 (minor) are each
recorded as `fixed`; none were rejected, deferred, or escalated. The edits add a
Phase D diagram node, correct stale line citations, and supply the missing
intended-reader sentence. Front-matter `source_commit` is unchanged, so
`target_doc_source_commit_after` equals `_before`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The Phase D mermaid diagram was missing the active downstream spirv-opt node and mis-ordered the disabled `optimizeSPIRV` node. The active spirv-opt step is the `compiler->compile` call gated under `if (compiler)` in `source/slang/slang-emit.cpp`, which runs after the link/validate chain; the disabled `#if 0` `optimizeSPIRV` block precedes that setup. | Phase D diagram (doc lines ~700, 720-722): added the `spirvOpt` node labeled "(downstream) compiler->compile spirv-opt" with its connecting edges, and reordered the disabled `optimizeSPIRV` node to sit before the link/validate chain. |
| F-002 | fixed | The Source-anchor citations were stale by 100-plus lines against HEAD: `emitSPIRVFromIR` resolves at `source/slang/slang-emit-spirv.cpp:11803` (doc previously cited ~11598) and `legalizeIRForSPIRV` at `source/slang/slang-ir-spirv-legalize.cpp:3265` (doc previously cited 3104). | Refreshed the affected line citations so `emitSPIRVFromIR` reads ~11803 and `legalizeIRForSPIRV` reads 3265, matching the recorded source commit. |
| F-003 | fixed | `_meta/prompts/_common.md:65` requires the first body paragraph to state the intended reader; the opening paragraph lacked any audience statement. | Intro paragraph (doc lines ~14-17): added a sentence stating the page is for a compiler developer who needs to locate where in the SPIR-V direct-emit pipeline a particular pass runs. |
