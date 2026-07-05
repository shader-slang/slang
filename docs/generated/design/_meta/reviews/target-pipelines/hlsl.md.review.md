---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:34:34+00:00
target_doc: target-pipelines/hlsl.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: d6ab7e839f67ff67089c6ff596134280c2acd4d4480e7715012652269230eb0f
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 1
  minor: 2
  nit: 0
---

# Review report for target-pipelines/hlsl.md

## Summary
The HLSL page largely matches the source ordering for the HLSL source path and the DXIL/DXBytecode downstream transition. The main issue is a Phase B diagram/table mismatch: `checkStaticAssert` appears in the diagram but is missing from the ordered table required by the target-pipeline contract. I also found a small source-section coverage gap and stale Phase D line references.

## Items checked
- Ran `regenerate.py show target-pipelines/hlsl.md` and reviewed the target document, `_common.md`, `target-pipelines-hlsl.md`, and the five dependency documents listed by `depends_on`.
- Checked the target front matter for required keys, the recorded target source commit, warning string, and 64-character hex watched-path digest.
- Spot-checked more than 10 source-backed claims against `slang-emit.cpp`, `slang-code-gen.cpp`, `slang-global-session.cpp`, `slang-emit-c-like.cpp`, `slang-emit-hlsl.cpp`, `slang-emit-hlsl-prelude.cpp`, `slang-ir-hlsl-legalize.cpp`, and `slang-ir-legalize-binary-operator.cpp`.
- Verified the downstream mapping from `DXIL`/`DXBytecode` to `HLSL`, HLSL emitter construction, byte-address-buffer options, HLSL legalization gates, uniform-buffer-load gate, phi elimination, metadata collection, and downstream DXC/fxc descriptions.
- Checked required target-pipeline sections and compared the Phase B diagram/table shape against the target-pipeline contract.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Phase B diagram/table; lines 155-305 | The Phase B diagram contains `cSA[checkStaticAssert]`, but the ordered table has no `checkStaticAssert` row. The target-pipeline contract requires one row per pass node in the diagram, so the diagram and table disagree. | `_common.md` requires a companion ordered table with one row per pass node in the diagram; `source/slang/slang-emit.cpp:1793-1795` shows `checkStaticAssert(irModule->getModuleInst(), sink)` immediately after `specializeArrayParameters`. | Add a `checkStaticAssert` row after `specializeArrayParameters` with file `slang-emit.cpp`, gate `(always)`, and a note that it is a direct call rather than `SLANG_PASS`, or remove the diagram node if direct calls are intentionally out of scope. |
| F-002 | minor | Source and `wrapStructuredBuffersOfMatrices`; lines 31-51, 305, 645-651 | The Source section omits `slang-ir-wrap-structured-buffers.cpp` even though the page treats `wrapStructuredBuffersOfMatrices` as a notable HLSL-only pass. The manifest's resolved watched files also omit that implementation file, so changes to this HLSL-specific pass would not stale the page. | `source/slang/slang-emit.cpp:1797-1808` shows the `case CodeGenTarget::HLSL` call to `wrapStructuredBuffersOfMatrices`; the pass row cites `source/slang/slang-ir-wrap-structured-buffers.cpp`, but the Source section and resolved watched-file set do not include it. | Add `slang-ir-wrap-structured-buffers.cpp` to the Source section and to the HLSL page manifest `watched_paths`. |
| F-003 | minor | Phase D line references; lines 476-521 | A few approximate Phase D line references are stale by more than a few lines. The page cites line ~2616 for `new HLSLSourceEmitter` and line ~2752 for artifact wrapping, while the current source has those at lines 2630 and 2766. | `source/slang/slang-emit.cpp:2628-2630` constructs `HLSLSourceEmitter`; `source/slang/slang-emit.cpp:2766-2767` wraps the emitted text in an artifact. | Refresh the Phase D line references after remediation. |

## No-issues notes
- The downstream mapping from `DXIL`/`DXBytecode` to `HLSL` is supported by `source/slang/slang-code-gen.cpp:263-266` and `source/slang/slang-code-gen.cpp:377-381`.
- The HLSL byte-address-buffer description correctly avoids the prompt's stale `scalarizeVectorLoadStore = true` claim; source only sets `useBitCastFromUInt` for DX 5.0-era profiles.
- The HLSL-specific `legalizeEmptyRayPayloadsForHLSL`, `legalizeNonStructParameterToStructForHLSL`, `legalizeLogicalAndOr`, and `legalizeUniformBufferLoad` gates match `slang-emit.cpp`.
