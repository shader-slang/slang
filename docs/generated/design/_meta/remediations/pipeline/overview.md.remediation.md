---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-02T21:20:00Z
target_doc: pipeline/overview.md
review_report: ../../reviews/pipeline/overview.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/overview.md

## Summary

Two findings were fixed and one was rejected as out of scope. The
unsupported `slangc` command-line ordering claim was removed and
replaced with source-backed statements, and the phi and target-spelling
detail was cut back to roadmap-level prose. The stage-heading level
request was rejected because it conflicts with the document's own
structure.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Verified: `source/slang/slang-options.cpp:1726` groups all `.slang` inputs into one translation unit, and `source/slang/slang-session.cpp:1745` only consults the dictionary before the file search (~line 1881); `slang-compile-request.cpp:717-725` supports only the sibling-registration limitation. | Driver entry points: dropped the command-line ordering claim; limited the sibling limitation to the request-local dictionary and noted the single-translation-unit grouping and independent file lookup. |
| F-002 | fixed | Verified against the pipeline-overview prompt checklist ("purely a roadmap"); the phi/optimizer detail and seven-target spelling block belong in `04-ast-to-ir.md`, `05-ir-passes.md` and `06-emit.md`. | AST to IR and IR passes: condensed phi text to one short paragraph, removed the thread-group spelling block, kept brief target-sensitivity description. |
| F-003 | rejected-out-of-scope | The prompt (`_meta/prompts/pipeline-overview.md`, required structure items 2-3) makes `## Stages` itself a level-2 section containing a subsection per stage; promoting the stages to `##` would leave `## Stages` empty and break that structure, so `###` is the correct nesting. | — |
