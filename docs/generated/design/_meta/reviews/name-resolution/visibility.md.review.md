---
review_report: true
reviewer_model: gpt-5.5
reviewed_at: 2026-06-12T13:38:32+00:00
target_doc: name-resolution/visibility.md
target_doc_source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
target_doc_watched_paths_digest: 7000c50536855f3dc1b45bca7d5d1e8dd84cac67a0962f289a509743162726d3
source_commit: eb9403ef595a99c2ff6def1d538dbd7a792d9371
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 2
  minor: 1
  nit: 0
---

# Review report for name-resolution/visibility.md

## Summary
The page is broadly aligned with the visibility implementation and resolves its peer links, but it has two substantive review findings. The most important issue is a factual error in the concepts section: it names `SessionDesc::minLanguageVersion`, which does not exist; the source field is `SlangGlobalSessionDesc::minLanguageVersion`. The page also omits a prompt-required re-export failure mode and gives the wrong diagnostic for duplicated visibility modifiers.

## Items checked
- Ran `regenerate.py show name-resolution/visibility.md` and checked the manifest entry, per-doc prompt, ten resolved watched files, and three depends-on docs.
- Verified target front matter, required section order, source paragraph, concepts list, rules subsections, edge cases, and peer links against the common and per-page prompt contracts.
- Resolved the document's relative links to source files, peer name-resolution pages, AST reference pages, and the glossary; the linked peers exist in the manifest.
- Spot-checked 16 factual/source-alignment claims at `eb9403ef595a99c2ff6def1d538dbd7a792d9371`, including the visibility modifier classes, `DeclVisibility`, `ModuleDecl::defaultVisibility`, language-version constants, `getDeclVisibility`, lookup filtering, overload filtering, private extension access, `getTypeVisibility`, `checkVisibility`, `ExternModifier`, `ExportedModifier`, `IgnoreForLookupModifier`, and the listed diagnostics.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Concepts`, `SlangLanguageVersion` bullet | The page says ``SessionDesc::minLanguageVersion`` defaults to `SLANG_LANGUAGE_VERSION_2025`, but `SessionDesc` has no such field. The field with that default is `SlangGlobalSessionDesc::minLanguageVersion`, so the cited API surface is wrong. | `include/slang.h:4244` declares `struct SessionDesc` and its fields through `skipSPIRVValidation` without `minLanguageVersion`; `include/slang.h:5565` declares `struct SlangGlobalSessionDesc`, and `include/slang.h:5574` sets `minLanguageVersion = SLANG_LANGUAGE_VERSION_2025`. | Change the concept bullet to name `SlangGlobalSessionDesc::minLanguageVersion`, or remove the session-default sentence if this page should only discuss per-module language defaults. |
| F-002 | major | `## Edge cases and failure modes` | The per-page prompt requires an edge case for "a `using`-decl that re-exports an `internal` decl from another module: how it is rejected," but the generated edge-case list does not cover `using`, re-exporting, `FuncAliasDecl`, or the non-exported-import diagnostic path. | `docs/generated/design/_meta/prompts/name-resolution-visibility.md:93` requires this edge case. The closest source-backed rejection path checks public callable aliases: `source/slang/slang-check-decl.cpp:9012` starts `validatePublicCallableOperandVisibility`, `source/slang/slang-check-decl.cpp:9025` handles `FuncAliasDecl`, and `source/slang/slang-diagnostics.lua:2652` defines `public-custom-derivative-uses-non-exported-import`. | Add the required edge case, naming the actual source path and diagnostic if this is the intended re-export case. If the watched source no longer has a `using`-specific rejection path, state that the information was not located in the watched paths and identify the additional path or prompt correction needed. |
| F-003 | minor | `## Edge cases and failure modes`, "Visibility modifier on a node that does not accept one" bullet | The page says `invalid-visibility-modifier-on-type-of-decl` fires when "the user writes `public public ...`", but duplicate visibility modifiers are handled by the duplicate-modifier conflict-group check, not by `InvalidVisibilityModifierOnTypeOfDecl`. | `source/slang/slang-check-modifier.cpp:1501` maps `PublicModifier`, `PrivateModifier`, and `InternalModifier` to the `VisibilityModifier` conflict group; `source/slang/slang-check-modifier.cpp:2295` checks mutually exclusive modifier conflicts and emits `Diagnostics::DuplicateModifier` at `source/slang/slang-check-modifier.cpp:2306`. The duplicate diagnostic is defined as `duplicate-modifier` in `source/slang/slang-diagnostics.lua:2709`. | Remove `public public ...` from this bullet or change it to cite `DuplicateModifier`; keep `invalid-visibility-modifier-on-type-of-decl` for the namespace/unsupported-node cases that actually emit that diagnostic. |

## No-issues notes
- The front matter has all required generated-doc keys and the watched-path digest is a well-formed SHA-256 hex string matching the manifest context shown for this review.
- `getDeclVisibility`, `isDeclVisibleFromScope`, `filterLookupResultByVisibilityAndDiagnose`, and `TryCheckOverloadCandidateVisibility` are all cited at line ranges that match the current source layout.
- The `IgnoreForLookupModifier` discussion matches the enum tag-type inheritance producer and the lookup-side skip.
