---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T14:50:00Z
target_doc: pipeline/04b-pre-link-passes.md
review_report: ../../reviews/pipeline/04b-pre-link-passes.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/04b-pre-link-passes.md

## Summary

Three findings were reviewed. The fabricated `Module::compile` caller and the overstated layout-module description were both verified against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23` and fixed. The front-matter digest finding was rejected as out of scope because that field belongs to the operator's `mark-fresh` run. The document was edited.

## Actions

| Finding ID | Action                | Rationale                                                                                                                                                                                                                                                                                                                                                                                                                      | Fix summary                                                                                                                                                                                                                                                                                                          |
| ---------- | --------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------ | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001      | fixed                 | `rg -n "Module::compile" source/slang/` returns nothing at HEAD. The real callers are `FrontEndCompileRequest::generateIR` in `source/slang/slang-compile-request.cpp` (which calls `generateIRForTranslationUnit` at line 552) and the imported-module path in `source/slang/slang-session.cpp` (line 1130). `generateIRForTranslationUnit` itself is still at `source/slang/slang-lower-to-ir.cpp:15386`, matching the page. | `## Source`: replaced "from `Module::compile` and friends" with the two real callers, linked but deliberately without line numbers since neither file is in this page's `watched_paths`.                                                                                                                             |
| F-002      | fixed                 | Confirmed at HEAD: `createIRModuleForLayout` at `source/slang/slang-lower-to-ir.cpp:16353` decorates the module instruction at `:16450` and adds `addRequireCapabilityAtomDecoration` on entry-point stubs at `:16491`, in addition to the stub globals and their layout instructions.                                                                                                                                         | `### TargetProgram::createIRModuleForLayout`: "whose only contents are `IRLayoutDecoration`s" replaced with stubs, module-instruction and stub layout decorations plus their layout instructions, and entry-point capability decorations; the no-executable-bodies and no-mandatory-passes disclaimers are retained. |
| F-003      | rejected-out-of-scope | `docs/generated/design/_meta/prompts/_remediate.md` lines 97-100 reserve `generated_at`, `source_commit`, and `watched_paths_digest` for the operator's `regenerate.py mark-fresh` run: "Do not edit those three fields yourself." The digest is refreshed when the operator marks this page fresh after the edits above.                                                                                                      | —                                                                                                                                                                                                                                                                                                                    |
