---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T09:12:40Z
target_doc: syntax-reference/keywords-and-builtins.md
review_report: ../../reviews/syntax-reference/keywords-and-builtins.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for syntax-reference/keywords-and-builtins.md

## Summary
All six findings were verified against source at the recorded commit and all six were fixed. The three major findings were real: the statement table was missing `__requireCapability`, the `core.meta.slang` bullet named three declarations that do not exist anywhere in the watched meta-modules, and the page overstated how ordinary meta-module declarations participate in parsing. The three minor findings (table-registration attribution, the "take arguments" modifier heading, and the split first paragraph) were also confirmed and corrected. The document was edited; the linter passes.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | `source/slang/slang-parser.cpp:6981-6983` dispatches `LookAheadToken("__requireCapability")` to `ParseRequireCapabilityStatement` (defined at 7601); the table listed the neighboring internal branches but not this one. | Added a `__requireCapability` row citing lines 6981 and 7601, marked compiler-internal |
| F-002 | fixed | `Result` occurs zero times in `source/slang/core.meta.slang`, and no `Range`/`Iterator` declaration exists in any watched meta-module; only `struct Optional` (1825) and `struct Tuple` (1941) are real. | Bullet reduced to scalar/vector/matrix types, `Optional`, `Tuple`, and core intrinsics |
| F-003 | fixed | `tryLookUpSyntaxDecl` at `source/slang/slang-parser.cpp:1115-1144` returns `nullptr` unless the decl casts to `SyntaxDecl`, so ordinary function/type/operator decls never drive a parser callback. | Both passages now call meta-module names built-in vocabulary, with only `SyntaxDecl` bindings driving callbacks |
| F-004 | fixed | `getSyntaxParseInfos()` (10869-10872) only wraps the array in a `ConstArrayView`; `populateBaseLanguageModule` (10874-10891) loops over it calling `addBuiltinSyntaxImpl`. | "populated by `getSyntaxParseInfos()`" replaced with the exposes/registers split |
| F-005 | fixed | `parseSharedModifier` (10262-10279) and the callbacks that follow only select and construct modifier nodes; they read no trailing syntax. | Heading changed to "Callback-parsed modifiers (some take arguments)" |
| F-006 | fixed | `docs/generated/design/_meta/prompts/_common.md:65-66` requires the first body paragraph to state coverage and intended reader; the audience sat in a second paragraph. | Merged the intended-reader sentence into the opening paragraph |
