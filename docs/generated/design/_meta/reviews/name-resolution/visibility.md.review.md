---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:06:30+00:00
target_doc: name-resolution/visibility.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 94e3bb442f068e23a07ff33e9536a9fc2e08c2fa82513f4ca6488832ebf31946
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: partial
finding_count: 6
severity_breakdown:
  critical: 0
  major: 0
  minor: 6
  nit: 0
---

# Review report for name-resolution/visibility.md

## Summary
The page is complete, well structured, and link-clean, and most claims match the recorded source commit. Six bounded inaccuracies remain; the most important workflow issue is that the recorded watched-path digest no longer matches the manifest's resolved watched files.

## Items checked
- Read the target, `_common.md`, the per-document prompt, all 12 resolved watched files, and all three dependency documents.
- Confirmed that the target's recorded source commit is current `HEAD` and that the watched source files have no worktree differences from that commit.
- Re-derived every line-number citation in the body across the watched files and spot-checked more than 30 factual and behavioral claims.
- Resolved every relative link at the recorded commit, confirmed every generated peer is in the manifest, and ran the generated-doc linter successfully.
- Verified all required front-matter fields and recomputed the watched-path digest.

## Findings
| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | minor | Front matter, line 6 | The recorded `watched_paths_digest` is `94e3bb...`, but the current manifest entry resolves to a digest of `e44865f09117c2ed040c2745cf741834f8cd6ea84e74a6d06b92d05d67002439`. | `docs/generated/design/_meta/manifest.yaml` lines 505-519 defines the resolved watched set; `python3 docs/generated/design/_meta/regenerate.py digest name-resolution/visibility.md` returns `e44865f09117c2ed040c2745cf741834f8cd6ea84e74a6d06b92d05d67002439`. | Regenerate the front matter against the current manifest so `watched_paths_digest` records `e44865f09117c2ed040c2745cf741834f8cd6ea84e74a6d06b92d05d67002439`. |
| F-002 | minor | `## Concepts`, lines 64-82 | The page says `ModuleDecl::languageVersion` is set from the module declaration's version or a linkage default. A `module` declaration has no version; the compile request initializes the field from `CompilerOptionName::LanguageVersion`, and parsing selected modern constructs upgrades legacy to 2025. | `source/slang/slang-compile-request.cpp` lines 324-339 assigns the option-set version; `source/slang/slang-parser.cpp` lines 1221-1226 and 1376-1405 show the legacy upgrade and the versionless `module` syntax. | Describe the option-set initialization and parser upgrade accurately; add these two files to `watched_paths` if this lifecycle detail remains. |
| F-003 | minor | `### Per-keyword semantics`, lines 118-120 | The text implies every `GenericTypeConstraintDecl` inherits the generic inner declaration's visibility. `getDeclVisibility` does so only when the constraint's parent is a `GenericDecl`; otherwise it returns `Default`. | `source/slang/slang-check-decl.cpp` lines 21258-21265 checks `as<GenericDecl>(decl->parentDecl)` and returns `Default` when it fails. | Qualify the rule as applying to generic parameters and constraints owned by a `GenericDecl`, and state that other `GenericTypeConstraintDecl` parents fall back to `Default`. |
| F-004 | minor | `### Container-level cap`, lines 241-256 | The phrase “any generic type arguments” overstates `_getTypeVisibility`: it only recurses into arguments that cast to `DeclRefType`, not every type-valued generic argument. | `source/slang/slang-check-expr.cpp` lines 1114-1120 guards recursion with `as<DeclRefType>(arg)`. | Replace “any generic type arguments” with “declaration-reference type arguments,” or explain the exact operand shapes the helper traverses. |
| F-005 | minor | `### Container-level cap`, lines 258-266 | The page says `checkVisibility` enforces that any declaration cannot exceed its parent container's visibility. For an aggregate declaration, the parent search starts at the declaration itself and immediately compares its visibility with itself; the stated parent-cap behavior applies to non-aggregate members. | `source/slang/slang-check-modifier.cpp` lines 2375-2388 initializes `parentDecl = decl` and stops at the first `AggTypeDeclBase`; `source/slang/slang-check-decl.cpp` lines 3100-3109 calls this function for structs and classes. | Narrow the claim to non-aggregate members, and avoid promising a parent-container check for aggregate declarations unless the implementation changes. |
| F-006 | minor | `## Edge cases and failure modes`, lines 444-451 | The page says a `using` declaration only brings a namespace into scope, but the checker accepts any `NamespaceDeclBase`, including a module. | `source/slang/slang-check-decl.cpp` lines 17332-17338 explicitly says “namespace (or a module, since modules are namespace-like),” and lines 17360-17376 tests `NamespaceDeclBase`. | Say that `using` accepts namespace-like targets, including modules, while still rejecting individual declarations. |

## No-issues notes
- All cited modifier declarations, enum values, source line ranges, diagnostic names, and diagnostic codes match the recorded commit.
- The lookup and overload visibility paths are correctly shown as separate branches that share `isDeclVisibleFromScope`.
- The required rules, edge cases, Mermaid flowchart, and `## See also` links satisfy the prompt contract.
