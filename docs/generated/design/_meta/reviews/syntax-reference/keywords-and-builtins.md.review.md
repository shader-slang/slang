---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:17:44+00:00
target_doc: syntax-reference/keywords-and-builtins.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 12225714da6281dfb4ac737612c99f7829fb0cf890341684715ac2d919b445cb
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 1
severity_breakdown:
  critical: 0
  major: 0
  minor: 1
  nit: 0
---

# Review report for syntax-reference/keywords-and-builtins.md

## Summary
The keyword inventory is broadly aligned with the parser syntax table, hardcoded statement dispatch, token catalog, and watched meta-module files. The only finding is a stale source citation for the `new` expression keyword; the behavior is described correctly, but the line reference points at the wrong parser function.

## Items checked
- Ran `python3 docs/generated/design/_meta/regenerate.py show syntax-reference/keywords-and-builtins.md`; reviewed the listed watched files and dependency doc `syntax-reference/tokens.md`.
- Verified the target front matter fields, source commit, digest shape, title, required sections, and relative links.
- Checked all explicit line-number citations in the target body against `source/slang/slang-parser.cpp`, including statement dispatch, `g_parseSyntaxEntries[]`, struct/class/enum special cases, `catch`, and `new`.
- Spot-checked more than 30 listed keywords against `slang-parser.cpp`, including declaration, statement, modifier, and expression registrations.
- Checked representative builtin names in `core.meta.slang`, `hlsl.meta.slang`, `glsl.meta.slang`, and `diff.meta.slang`, including `Optional`, `Tuple`, `vector`, `Texture2D`, `RWStructuredBuffer`, `WaveGetWaveIndex`, `SV_WaveIndex`, `SV_GroupIndex`, `vec3`, `mat4`, `gl_Position`, and `DifferentialPair`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | `## Parser-registered syntax keywords`, expression keyword row for `new` | The `new` row says it is parsed by `parsePrefixExpr` at `slang-parser.cpp line 9372` and that `parsePrefixExpr` is defined at line 9364, but those lines now belong to SPIR-V assembly parsing. The behavior claim is correct, but the source citation is stale by about 80 lines. | `source/slang/slang-parser.cpp:9370-9373` starts `parseSPIRVAsmExpr`; `source/slang/slang-parser.cpp:9441-9453` defines `parsePrefixExpr` and handles `AdvanceIf(parser, "new")`. | Update the row to cite `parsePrefixExpr` at the current line range, especially the `AdvanceIf(parser, "new")` branch. |

## No-issues notes
- The document correctly distinguishes lexer-level token kinds from parser-recognized identifier keywords.
- The declaration and modifier keyword tables match the current `g_parseSyntaxEntries[]` entries spot-checked in `source/slang/slang-parser.cpp`.
- The `struct`, `class`, and `enum` explanation correctly notes that these are special-cased in the type-specifier parser rather than registered through `_makeParseDecl`.
- The HLSL, GLSL, core, and autodiff meta-module examples are present in the watched `*.meta.slang` files.
