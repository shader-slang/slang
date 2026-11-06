---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T21:20:01Z
target_doc: pipeline/04b-pre-link-passes.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: d9fbf98cc4d17c2b8019253e037a7e9366f0b2086a5feb13733c0efd78728ffe
source_commit: be3c344414f6541ba6731813b36591a94cd05482
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 2
  major: 0
  minor: 2
  nit: 0
---

# Review report for pipeline/04b-pre-link-passes.md

## Summary

The document covers the required pipeline structure, and its ordering, gates, and line citations match the source. Four behavioral claims need correction. Most importantly, disabling final-link autodiff pruning does not mean prelink always deep-clones differentiable-interface witness tables: its cloning override can create declarations without entries.

## Items checked

- Read `_review.md`, `_common.md`, the document-specific prompt, and the resolved watched paths from `regenerate.py show`.
- Verified against the **current working tree**, which the operator identifies as identical to the document’s source commit throughout `source/`.
- Checked all 112 explicit line-number citation occurrences, including ranges and repeated citations.
- Spot-checked more than 20 behavioral claims: phase ordering, debug-source gates, coverage instrumentation, prelink registration and cloning, inlining termination, validation gates, diagnostics, stripping, obfuscation, module indexing, and adjacent IR-generation routines.
- Resolved all 99 Markdown link occurrences in the working tree; checked generated-page references against the manifest. Historical Git-object verification did not complete; link verification uses the requested working-tree baseline.
- Consulted dependency documents only for cross-reference checks.
- Checked required sections, phase-table columns, Mermaid conventions, front-matter fields, and Liquid safety. The document is 48,193 bytes, below its 65,536-byte cap.
- No files were written; diagnostic examples were checked against source and existing test expectations, without running compiler tests.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Notable passes`, lines 579–588 | The claim that prelink “always deep-clones differentiable-interface witness tables” confuses the shared pruning predicate with the actual prelink cloning path. It also contradicts the earlier description of bodiless witness-table clones. | `source/slang/slang-ir-link.cpp:2611` handles `kIROp_WitnessTable` in `IRPrelinkContext::maybeCloneValue` by creating a table with cloned types; lines 2645–2651 register it and clone decorations without copying entries. This bypasses `cloneWitnessTableImpl` and its `shouldDeepCloneWitnessTable` decision at line 759. | State that prelink disables the shared final-codegen pruning policy, while retaining its separate shallow-import policy. Explain that witness tables reached through the override can remain declarations without entries; remove the unconditional deep-cloning guarantee. |
| F-002 | critical | `### performMandatoryEarlyInlining and the surrounding loop`, lines 668–670 | “Functions tagged with it must not survive the pre-link pipeline” incorrectly promises removal of function definitions. The pass replaces eligible calls, and exported definitions can remain alive. “Inlines every call” also omits eligibility checks. | `source/slang/slang-ir-inline.cpp:148` first applies `canInline`; line 282 rejects generic-assembly bodies. Line 940 removes the original **call**. `source/slang/slang-ir-dce.cpp:542–559` retains exported definitions when `keepExportsAlive` is enabled, as it is at `source/slang/slang-lower-to-ir.cpp:15650`. | Describe mandatory early inlining as selecting eligible call sites without a profitability heuristic. Remove the guarantee that tagged definitions disappear, and mention that exported definitions can survive DCE. |
| F-003 | minor | Phase D diagnostic table, line 336 | “A call argument does not reach” `E40012` is too broad. A constexpr demand originating at a call can propagate through an argument’s producer chain and produce the general diagnostic. | `source/slang/slang-ir-constexpr.cpp:543–557` emits `E40013` only if the argument cannot be marked constexpr during backward propagation; lines 610–671 subsequently validate propagated demands. `tests/diagnostics/constexpr-diagnostic-location.slang:36–37` expects “expected a compile-time constant” at the argument to `Load`. | Limit the `E40013` explanation to arguments that cannot be marked constexpr immediately. State that propagated demands on argument-producing instructions can instead produce `E40012`, and cite the existing regression example. |
| F-004 | minor | `## Phase A`, lines 173–178 | The NVAPI explanation presents both macros together as the trigger without mentioning that the register-space macro is optional. | `source/slang/slang-compile-request.h:318–333` requires `NV_SHADER_EXTN_SLOT`, but explicitly permits omission of `NV_SHADER_EXTN_REGISTER_SPACE`, initializing its value to `"space0"`. | Say that defining `NV_SHADER_EXTN_SLOT` triggers the modifier; `NV_SHADER_EXTN_REGISTER_SPACE` optionally overrides the default `space0`. |