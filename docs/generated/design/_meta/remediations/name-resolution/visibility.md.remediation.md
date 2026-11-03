---
remediation_report: true
remediator_model: claude-sonnet-5-5
remediated_at: 2026-11-03T07:00:00Z
target_doc: name-resolution/visibility.md
review_report: ../../reviews/name-resolution/visibility.md.review.md
target_doc_source_commit_before: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 1
  escalated: 0
---

# Remediation report for name-resolution/visibility.md

## Summary

Two findings (F-001, F-002) were verified against the source and fixed in the document; F-003 is valid but needs a manifest change and was deferred. The doc stays under its 32,768-byte cap (32,649 bytes) because the additions were paid for by tightening nearby text.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-check-expr.cpp:1281-1289` grants access only on target-type equality or successful `applyExtensionToType`; the old "different instantiation" wording was too broad. | Edge-case bullet rewritten: access requires equal targets or successful specialization; `G<int>` vs `G<float>` excluded; lines 1281-1289 cited. Lines 308-316 already carry the qualification. |
| F-002 | fixed | `source/slang/slang-lookup.cpp:450-462` only skips inheritance facets whose declared subtype witness names a marked `InheritanceDecl`; `slang-ast-modifier.h:276-279` documents it as an `InheritanceDecl` modifier. | Reworded the Concepts bullet, the "Interaction with" section and the edge-case bullet to the enum tag-type facet only. |
| F-003 | deferred | `source/slang/slang-syntax.cpp` is cited but absent from the manifest watched paths; remediation may not edit `_meta/manifest.yaml`. Follow-up: add the file to the manifest entry for this doc, then rerun mark-fresh. | — |
