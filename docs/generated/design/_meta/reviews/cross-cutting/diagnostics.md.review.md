---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T08:21:13Z
target_doc: cross-cutting/diagnostics.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: c43e0e630d51ac9b363fa905060d2361e78098b08359d47ea1f1237612ccbd70
source_commit: 801b9e6e57d41f5cef1a5d54c0eeff9e0db4fee6
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 1
  major: 0
  minor: 2
  nit: 0
---

# Review report for cross-cutting/diagnostics.md

## Summary

The document covers all required sections and accurately describes most checked diagnostic machinery. Its most important error is the claimed `fatal error:` prefix for the unknown-exception path: that path writes an unformatted message. The generation walkthrough also misattributes a processing call, and the document exceeds its prompt’s size limit.

## Items checked

- Read `_review.md`, `_common.md`, and the document-specific prompt; ran `regenerate.py show cross-cutting/diagnostics.md`.
- Verified against the **current working tree**. The comparison against `2518a942d59b1228fc8cc70a37739baab82b6002` showed no differences under `source/`; HEAD matches the supplied report commit.
- Spot-checked more than 20 claims, including sink acquisition, constructor settings, parent forwarding, severity ordering and names, warning-group defaults, override precedence, warning promotion, grouped override retention, diagnostic aliases, unknown identifiers, severity-mismatch rejection, generated metadata, variadic notes, token-paste handling, Visual Studio formatting, machine-readable output, color/Unicode selection, internal-error macros, and assertion routing.
- Checked the array-diagnostic examples against `visitGetArrayLengthExpr`, `_synthesizeMemberAssignMemberHelper`, and GLSL varying legalization.
- Verified the sole explicit source line-number citation: `outputExceptionDiagnostic` begins at `source/compiler-core/slang-diagnostic-sink.cpp:981`. The displayed `uninit.slang:6:13` is example output, not a source citation.
- Resolved all 51 Markdown links: 23 distinct file/directory targets exist both in the working tree and at the document’s recorded commit; both local heading targets exist. Confirmed `architecture/overview.md` is in the manifest and consulted it only for cross-reference checking.
- Checked required section coverage, front-matter keys and formats, Markdown conventions, and document size: 31,279 UTF-8 bytes. No Mermaid diagrams or raw Liquid openers appear.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Severity levels`, lines 299–308 | The claim that the unknown-exception path renders `fatal error: An unknown exception occurred` is false. `diagnoseRaw` does not add a severity prefix or newline. This misdescribes observable output for diagnostic consumers. | `source/compiler-core/slang-diagnostic-sink.cpp:985` passes only `"An unknown exception occurred"`. At lines 809 and 815, `diagnoseRaw` writes or appends that message verbatim; lines 823–827 use severity to trigger abortion without formatting it. | Replace the claimed output with `An unknown exception occurred`. Explain that this path has fatal control-flow behavior but bypasses rendering, so it does not produce the rendered name `fatal error`. Adjust the introduction to the “second route” accordingly. |
| F-002 | minor | `## Diagnostic definitions`, numbered step 2, lines 105–109 | After naming `slang-diagnostics-helpers.lua`, the text says `The file ends by calling helpers.process_diagnostics and returning the processed list`. That call and return belong to the catalog file instead. | `source/slang/slang-diagnostics-helpers.lua:1120` returns a table of helpers, exporting `process_diagnostics` at line 1135. `source/slang/slang-diagnostics.lua:6750` calls it, checks validation errors, and returns the processed list at line 6756. | Explicitly attribute the final processing call, validation-error check, and processed-list return to `slang-diagnostics.lua`; describe the helpers file as defining and exporting the processing function. |
| F-003 | minor | Whole document | The document is 31,279 bytes, exceeding the document-specific requirement `Document length under 24 KB`, although it remains below the manifest’s 40,960-byte cap. | `docs/generated/design/_meta/prompts/cross-cutting-diagnostics.md:54` sets the stricter limit. The target file measures 31,279 UTF-8 bytes. | Reduce the page below 24 KB while retaining all required sections. Condense the fatal-entry enumeration, extended reproduction examples, and repeated warning-group and option explanations. |