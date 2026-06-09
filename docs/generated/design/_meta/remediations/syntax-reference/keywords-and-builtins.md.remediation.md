---
remediation_report: true
remediator_model: claude-opus-4.8
remediated_at: 2026-06-05T15:45:00Z
target_doc: syntax-reference/keywords-and-builtins.md
review_report: ../../reviews/syntax-reference/keywords-and-builtins.md.review.md
target_doc_source_commit_before: 52339028a2aa703271533454c6b9528a534bac31
target_doc_source_commit_after: 52339028a2aa703271533454c6b9528a534bac31
actions:
  fixed: 2
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/keywords-and-builtins.md

## Summary

Both findings were fixed. Section headings were renamed and regrouped
to match the prompt contract (`## Lexer-recognized keywords`, a single
`## Parser-registered syntax keywords` parent with statement / decl /
modifier / expression subsections, and `## Core-module syntax
declarations`), and the reserved-prefix paragraph no longer claims
parser-registered public spellings for `init`, `subscript`, and
`include`.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `docs/generated/design/_meta/prompts/syntax-keywords-and-builtins.md:28-54` mandates the three section names and the grouped parser-keyword section. | Renamed `## Lexer-recognized symbols` to `## Lexer-recognized keywords`; added `## Parser-registered syntax keywords` and demoted Statement/Decl/Modifier/Expression to `###` (Simple/Complex modifiers to `####`); renamed `## Core-module-supplied vocabulary` to `## Core-module syntax declarations`. |
| F-002 | fixed | `source/slang/slang-parser.cpp:10395,10405` register public `extension`/`import`, but `__init`/`__subscript`/`__include` (10397,10398,10406) have no underscore-free entry. | Reworded the public-spelling sentence to list only `extension` and `import` and to note `__init`, `__subscript`, `__include` lack underscore-free spellings. |
