---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T21:04:45Z
target_doc: target-pipelines/hlsl.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 0b7fe7ec288dbc7e2a2ca1dd7403506557b25cefbb5c003b9e9b844c353f39c6
source_commit: f40a5af1027e6fd524a5855f77d62bddffd998c9
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 7
severity_breakdown:
  critical: 2
  major: 1
  minor: 4
  nit: 0
---

# Review report for target-pipelines/hlsl.md

## Summary

The document substantially matches the current pipeline, but requires corrections before acceptance. Most importantly, it misdescribes composite-select legalization: the implementation tests the result type, preserves matrices, and introduces control flow rather than element-wise selects. The downstream diagram also reverses the nesting of dispatch and source emission, while several diagrams omit required conditional paths.

## Items checked

- Verified against the **current working tree**, which the review input identifies as matching the document’s source commit throughout `source/`; no files were written.
- Read `_review.md`, `_common.md`, the document-specific prompt, and `regenerate.py show target-pipelines/hlsl.md`.
- Audited the body’s explicit line-number citations, including 292 `line`/`lines` references with repeated citations and ranges, against their identified source files.
- Checked more than 20 behavioral claims, including lowering-flag accumulation, specialization order, coverage options, byte-address options, payload wrappers and qualifiers, matrix storage, uniform-buffer loads, logical operators, phi defaults, scope correction, NVAPI handling, named constants, and downstream dispatch.
- Resolved all 199 Markdown link occurrences; all 10 referenced generated peer pages exist in the manifest. Consulted dependency documents only to check cross-reference descriptions.
- Checked required sections, table columns, diagram structure, front-matter keys and formats, and Liquid safety. The document occupies 98,252 bytes against its 98,304-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Notable passes` → `legalizeNonVectorCompositeSelect`, lines 953–960; Phase B row 20 | The claim that the pass rewrites selects whose **condition** is a matrix or struct into `element-wise selects` contradicts the implementation. Selection is based on the **result type**, matrices are explicitly excluded, and the rewrite produces branches and a merge parameter. | `source/slang/slang-ir-legalize-composite-select.cpp:70–72` excludes basic, vector, and matrix result types. Lines 24–31 emit if/else blocks and branches; line 47 emits the merge parameter. | Describe legalization of remaining composite **results**, such as structs or arrays, into if/else control flow with a merge block parameter. Explicitly distinguish the broader scan flag from the pass’s matrix exclusion. Remove the vector-only justification from row 20 unless appropriately qualified. |
| F-002 | major | Phase A–C diagrams; `## Conditional gates` | The required control-flow representation is incomplete. Phase A explicitly omits gates; B and C also draw many gated calls as unconditional. Phase B lacks the path where minimal optimization is enabled and `reqSet.generics` is false, collapses five checks into one node, and includes two skipped passes as pipeline nodes. The gate consolidation is split across several tables rather than the required single grouped table. | `docs/generated/design/_meta/prompts/_common.md`, Target-pipeline contract items 5–6; `source/slang/slang-emit.cpp:1087–1104`, `1501–1532`, `1693–1710`, `1740–1747`, and `2738–2743` show representative missing branches. | Add gate diamonds and bypass edges for each conditional call; add the neither-simplification-nor-DCE path at 1740–1747; separate the validation checks into individual nodes; remove skipped-pass nodes while retaining explanatory prose. Consolidate gates into one grouped table and add the required validation-call summaries beneath the diagrams. |
| F-003 | minor | Phase B row 26; `## Conditional gates` | `checkForMissingReturns` is shown as gated only by `reqSet.missingReturn`. Its enclosing `shouldRunNonEssentialValidation()` condition is missing, including from that option’s controlled-pass list. | `source/slang/slang-emit.cpp:1693–1705`: the `requiredLoweringPassSet.missingReturn` test is nested inside the nonessential-validation block. | Make the row’s gate the conjunction of both conditions and add `checkForMissingReturns` to the nonessential-validation option row. |
| F-004 | minor | `## Conditional gates` → `requiredLoweringPassSet.*`, `generics` and `existentialTypeLayout` rows | Both “Set by” cells omit the interface-type fallback. A flag can therefore become true without any of the listed triggering opcodes, contrary to the table’s apparent enumeration. | `source/slang/slang-emit.cpp:673–688` examines an instruction’s type or data type, unwraps pointer types, and sets both flags when the resulting opcode is `kIROp_InterfaceType`. | Add this shared trigger to both cells: an interface type, or an instruction whose data type becomes an interface type after pointer unwrapping. |
| F-005 | critical | Phase D diagram, lines 619–644; ordered table rows 8–10 | The graph places target dispatch, assembly recursion, and `emitWithDownstreamForEntryPoints` **after** HLSL text generation. In the implementation these are outer calls that initiate source generation. Following the depicted order implies an extra source-emission pass before assembly recursion. | `source/slang/slang-code-gen.cpp:1111–1116` recurses to the binary target first; lines 1176–1181 dispatch binary targets to `emitWithDownstreamForEntryPoints`; that function emits source at line 538 and invokes the downstream compiler at line 1029. | Start the graph with `_emitEntryPoints` dispatch. Show assembly requests recursing to their binary target, then binary requests entering `emitWithDownstreamForEntryPoints`, which generates HLSL before calling DXC/fxc. Place disassembly after binary compilation and reorder the table consistently. |
| F-006 | minor | Phase B row 65 | The statement that HLSL keeps declaration initializers except for arrays and `CoopVector` types omits the resource-global replacement rule. The cited target-policy helper is only one part of the extraction decision. | `source/slang/slang-ir-explicit-global-init.cpp:133–149`: `shouldExtractInitializer` preserves `IRActualGlobalRate`, but otherwise extracts resource-global replacement candidates before consulting `isInitializerRequiredByTarget`. | Qualify the array/`CoopVector` rule as the target-policy portion. State that resource globals selected for per-invocation replacement also have initializers extracted, while actual-global storage is preserved. |
| F-007 | minor | Phase D table row 7, File column | `createArtifactForCompileTarget` links to its call site in `slang-emit.cpp`, although the required File column identifies the implementation file. | `source/compiler-core/slang-artifact-util.cpp:45` defines `ArtifactUtil::createArtifactForCompileTarget`; `source/slang/slang-emit.cpp:3197` only calls it. | Link the File cell to `source/compiler-core/slang-artifact-util.cpp`; retain `slang-emit.cpp:3197` as the call-site citation in Notes. |