---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:07:22+00:00
target_doc: ast-reference/modifiers.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 00900db2297740a4e95ed1bd166180aeec693f1bbb14d06c300bedcc1eff4d63
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: fail
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: partial
finding_count: 8
severity_breakdown:
  critical: 0
  major: 4
  minor: 4
  nit: 0
---

# Review report for ast-reference/modifiers.md

## Summary
The page exhaustively covers the concrete modifier classes, and its links and sole line citation are valid. It still fails the AST-family table contract: the catalog is split across 26 tables, many parsed nodes are marked `(none)` in the Grammar column, and 15 Key fields cells are not in `name: Type` form. The front-matter digest also reflects the former three-file watched set rather than the seven files currently resolved by the manifest.

## Items checked
- Verified all seven resolved watched files and both dependency documents against source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`; review-time `HEAD` is the same commit.
- Compared all 273 FIDDLE-declared classes in `slang-ast-modifier.h` with the tables: all 264 concrete classes appear exactly once, while all nine abstract classes are excluded.
- Spot-checked more than 10 factual claims, including modifier inheritance, parser construction of `UncheckedAttribute`, scoped-name flattening, optional commas and double brackets, GLSL layout group markers, matrix-layout inversion, checked GLSL binding conversion, memory-qualifier aggregation, intrinsic parsing, work-graph fields and spellings, differentiability mappings, visibility defaults, and capability storage.
- Re-derived the sole source line citation: `DifferentiableAttribute` is exactly at `source/slang/slang-ast-modifier.h:1715-1762`.
- Resolved all 73 relative links (21 unique targets) at the recorded commit, verified every linked heading anchor, and confirmed every generated peer is present in the manifest.
- Swept 627 backticked identifier tokens and 127 bracketed attribute spellings against source, and separately verified all seven whole source-file names.
- Recomputed the current seven-file watched-path digest as `3390b4894a127a22ac54d3fd216aa54aa11f8c940d5c28b9bed1c58dcd6a4e5b`.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `## Nodes`, lines 126-533 | The contract requires one node table, but the page splits the 264 concrete classes across 26 separate tables. Coverage is exhaustive, but the required catalog shape is absent. | `docs/generated/design/_meta/prompts/_common.md:99-108` requires “a single table with one row per concrete” class. | Merge the 26 tables into one table with the required five columns; preserve category ordering without additional table headings. |
| F-002 | major | `## Nodes`, Grammar column | Many parsed modifiers and attributes are labeled `(none)`, which the prompt reserves for synthesized nodes. Examples include `ForceUnrollAttribute`, `MaxItersAttribute`, `GLSLBindingAttribute`, and `NodeLaunchAttribute`, despite explicit parser or `attribute_syntax` producers. | `source/slang/core.meta.slang:4381-4490` registers the first three examples; `source/standard-modules/experimental/workgraph.slang:17` registers `NodeLaunchAttribute`. The rule is in `docs/generated/design/_meta/prompts/ast-reference-modifiers.md:21-25`. | Audit every Grammar cell: link parsed keyword modifiers to `#modifiers`, parsed bracket attributes to `#attributes-and-decorations`, and retain `(none)` only for genuinely synthesized/internal nodes. |
| F-003 | major | `## Nodes`, Key fields column | Fifteen rows use untyped concepts such as `optional unroll count`, `opcode (in args)`, `binding`, and `max (in args)` instead of the mandatory `name: Type` form. The prose explicitly adopts this alternate convention, but the family contract does not allow it. | `docs/generated/design/_meta/prompts/_common.md:104-108` mandates `name: Type`; inherited parsed arguments are stored in `AttributeBase::args` at `source/slang/slang-ast-modifier.h:804-815`. | Replace all 15 conceptual cells with actual fields in `name: Type` form, using inherited `args: List<Expr*>` where no more specific stored field exists. |
| F-004 | major | `## Notable nodes`, lines 579-589 and 688-702 | The prompt requires the layout modifiers’ role in parameter binding, but the page only describes parser/checker representation and the absence of former class names. It never connects `GLSLBindingAttribute`’s binding/set pair or `HLSLRegisterSemantic`’s register/space tokens to that role. | `docs/generated/design/_meta/prompts/ast-reference-modifiers.md:47-55`; the relevant stored binding data is declared at `source/slang/slang-ast-modifier.h:481-495` and `:1080-1087`. | Extend the existing layout callout with a concise representation-level parameter-binding explanation using those fields, and defer semantic rules to the linked semantic-checking page. |
| F-005 | minor | `### TargetIntrinsicModifier and SpecializedForTargetModifier`, lines 569-577 | The page says the predicate applies “when a capability is in effect,” but capability selection comes from `targetToken`; the predicate and `scrutineeDeclRef` are a separate guard passed to the IR target-intrinsic decoration. | `source/slang/slang-ast-modifier.h:282-295`; `source/slang/slang-lower-to-ir.cpp:13368-13404` constructs target capabilities separately from the predicate/scrutinee pair. | State that `targetToken` selects target capabilities and that the optional predicate guards the intrinsic using its scrutinee declaration; remove the capability claim from the predicate sentence. |
| F-006 | minor | `## Source`, lines 47-58 | The page calls all four spelling files “core-module sources,” but `workgraph.slang` is an experimental standard module, not a core-module source. | The file is `source/standard-modules/experimental/workgraph.slang`; its attribute declarations are at lines 17-51. | Change “four core-module sources” to “four sources” and distinguish the three meta-module files from the experimental standard module. |
| F-007 | minor | `### Families named in the prompt but not present in the header`, lines 688-702 | The statements about what the page “was asked to cover,” prompt correction, and future regeneration are workflow commentary rather than source documentation. The factual absence of the three classes is useful, but the generation-history narrative violates the no-editorial-commentary rule. | `docs/generated/design/_meta/prompts/_common.md:74-81` forbids editorial commentary; current absence is established by the complete class inventory in `source/slang/slang-ast-modifier.h`. | Keep concise factual notes that the three classes do not exist and identify the real groupings inside the relevant notable callouts; remove prompt-history and future-regeneration prose. |
| F-008 | minor | Front matter, line 6 | `watched_paths_digest` is valid hex but stale: it is the digest of the former three-file watched set. The current manifest resolves seven files and `regenerate.py digest ast-reference/modifiers.md` returns `3390b4894a127a22ac54d3fd216aa54aa11f8c940d5c28b9bed1c58dcd6a4e5b`, not `00900d…`. | `docs/generated/design/_meta/manifest.yaml:415-429` lists the seven watched files; `docs/generated/design/_meta/prompts/_common.md:22-35` requires the current digest. | On the next permitted regeneration, replace the front-matter digest with the seven-file digest; do not edit the generated page during review. |

## No-issues notes
- Every concrete FIDDLE class is present exactly once, and no abstract FIDDLE class is incorrectly included in the node catalog.
- The parser claims about `[[a]]`, optional commas, and `::` name flattening match `source/slang/slang-parser.cpp:933-1047`.
- The sole line-number citation and all relative links resolve exactly at the recorded source commit.
