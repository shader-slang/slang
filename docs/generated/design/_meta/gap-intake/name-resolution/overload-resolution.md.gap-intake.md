---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-03T00:00:00Z
target_doc: name-resolution/overload-resolution.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 1
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for name-resolution/overload-resolution.md

## Summary

One gap was open, `2b32ad0568a6` (`drift-from-source` against
`#the-builtin-operator-fast-path`), and it is `fixed`. The source agrees
with the observation, not the document, so this is a documentation
defect rather than a compiler bug: nothing was rejected, deferred, or
escalated. The edit was made during the focused refresh of the page to
`2518a942d59b1228fc8cc70a37739baab82b6002`, which also corrected stale
line citations and other drift in the page; this report covers only the
open gap. It replaces the earlier intake report for this page, whose
eight decisions are already recorded in the ledger.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 2b32ad0568a6 | fixed | `source/slang/slang-check-impl.h` lines 1056-1074: `SharedSemanticsContext::isGLSLOperatorScope()` returns `isGLSLSourceLanguage() \|\| m_hasImportedGLSLModule`, and `isGLSLSourceLanguage()` reads the translation unit's resolved `sourceLanguage` (false when there is no translation-unit request). `source/slang/slang-check-decl.cpp` line 18683 sets `m_hasImportedGLSLModule` only when a module named `glsl` is imported, with a comment that it enables only the operator rules. `source/slang/slang-compile-request.cpp` lines 778-807 (outside the watched paths) show `-allow-glsl` being normalized into `sourceLanguage = GLSL` on every translation unit with `DeprecatedAllowGlslOption`, so it is not an independent input. The `SemanticsExprVisitor::isGLSLOperatorScope` declaration comment at `slang-check-impl.h` lines 4592-4594 still carries the old `-allow-glsl` wording; the code is authoritative. The "do not combine `-allow-glsl` with `import glsl;`" advice in the suggested addition was not confirmed in the source and was not written. | Replaced the "`-allow-glsl` or `glsl` module in scope" sentence with the two real inputs (resolved GLSL source language, explicit `import glsl;`) and a paragraph marking `-allow-glsl` deprecated and reachable only through the first input. |
