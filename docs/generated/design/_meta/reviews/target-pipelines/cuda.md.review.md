---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:34:34+00:00
target_doc: target-pipelines/cuda.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 14e144c55f95a3a6bcf4a07633067a3feb34968de49ae572e8b9c5be07287d5b
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: fail
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 1
  major: 2
  minor: 1
  nit: 0
---

# Review report for target-pipelines/cuda.md

## Summary
The CUDA page is not ready for remediation as-is because it misdescribes the ordinary `PTX` path. The source maps final `PTX` requests to a `CUDASource` intermediate before NVRTC runs, but the document repeatedly treats `PTX` as if `linkAndOptimizeIR` sees `CodeGenTarget::PTX` directly and therefore takes the default switch arms. I also found one phase table completeness gap, one watched-source coverage gap, and several stale line references.

## Items checked
- Ran `regenerate.py show target-pipelines/cuda.md` and reviewed the target document, `_common.md`, `target-pipelines-cuda.md`, and the five dependency documents listed by `depends_on`.
- Checked the target front matter for required keys, the recorded target source commit, warning string, and 64-character hex watched-path digest.
- Spot-checked more than 10 source-backed claims against `slang-emit.cpp`, `slang-code-gen.cpp`, `slang-global-session.cpp`, `slang-emit-c-like.cpp`, `slang-emit-cuda.cpp`, `slang-ir-cuda-immutable-load.cpp`, `slang-ir-legalize-varying-params.cpp`, `slang-ir-optix-entry-point-uniforms.cpp`, and `slang-ir-pytorch-cpp-binding.cpp`.
- Verified the main CUDA-specific pass gates for OptiX uniform collection, cooperative vectors, CUDA varying-parameter legalization, immutable-buffer loads, and `shouldLegalizeExistentialAndResourceTypes = false`.
- Checked required target-pipeline sections and compared the Phase B diagram/table shape against the target-pipeline contract.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | Intro, Phase A, Phase B, Phase C, and Conditional gates; lines 14-26, 92-109, 153-156, 204-210, 478-494, 685-693 | The page says final `PTX` compilation falls through `linkAndOptimizeIR` default arms and runs `collectEntryPointUniformParams`, `moveEntryPointUniformParamsToGlobalScope`, and `removeTorchAndCUDAEntryPoints`, while skipping some `CUDASource`-only passes. In the ordinary downstream path, final `PTX` requests first map to `CodeGenTarget::CUDASource`, so the source-emission pipeline sees `CUDASource`, not `PTX`. This makes the PTX pass ordering and several PTX-only table rows actively misleading. | `source/slang/slang-code-gen.cpp:261-266` maps `CodeGenTarget::PTX` to `CodeGenTarget::CUDASource`; `source/slang/slang-code-gen.cpp:377-381` chooses that source target before downstream compilation; `source/slang/slang-code-gen.cpp:527-531` emits source from the source target; `source/slang/slang-global-session.cpp:211-213` registers the `CUDASource -> PTX` NVRTC transition. | Rewrite the PTX discussion to say that final `PTX` uses the `CUDASource` IR/source pipeline followed by NVRTC. Remove or clearly mark the current `PTX`-only default-arm rows as not describing the ordinary final-PTX emit path, and update affected claims about `inlineGlobalConstantsForLegalization`, entry-point uniforms, and CUDA varying-parameter legalization. |
| F-002 | major | Phase B diagram/table; lines 215-365 | The Phase B diagram contains `cSA[checkStaticAssert]`, but the companion ordered table has no `checkStaticAssert` row. The target-pipeline contract requires one table row per pass node in the diagram, so readers cannot map that diagram node to its file, gate, or notes. | `_common.md` requires a companion ordered table with one row per pass node in the diagram; `source/slang/slang-emit.cpp:1793-1795` shows `checkStaticAssert(irModule->getModuleInst(), sink)` immediately after `specializeArrayParameters`. | Add a `checkStaticAssert` row after `specializeArrayParameters` with file `slang-emit.cpp`, gate `(always)`, and a note that it is a direct call rather than `SLANG_PASS`, or remove the diagram node if the page intentionally excludes direct calls. |
| F-003 | major | Source and `synthesizeActiveMask`; lines 61-62 and 721-729 | The document makes target-specific claims about `synthesizeActiveMask` and links its implementation, but the resolved watched files from `regenerate.py show target-pipelines/cuda.md` do not include `source/slang/slang-ir-synthesize-active-mask.cpp`. Changes to the pass implementation would not make this generated page stale even though the page describes its behavior. | `source/slang/slang-emit.cpp:1969-1973` shows the CUDA-family `synthesizeActiveMask` call; `source/slang/slang-ir-synthesize-active-mask.cpp` is the implementation file cited by the page but absent from the resolved watched-file set. | Add `source/slang/slang-ir-synthesize-active-mask.cpp` to the CUDA page's manifest `watched_paths` so the digest tracks the source the page summarizes. |
| F-004 | minor | Phase D and `shouldLegalizeExistentialAndResourceTypes` references; lines 80-83, 537-574, 769-811 | Several approximate line references are stale by more than a few lines. For example, the page cites line 2666 for `shouldLegalizeExistentialAndResourceTypes = false`, line ~2621 for `new CUDASourceEmitter`, and line ~2752 for artifact wrapping, but the current source has those at lines 2680, 2635, and 2766 respectively. | `source/slang/slang-emit.cpp:2633-2635` constructs `CUDASourceEmitter`; `source/slang/slang-emit.cpp:2677-2680` sets `shouldLegalizeExistentialAndResourceTypes = false` for CUDA source; `source/slang/slang-emit.cpp:2766-2767` wraps the text artifact. | Refresh the cited line numbers in the Source, Phase D, and notable-pass prose after fixing the PTX path description. |

## No-issues notes
- The non-PTX `CUDASource`/`CUDAHeader` OptiX uniform branch matches `source/slang/slang-emit.cpp:1164-1167`.
- The CUDA immutable-load claim is supported by `source/slang/slang-emit.cpp:2304-2308` and `source/slang/slang-ir-cuda-immutable-load.cpp:292-295`.
- The CUDA emitter construction and source artifact wrapping are correctly described apart from stale line references.
