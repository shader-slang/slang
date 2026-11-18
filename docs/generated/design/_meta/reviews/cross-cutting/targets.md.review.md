---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T19:58:29Z
target_doc: cross-cutting/targets.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: 524db519639c68c87df847957eafdee39c8abdb2e708167900770ba6290809b5
source_commit: f40a5af1027e6fd524a5855f77d62bddffd998c9
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 1
  major: 1
  minor: 2
  nit: 0
---

# Review report for cross-cutting/targets.md

## Summary

The capability and profile explanations largely match the source, but the document needs corrections before acceptance. Most importantly, the target table describes `SLANG_C_SOURCE` as producing C/C++ through the C++ emitter, although the source-emitter dispatcher rejects it. The new-target checklist also omits required routing work.

## Items checked

- Reviewed against the **current working tree**; `git diff` confirmed no differences in `source/` from the document’s source commit.
- Read the review contract, common generation contract, document prompt, and resolved watched paths.
- Spot-checked more than 20 claims, including public target enumeration coverage, `SourceLanguage`, profile packing and family derivation, node-stage capabilities, SPIR-V defaults, cross-family profiles, explicit-capability version checks, unknown-name diagnostics, capability aliases, subgroup restrictions, abort and ray-query declarations, LLVM dispatch, the Slang-emission stub, and prelude registration.
- Checked target/stage specialization implementations and new-backend dispatch requirements.
- Resolved all 84 Markdown link occurrences: 57 distinct destinations, including the local profile anchor. All eight linked peer pages have manifest entries. Dependency documents were consulted only for cross-reference checks.
- Found zero source line-number citations in the body; none required verification.
- Checked required sections, target-table coverage, front-matter keys and formats, source filenames, Liquid safety, and document size: 28,606 UTF-8 bytes.
- No files were written and no compilation tests were run.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Targets`, line 39 | The C++ shader row includes `SLANG_C_SOURCE` and promises `C/C++ text` from `slang-emit-cpp.cpp`. For ordinary Slang-to-target emission, this enum does not select the C++ emitter: it maps to `SourceLanguage::C`, for which the emitter dispatcher has no case. | `source/slang/slang-emit-c-like.cpp:77` returns `SourceLanguage::C`. `source/slang/slang-emit.cpp:3045–3088` constructs an emitter for `CPP`, but not `C`, then diagnoses `UnableToGenerateCodeForTarget` when no emitter was selected. | Separate `SLANG_C_SOURCE` from the working C++ targets and label it as a declared enum without an implemented source emitter on this path. Remove the promise that `slang-emit-cpp.cpp` produces C output. |
| F-002 | major | `## Adding a new target`, lines 464–478 | The purported `full checklist` covers the public enum and source-emitter dispatch, but omits the internal target mapping and outer code-generation dispatcher. Following it leaves a new target unable to reach its emitter. It also presents the textual dispatcher as sufficient for non-textual backends. | `source/slang/slang-target.h:24` defines the separate `CodeGenTarget` mapping. `source/slang/slang-code-gen.cpp:1233–1303` explicitly routes targets and otherwise reaches `SLANG_UNEXPECTED("unhandled code generation target")`. The linked dependency `docs/generated/design/pipeline/06-emit.md:480–491` distinguishes textual and direct backend registration. | Extend the existing checklist to include `CodeGenTarget`, source-language mapping where needed, and `CodeGenContext::emitEntryPoints`. Explain that direct backends also require routing through `_emitEntryPoints`; link the dependency’s existing checklist for details. |
| F-003 | minor | `## How target choice affects IR`, lines 393–400 | The paragraph says both specialization passes resolve `[target]` / `[stage]` paths against `TargetRequest` and identifies their syntax as `__target_switch`. Stage specialization instead handles stage-dependent IR using each referencing entry point’s stage. | `source/slang/slang-parser.cpp:7322` recognizes `__stage_switch`; `source/slang/slang-lower-to-ir.cpp:8948–8959` lowers it using `GetCurrentStage`. `source/slang/slang-ir-specialize-stage-switch.cpp:124–168` takes only an IR module and clones functions using entry-point stages. | Distinguish `__target_switch`, specialized using target capabilities, from `__stage_switch`, lowered through `GetCurrentStage` and specialized per entry-point stage. Remove the shared `TargetRequest` description. |
| F-004 | minor | Whole document | At 28,606 bytes, the page exceeds its own prompt’s explicit `Document length under 24 KB` requirement, although it fits the manifest’s larger cap. | `docs/generated/design/_meta/prompts/cross-cutting-targets.md:63`; measured UTF-8 size of `docs/generated/design/cross-cutting/targets.md`. | Shorten the page below 24 KB while preserving required sections; condense extended examples and repeated explanations in the capability/profile discussion. |