---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T20:08:48Z
target_doc: target-pipelines/wgsl.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: f1572ca0c903d9ebfc5c7b40ec8b0599a62a30152d7b1ee7d6797a9c04c5fd74
source_commit: f40a5af1027e6fd524a5855f77d62bddffd998c9
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 1
  minor: 3
  nit: 0
---

# Review report for target-pipelines/wgsl.md

## Summary

The document accurately captures the main WGSL pass ordering, legalization behavior, and Tint/glslang downstream routing. It needs corrections before acceptance: both lowering-requirement scans appear in diagrams but lack companion table rows, and three pass-definition links point to unrelated implementation files. Two behavioral descriptions also need qualification.

## Items checked

- Read `_review.md`, `_common.md`, and the target prompt; ran `regenerate.py show target-pipelines/wgsl.md`.
- Verified against the **current working tree**, which the operator identifies as matching the document’s source commit throughout `source/`.
- Checked the 141 phase-table rows against orchestration and implementation sources, including pass order, target filtering, optimization gates, and repeated invocations.
- Spot-checked more than 10 behavioral claims, including scan accumulation, autodiff gates, tagged-union propagation, byte-address options, call bridging, switch defaults, matrix logical operations, phi defaults, constant-array folding, precise diagnostics, boolean casts, and downstream routing.
- Checked line-number citations throughout the body, including all 26 cited IR-validation call sites, emitter spelling ranges, texture-intrinsic locations, and downstream transition registrations.
- Resolved all 186 relative-link occurrences (112 unique destinations) in the working tree; confirmed all ten linked generated peer pages are manifest entries. Dependency documents were consulted only for cross-reference checks.
- Checked required sections, table columns, Mermaid conventions, Liquid safety, and front-matter formatting. The document is 91,805 bytes, within its 98,304-byte cap.
- No files were written. Verification was by source inspection; emitted examples were not recompiled.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Phase A and Phase B diagrams and tables, lines 123–210 and 234–527 | Both diagrams include `calcRequiredLoweringPassSet`, but neither companion table includes the corresponding scan. This violates the required diagram/table correspondence and omits the operations that establish the tables’ lowering gates. | `source/slang/slang-emit.cpp:1085` and `:1666` contain the two scan calls. `docs/generated/design/_meta/prompts/_common.md`, Target-pipeline page contract, requires a companion ordered table with one row per pass node. | Add an unconditional `calcRequiredLoweringPassSet` row after the build-identifier step in Phase A and after `lowerResultType` in Phase B. Link both to `slang-emit.cpp`, identify them as direct calls, and renumber subsequent rows and affected row references. |
| F-002 | minor | Phase B rows 4 and 40; Phase D row 5, lines 454, 490, 766 | Three `File` cells do not point to the files defining their passes: `checkAutodiffPatterns`, `removeWeakUseInsts`, and `simplifyForEmit`. The linked files exist, but do not contain those symbols. | Definitions are at `source/slang/slang-ir-autodiff.cpp:1037`, `source/slang/slang-emit.cpp:1000`, and `source/slang/slang-ir-simplify-for-emit.cpp:505`, respectively. | Replace the three links with `slang-ir-autodiff.cpp`, `slang-emit.cpp`, and `slang-ir-simplify-for-emit.cpp`, respectively. |
| F-003 | minor | Phase D row 1 and Conditional gates, lines 762 and 951 | The description says the WGSL target switch `Sets LineDirectiveMode::None` without mentioning its enclosing condition. Explicitly selected line-directive modes bypass this default selection. | `source/slang/slang-emit.cpp:2985` guards the switch with `if (lineDirectiveMode == LineDirectiveMode::Default)`; the WGSL assignment is at `:3001`. | Qualify both descriptions: all three WGSL targets select `None` **when the configured mode is `Default`**; explicit modes retain their configured value. |
| F-004 | minor | Notable passes → `legalizeIRForWGSL`, lines 1028–1032 | `each stage's entry function takes and returns structs` overstates the varying legalizer’s behavior. Input packing is conditional, and void-returning entry points retain a void result. | `source/slang/slang-ir-legalize-varying-params.cpp:3416` selects only parameters carrying `VaryingInput`; packing returns when none qualify. At `:4186–4187`, `wrapReturnValueInStruct` immediately returns for `IRVoidType`. | Describe conditional packing of varying inputs and wrapping of non-void results. Explicitly note that an entry point can have no packed input struct and can return void. |