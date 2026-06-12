---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-05T14:58:02+00:00
target_doc: syntax-reference/keywords-and-builtins.md
target_doc_source_commit: 52339028a2aa703271533454c6b9528a534bac31
target_doc_watched_paths_digest: 20804753dbe34bcc88e551a3b9b9e42d42729a73662c7d2065776e63a87d47a1
source_commit: fb192be9f5b3b58555e034599e072158e5c48dfd
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 2
severity_breakdown:
  critical: 0
  major: 1
  minor: 1
  nit: 0
---

# Review report for syntax-reference/keywords-and-builtins.md

## Summary
The page is largely source-aligned: parser-registered declarations, modifiers, expressions, and sampled meta-module names match the watched files. Two issues remain: several required sections are renamed or split away from the prompt contract, and the reserved-prefix discussion names public spellings the parser does not register.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show syntax-reference/keywords-and-builtins.md` and used the listed watched files at `52339028a2aa703271533454c6b9528a534bac31`.
- Read the per-document prompt, `_common.md`, and dependency doc `syntax-reference/tokens.md`.
- Resolved all relative markdown links and anchors in the target document.
- Verified required front matter keys and checked that the target digest is a 64-character hex value.
- Spot-checked at least 14 source-alignment claims, including statement keyword dispatch, `g_parseSyntaxEntries`, `_makeParseDecl`, `_makeParseModifier`, `_makeParseExpr`, `new` handling in `parsePrefixExpr`, `struct`, `class`, and `enum` special handling, core scalar aliases, HLSL wave builtins, GLSL vector aliases, and diff attribute declarations.
- Checked the required section list from `docs/generated/design/_meta/prompts/syntax-keywords-and-builtins.md`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | Required section headings | The prompt requires `## Lexer-recognized keywords`, `## Parser-registered syntax keywords`, and `## Core-module syntax declarations`, but the document uses `## Lexer-recognized symbols`, splits parser syntax across separate statement, declaration, modifier, and expression sections, and uses `## Core-module-supplied vocabulary`. | `docs/generated/design/_meta/prompts/syntax-keywords-and-builtins.md:28-54` names the required sections; the target headings are `## Lexer-recognized symbols`, `## Statement keywords`, `## Decl keywords`, `## Modifier keywords`, `## Expression keywords`, and `## Core-module-supplied vocabulary`. | Rename and regroup the sections to match the prompt contract, preserving the useful subgroups as subsections under `## Parser-registered syntax keywords` and renaming the core-module section to `## Core-module syntax declarations`. |
| F-002 | minor | `## Reserved identifier prefixes` | The text says many double-underscore names have public spellings without underscores and gives `init`, `subscript`, and `include` as examples, but the parser registers only `__init`, `__subscript`, and `__include` for those three. | `source/slang/slang-parser.cpp:10397-10406` registers `__init`, `__subscript`, and `__include`; the same range registers public spellings only for `extension` and `import`. | Remove `init`, `subscript`, and `include` from the public-spelling examples, or qualify the sentence so only `extension` and `import` are presented as parser-registered public spellings. |
