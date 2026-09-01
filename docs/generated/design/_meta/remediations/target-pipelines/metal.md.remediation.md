---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T13:20:00Z
target_doc: target-pipelines/metal.md
review_report: ../../reviews/target-pipelines/metal.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for target-pipelines/metal.md

## Summary

All seven findings were re-verified against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`. Six were fixed and one (F-001, the stale `watched_paths_digest`) was rejected as out of scope because the remediation contract reserves that front-matter field for the operator. The document was edited. Adding `addUserTypeHintDecorations` and splitting three combined rows grew the Phase B table from 70 to 74 rows, so it was renumbered and the three in-page references to Phase B row numbers were updated to match.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-out-of-scope | The mismatch is real, but `docs/generated/design/_meta/prompts/_remediate.md:97-100` states that `generated_at`, `source_commit`, and `watched_paths_digest` are refreshed by the operator running `regenerate.py mark-fresh` and that the remediator must not edit them. Since this remediation edits the page, `mark-fresh` will record the correct digest; no action is available to me here. | — |
| F-002 | fixed | Confirmed: `source/slang/slang-emit.cpp:1774-1777` gates `SLANG_PASS(addUserTypeHintDecorations)` only on `CompilerOptionName::VulkanEmitReflection`, with no target predicate, so it is reachable on a Metal compile; `_common.md:364-368` requires every reachable call to appear in a phase table. The pass is defined in `source/slang/slang-ir-user-type-hint.cpp`. | Phase B: added a `getBoolOption VulkanEmitReflection` diamond and an `addUserTypeHintDecorations` node after `lowerCombinedTextureSamplers`, added the matching ordered row (now row 54), and deleted the paragraph that explained the omission. The option row in the consolidated gate table was kept. |
| F-003 | fixed | Confirmed all three: `finalizeAutoDiffPass` / `stripAutoDiffDecorations` are separate `SLANG_PASS` calls at `slang-emit.cpp:1448` and `:1452`; the direct `eliminateDeadCode(irModule, ...)` at `:1654` sits between `checkGetStringHashInsts` and `lowerTuples` and is drawn as `dce4`; and `:1938-1941` are two alternative calls drawn as separate nodes. `_common.md:329-338` requires one table row per pass node. | Phase B: row 9 split into `finalizeAutoDiffPass` + `stripAutoDiffDecorations`; new `eliminateDeadCode` row added after `checkGetStringHashInsts` (noted as a direct call, not `SLANG_PASS`); the combined `simplifyIR` / `eliminateDeadCode` row split in two; table renumbered 1-74 and the row cross-references updated (29->30, 54-59->57-62, 30->31). |
| F-004 | fixed | Confirmed: `source/slang/slang-emit.cpp:2463-2479` selects `BufferElementTypeLoweringPolicyKind::Metal` at lines 2470-2472 and then issues `SLANG_PASS(lowerBufferElementTypeToStorageType, ...)` unconditionally at lines 2476-2479. A gate expression in the Gate column implies the pass itself is conditional. | Phase C row 22: Gate cell changed to `(always)` (line 2476); the `isMetalTarget` / `Metal` policy selection moved into the Notes cell. |
| F-005 | fixed | Confirmed: `source/slang/slang-ir-insts.lua:1379-1382` declares `metalSetVertex`, `metalSetPrimitive`, `metalSetIndices`, and `MetalCastToDepthTexture`; line 1679 declares `MetalAtomicCast`. That is five, not four, and the subsection's own producer analysis already treats `MetalAtomicCast` as the unproduced fifth. | `### Where the Metal-specific opcodes come from`: opening sentence now says five Metal-only value/resource opcodes, cites both Lua ranges, and states that only four have a producer at this commit. |
| F-006 | fixed | Confirmed: the intro ran across two paragraphs and named no reader; `_common.md:65-66` requires the first body paragraph to state coverage and intended reader, and `prompts/target-pipelines-metal.md:12-14` defines that audience. | Intro: added the compiler-developer audience clause from the per-document prompt and joined the `05-ir-passes.md` paragraph into the same opening paragraph. |
| F-007 | fixed | Confirmed: `docs/user-guide/a2-02-metal-target-specific.md` exists, and `_common.md:355-362` requires the See-also list to link the user-facing target documentation when one exists. | `## See also`: added a bullet linking `../../../user-guide/a2-02-metal-target-specific.md`. |

## Note on the documented builder bug

The subsection stating that `IRBuilder::emitMetalSetPrimitive` and `emitMetalSetIndices` both pass `kIROp_MetalSetVertex` was left untouched. It is a verified, still-latent source defect, not a documentation error.
