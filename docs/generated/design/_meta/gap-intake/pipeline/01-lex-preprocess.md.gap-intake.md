---
gap_intake_report: true
intake_model: claude-sonnet-5-5
intake_at: 2026-11-02T00:00:00Z
target_doc: pipeline/01-lex-preprocess.md
target_doc_source_commit_before: 48c746dc1eda1c6e2aa98c17bbdb7a645c24a048
target_doc_source_commit_after: 2518a942d59b1228fc8cc70a37739baab82b6002
gap_count: 9
actions:
  fixed: 0
  rejected_bogus: 9
  rejected_out_of_scope: 0
  deferred: 0
  escalated_to_finding: 0
---

# Gap-intake report for pipeline/01-lex-preprocess.md

## Summary

All nine reopened gaps were re-verified against the document text and the
watched source; each asks for material the document already states with
source-confirmed detail, so all nine are `rejected-bogus` (no gap-driven
edits, no escalations). The document was separately refreshed to the
current commit: line citations were re-derived and the
`SourceLanguageDirective` / first-selection-wins preprocessor change was
documented.

## Actions

| Gap ID | Action | Evidence | Fix summary |
| --- | --- | --- | --- |
| 8082592cbed7 | rejected-bogus | Doc "Failure modes" already lists cyclic include and a stray `#else`/`#elif`/`#endif`, naming `DirectiveWithoutIf` and `CyclicInclude`; source: source/slang/slang-preprocessor.cpp lines 3530, 3710-3714 | — |
| 49106a3707d8 | rejected-bogus | Doc "Literal scanning" says only `octalLiteral` is a warning, scanning continues into `_lexNumber(lexer, 8)`, other two are errors; source: source/compiler-core/slang-lexer.cpp lines 2097-2099, 551, 1797 | — |
| 72ada6869b5d | rejected-bogus | Doc contains the `1e300lf` (Double) vs `1e300` (Float, INFINITY) example; source: source/compiler-core/slang-lexer.cpp lines 1272-1283, 1329-1333 | — |
| aa4d7f543736 | rejected-bogus | Doc "Macro expansion" states expansion is self-suppressing via the busy list and a self-naming macro stays a plain identifier; source: source/slang/slang-preprocessor.cpp lines 1462, 1728 | — |
| 4cc2b13c259b | rejected-bogus | Doc states a wrong argument count is diagnosed with `WrongNumberOfArgumentsToMacro` and contributes no tokens; source: source/slang/slang-preprocessor.cpp lines 1870, 1892 | — |
| ef72a4b5521d | rejected-bogus | Doc "Preprocessor directives" describes `#language`/`#lang`, `#version`, `#extension` effects and lists `once` and `warning` in `kPragmaDirectives`; source: source/slang/slang-preprocessor.cpp lines 4445-4448, 4558, 4593, 4731-4736 | — |
| 3e1202309230 | rejected-bogus | Doc states `#if` values are plain signed `int`, undefined identifier is 0 with a warning, overflow undetected; source: source/slang/slang-preprocessor.cpp lines 2984, 3117, 3194 | — |
| 2ed4b39212fa | rejected-bogus | Doc states an unknown `#pragma` sub-directive warns (`UnknownPragmaDirectiveIgnored`) versus the unknown-directive error; source: source/slang/slang-preprocessor.cpp lines 4242-4244, 4677 | — |
| f64c1edac915 | rejected-bogus | Doc category 2 gives the `TWICE(x)` worked example distinguishing call-site from body locations; source: source/slang/slang-preprocessor.cpp lines 2351, 2494 | — |
