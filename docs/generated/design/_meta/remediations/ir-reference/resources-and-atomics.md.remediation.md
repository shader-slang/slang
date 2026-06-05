---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: ir-reference/resources-and-atomics.md
review_report: ../../reviews/ir-reference/resources-and-atomics.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 3
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/resources-and-atomics.md

## Summary

All three findings were fixed; none were rejected, deferred, or
escalated. Two missing varying-input opcodes were added to Shader IO,
a required sampling notable callout was added, and target-specific
lowering prose forbidden by the prompt was trimmed in three places and
replaced with a link to the emit page.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-ir-insts.lua:1534,1539` declare `GetPerVertexInputArray` / `ResolveVaryingInputRef` (both hoistable); no sibling page lists them. Semantics verified at `source/slang/slang-ir-resolve-varying-input-ref.cpp:8-31`. | Added `GetPerVertexInputArray` and `ResolveVaryingInputRef` rows to the `### Shader IO` table. |
| F-002 | fixed | `ir-reference-resources-and-atomics.md:59` requires implicit/explicit/gradient sampling notable coverage; `sample` / `sampleGrad` at `source/slang/slang-ir-insts.lua:1508-1509`. | Added a `### sample and sampleGrad` notable callout contrasting implicit-LOD vs explicit-gradient encodings. |
| F-003 | fixed | `ir-reference-resources-and-atomics.md:75-77` forbids target-specific lowering content. | Trimmed SPIR-V/HLSL/Metal lowering detail from the `SubpassLoad` row, the `imageLoad`/`imageStore` notable, and the `ControlBarrier` notable, replacing each with a link to `../pipeline/06-emit.md`. |
