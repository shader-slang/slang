---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-30T13:58:52Z
target_doc: cross-cutting/core-module.md
review_report: ../../reviews/cross-cutting/core-module.md.review.md
target_doc_source_commit_before: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_source_commit_after: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
actions:
  fixed: 0
  rejected_bogus: 1
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for cross-cutting/core-module.md

## Summary

The review filed one minor finding (F-001) about the GLSL-module loading
description. Verified against the current target document and source: the
sentence the finding quotes as problematic does not exist in the doc, and the
doc already states exactly the wording the finding recommends. F-001 does not
apply to the current doc, so it is rejected as bogus. No edit was made this
cycle; `target_doc_source_commit_after` equals `_before`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | rejected-bogus | The quoted text "Loading it is target-conditional: the compiler pulls it in when the user is compiling GLSL or asks for GLSL-flavoured names" is absent from the target doc. `docs/generated/design/cross-cutting/core-module.md:121-127` already ties loading to `SlangGlobalSessionDesc::enableGLSL`, cites the `if (desc->enableGLSL)` branch (verified `source/slang/slang-api.cpp:218`) and the `glslModuleName` / `getBuiltinModule(BuiltinModuleName::GLSL)` retrieval (verified `source/slang/slang-session.cpp:1520,1523`) — i.e. the finding's own recommendation is already implemented in the doc. | — |
