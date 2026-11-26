---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-02T21:20:35Z
target_doc: pipeline/03-semantic-check.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: cd6860f053b52ab46cc0816a230dc20093e248da0db8a4bb4c551e2dee5c5a14
source_commit: be3c344414f6541ba6731813b36591a94cd05482
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 0
  minor: 2
  nit: 0
---

# Review report for pipeline/03-semantic-check.md

## Summary

The document substantially matches the source and satisfies the required structure. Two localized behavior summaries need correction: the differentiability section conflates forward and backward function-extension transformations, and the semantic compatibility section overlooks scalar/one-element-vector compatibility. All 36 line-number citations match the current source.

## Items checked

- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show pipeline/03-semantic-check.md`.
- Verified against the **current working tree**. The source comparison against `2518a942d59b1228fc8cc70a37739baab82b6002` produced no differences under `source/`; HEAD matches the supplied review commit.
- Checked all 36 explicit source line-number citations across nine files.
- Spot-checked more than 20 behavioral claims, including checker entry and dispatch, deferred body parsing, generic solver fallback, conformance witnesses, differentiability synthesis, modifier conflicts, GLSL allowances, visibility, dynamic-interface restrictions, generic entry points, capability collection, depth-output collection, semantic type matching, name suggestions, and discarded-result diagnostics.
- Resolved all 61 Markdown links, covering 31 distinct targets including the local heading anchor. Confirmed all four linked generated pages appear in the manifest; consulted the parsing dependency only for cross-reference consistency.
- Checked source filenames and simple backticked identifiers; confirmed diagnostic numbers exist in `slang-diagnostics.lua`.
- Verified required sections, watched-file coverage, front-matter fields, relative-link style, and absence of raw Liquid openers. The document is 32,702 bytes, below its 32,768-byte cap.
- No files were written.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | Differentiability as interface conformance, lines 307–312 | The sentence names `_funcExtensionForwardDiff` / `_funcExtensionBackwardDiff` together and says they rewrite the extension into `IForwardDifferentiable<foo>` with a `fwd_diff` member. That describes only the forward helper; the backward helper constructs a different conformance and members. | `source/slang/slang-check-decl.cpp:17207` implements the forward transformation. At `:17240`, the backward helper uses `getBackwardDiffFuncInterfaceType(baseFuncAsType)` and synthesizes `bwd_diff` with `kIROp_FunctionCopy`, followed by backward context machinery. | Restrict the existing explanation to `_funcExtensionForwardDiff`. Describe `_funcExtensionBackwardDiff` separately as creating an `IBackwardDifferentiable` conformance with a synthesized `bwd_diff` copying the user function and the required backward context members. |
| F-002 | minor | Shader-specific checks, lines 487–492 | The compatibility rule says types must be `both scalar, or both vectors of equal element count`. The implementation compares element counts without retaining the scalar/vector distinction, so a scalar and a one-element vector can also match. | `source/slang/slang-check-shader.cpp:150` implements `getScalarElementType`, assigning scalars count 1 and vectors their declared count. `isSemanticTypeCompatible` at `:175` compares those counts and scalar categories, without checking that both inputs have the same scalar/vector classification. | State that both types must be scalar or vector, with equal element counts—counting a scalar as one element—and matching scalar categories. Explicitly note that this admits scalar/one-element-vector pairs. |