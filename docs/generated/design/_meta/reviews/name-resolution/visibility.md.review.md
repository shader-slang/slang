---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-30T13:35:08+00:00
target_doc: name-resolution/visibility.md
target_doc_source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
target_doc_watched_paths_digest: 21fdad4e7e32c7256d8962453b7a50b0c472b695cd802a74745c15f4e19be6a0
source_commit: c21ead2690b5b9fa4a582f6b51a4cd5fb34d29d8
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
  major: 2
  minor: 0
  nit: 0
---

# Review report for name-resolution/visibility.md

## Summary
The visibility page has the required structure, valid front matter, and working relative links, and many core visibility claims match the watched sources. I found two major source-alignment issues: the language-version section asserts rejection behavior that is not supported by the checked implementation, and the synthesized-member section describes default inheritance where the checker often writes explicit visibility modifiers.

## Items checked
- Ran `regenerate.py show name-resolution/visibility.md` and reviewed the target document, `_common.md`, the visibility prompt, and the resolved watched-file set.
- Read the dependency documents `ast-reference/modifiers.md`, `ast-reference/declarations.md`, and `name-resolution/scopes.md`.
- Checked front matter for all required keys, the target source commit, warning string, and 64-character hex watched-path digest.
- Resolved the page's relative links to peer name-resolution docs, AST-reference docs, glossary, watched source files, and `include/slang.h`.
- Spot-checked source-backed claims for `VisibilityModifier`, `PublicModifier`, `PrivateModifier`, `InternalModifier`, `DeclVisibility`, `ModuleDecl::defaultVisibility`, `getDeclVisibility`, `checkModule`, `isDeclVisibleFromScope`, `filterLookupResultByVisibilityAndDiagnose`, `TryCheckOverloadCandidateVisibility`, `checkVisibility`, `DeclPassesLookupMask`, `IgnoreForLookupModifier`, and the cited diagnostics.
- Checked the language-version and synthesized-visibility sections against nearby implementation code because those claims affect user-visible access behavior.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Concepts`, `SlangLanguageVersion languageVersion` bullet, lines 64-76 | The page says `SlangGlobalSessionDesc::minLanguageVersion` means "New sessions therefore reject modules whose declared version is older than 2025 by default." That rejection behavior is not supported by the checked sources: `include/slang.h` only defines the field default, while `isValidSlangLanguageVersion` still accepts `SLANG_LANGUAGE_VERSION_LEGACY`, and the parser/preprocessor paths accept the `legacy` language version. | `include/slang.h` lines 5654-5655 only say `minLanguageVersion` is the "oldest Slang language version that any sessions will use"; `source/slang/slang-compiler.cpp` lines 7-14 returns true for `SLANG_LANGUAGE_VERSION_LEGACY`; `source/slang/slang-preprocessor.cpp` lines 4543-4569 accepts `legacy` when the version is valid. | Remove the rejection claim, or replace it with a source-backed statement limited to the enum/default values. If rejection/minimum-version enforcement is intended to be documented, add the actual enforcement path to the manifest and cite it. |
| F-002 | major | `### Generic parameters, accessors, and synthesized members`, lines 260-267 | The page says synthesized members are "constructed without an explicit visibility modifier and inherit the parent's default," with only a few synthesis sites explicitly propagating visibility. The watched checker code contradicts that framing: several synthesized declarations explicitly call `addVisibilityModifier`, often using the parent visibility or the minimum of parent and requirement visibility. This also leaves the prompt-required `slang-check-decl.cpp` synthesis sites under-cited. | `source/slang/slang-check-decl.cpp` lines 7442-7448, 8523-8529, and 8902-8908 explicitly add visibility to synthesized requirement members using `Math::Min`; lines 3873-3875 and 3933-3934 explicitly propagate synthesized differential member visibility; `source/slang/slang-check-expr.cpp` lines 817-818 explicitly adds visibility to synthesized differential declarations. | Rewrite the paragraph to say synthesized declarations are assigned visibility at their synthesis sites, commonly from the parent or from `Math::Min(parent, requirement)`, and cite the relevant `slang-check-decl.cpp` sites. Do not imply default inheritance is the normal mechanism for the listed examples. |

## No-issues notes
- The `DeclVisibility` enum values and their numeric order match `source/slang/slang-ast-support-types.h`.
- The lookup-boundary and overload-resolution filtering descriptions match `filterLookupResultByVisibilityAndDiagnose` and `TryCheckOverloadCandidateVisibility`.
- The `IgnoreForLookupModifier` discussion matches the enum tag-type producer and the lookup-side skip.
