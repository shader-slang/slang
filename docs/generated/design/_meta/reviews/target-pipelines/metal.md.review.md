---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:16:59+00:00
target_doc: target-pipelines/metal.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 9aaa443e91a52404f1e58afd1c1a28e07ed4703f76a534982a47c4696a8a4462
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 1
  major: 1
  minor: 0
  nit: 0
---

# Review report for target-pipelines/metal.md

## Summary
The Metal page has the required target-pipeline structure, valid generated-document front matter, and strong coverage of the current Metal legalization and emit path. One critical factual contradiction remains: the introduction says `Metal`, `MetalLib`, and `MetalLibAssembly` share the same IR pipeline even though `wrapCBufferElementsForMetal` excludes `MetalLibAssembly`. There is also a Phase B diagram/table contract mismatch around `checkStaticAssert`.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show target-pipelines/metal.md` and used the target front matter source commit and digest in this report.
- Read the target document, `_common.md`, `target-pipelines-metal.md`, and the dependency docs `pipeline/04-ast-to-ir.md`, `pipeline/05-ir-passes.md`, `pipeline/06-emit.md`, `ir-reference/index.md`, and `cross-cutting/targets.md`.
- Checked the required target-pipeline sections, phase table columns, conditional-gate grouping, loop statement, downstream MetalLib handling, and front matter keys.
- Verified more than 15 factual claims against source at the target commit, including `isMetalTarget`, `CodeGenTarget::Metal*` switch arms, ordered `SLANG_PASS` calls in `linkAndOptimizeIR`, `legalizeIRForMetal`, Metal byte-address-buffer options, Metal parameter handling, `wrapCBufferElementsForMetal`, `DescriptorHandle<T>` layout emission, and the Metal downstream transition map.
- Checked the relative links used by the target page and dependency context; no dangling link was found during review.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | Intro, lines 12-20 | The introduction says `All three share the same IR pipeline; they differ only in the downstream tool`, but `linkAndOptimizeIR` does not run the exact same IR pipeline for all three Metal target values. `wrapCBufferElementsForMetal` is selected for `Metal` and `MetalLib` only, while `MetalLibAssembly` falls through to `default`; the page later acknowledges that exception, so the opening summary is both source-contradicted and internally inconsistent. | `source/slang/slang-emit.cpp:1811-1818` lists `CodeGenTarget::Metal` and `CodeGenTarget::MetalLib` before `SLANG_PASS(wrapCBufferElementsForMetal)` and omits `CodeGenTarget::MetalLibAssembly`; `source/slang/slang-emit.cpp:2032-2036` shows a different Metal switch where all three values do share `legalizeIRForMetal`. | Change the intro to say the targets share most Metal legalization, but `MetalLibAssembly` skips `wrapCBufferElementsForMetal` in `linkAndOptimizeIR`; keep the later caveat or move it near the opening target-family summary. |
| F-002 | major | `## Phase B: Specialization and type legalization`, diagram and table | The Phase B diagram contains a `checkStaticAssert` node between `specializeArrayParameters` and `wrapCBufferElementsForMetal`, but the companion ordered table has no row for that node. The target-pipeline contract requires one table row per pass node in a phase diagram, so the diagram and table are not mutually complete. | `docs/generated/design/target-pipelines/metal.md:272-275` draws `cSA[checkStaticAssert]` before `wCBE`; the Phase B table ends with `specializeArrayParameters` and `wrapCBufferElementsForMetal` at `docs/generated/design/target-pipelines/metal.md:420-423`. The source call exists at `source/slang/slang-emit.cpp:1792-1795`, but it is a direct helper call rather than a `SLANG_PASS`. | Either remove `checkStaticAssert` from the Phase B pass diagram, or add a matching ordered table row and label it clearly as a direct helper call rather than a `SLANG_PASS`-wrapped pass. |

## No-issues notes
- The target document front matter contains all required generated-document keys and the watched digest is a valid SHA-256-shaped hex value.
- The Phase C ordering for `legalizeIRForMetal`, `floatNonUniformResourceIndex`, `legalizeLogicalAndOr`, and `legalizeImageSubscript` matches `linkAndOptimizeIR`.
- The loops section is consistent with the Metal path: `linkAndOptimizeIR` has no Metal fixed-point pass loop, and `legalizeIRForMetal` uses ordinary traversal loops rather than an iterative pass loop.
- The `DescriptorHandle<T>` parameter-binding description matches `source/slang/slang-emit-metal.cpp:135-198`, including the resource-type unwrap and system-semantic fallback.
