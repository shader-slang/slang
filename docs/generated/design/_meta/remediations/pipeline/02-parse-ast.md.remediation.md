---
remediation_report: true
remediator_model: claude-opus-4.7
remediated_at: 2026-05-15T19:30:00+00:00
target_doc: pipeline/02-parse-ast.md
review_report: ../../reviews/pipeline/02-parse-ast.md.review.md
target_doc_source_commit_before: 3da83a82d83ad1b0fbd58465ed3a89d2880533dd
target_doc_source_commit_after: 470b96e8c29ca660c537d4d0f88cc21a12f962e6
actions:
  fixed: 1
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for pipeline/02-parse-ast.md

## Summary

Restructured the page to match the prompt's required headings.
"Generics and where-clauses" is now `## Generics ambiguity` (with new
content covering the `<` disambiguation strategy), and "Modifiers
and attributes" is now `## Modifier parsing`. The two sections are
re-ordered so generics precedes modifiers (matching the prompt
ordering).

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | The prompt mandates `## Generics ambiguity` and `## Modifier parsing`; the page used different headings and was missing coverage of the `<` disambiguation strategy that the "Generics ambiguity" section is supposed to discuss. | Renamed and reordered the two sections. Added a paragraph to `## Generics ambiguity` explaining that the parser uses a try/rollback strategy (not a single-token heuristic) and noting that generic *declarations* are syntactically unambiguous because they follow a declaration keyword. Linked to `../../design/parsing.md` for the deeper treatment, as the prompt requests. |
