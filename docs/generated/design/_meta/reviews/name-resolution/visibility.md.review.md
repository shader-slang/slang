---
review_report: true
reviewer_model: gpt-6
reviewed_at: 2026-11-03T06:41:36Z
target_doc: name-resolution/visibility.md
target_doc_source_commit: 2518a942d59b1228fc8cc70a37739baab82b6002
target_doc_watched_paths_digest: a39bda949a6aee96678d87904efb315ace2ed593bf5e94a26786facff04438e6
source_commit: c63c2fbd5115a98b2b2469f43618671653d40bf1
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 1
  major: 0
  minor: 2
  nit: 0
---

# Review report for name-resolution/visibility.md

## Summary

The document substantially matches the implementation, and its line-number citations are accurate. The most important correction is that private access between extensions requires matching target types or successful generic specialization; sharing a generic type declaration alone is insufficient. Two additional findings concern an overly broad modifier description and a missing watched path.

## Items checked

- Reviewed against the **current working tree**, which equals the document’s source commit for `source/`, as specified. The comparison also showed no changes to `include/slang.h`.
- Read `_review.md`, `_common.md`, and the document-specific prompt; obtained the watched paths and dependencies with `regenerate.py show name-resolution/visibility.md`.
- Spot-checked more than 20 behavioral claims, including keyword classification, generic parameters, accessor inheritance, language-version defaults, local-variable validation, lookup filtering, overload checks, private extension access, type visibility, import reachability, synthesized constructors, differential members, and derivative aliases.
- Checked every explicit line-number citation in the body against the corresponding working-tree source. No misplaced line citations were found.
- Resolved all 61 Markdown link occurrences, covering 22 distinct targets; verified target existence at the recorded source commit and manifest membership for all seven linked generated pages.
- Consulted the three dependency documents only for cross-reference checks.
- Checked required sections, their ordering, the flowchart, front-matter fields, relative-link syntax, and Liquid safety. The document occupies 32,639 bytes against its 32,768-byte cap.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | critical | `## Edge cases and failure modes`, lines 558–568 | The statement that a private extension member “is reachable from an extension on a different generic instantiation of the same type” omits the required applicability condition. For example, an extension targeting `G<int>` does not gain access to private members of an extension targeting `G<float>` merely because both target `G`. | `source/slang/slang-check-expr.cpp:1281–1289` accepts target equality or successful `applyExtensionToType`. In `source/slang/slang-check-decl.cpp:18555–18645`, that helper infers arguments for generic extensions and finally rejects mismatched targets: `if (!type->equals(targetType)) return DeclRef<ExtensionDecl>();`. | State that access succeeds when target types match, or when the candidate’s generic extension can specialize successfully to the requesting container’s type. Explicitly exclude distinct concrete specializations such as `G<int>` and `G<float>`; qualify the related cross-instantiation wording at lines 308–316. |
| F-002 | minor | `## Concepts`, lines 95–100; `### Interaction with IgnoreForLookupModifier`; lines 618–621 | Describing `IgnoreForLookupModifier` as hiding any marked declaration from lookup entirely overgeneralizes its implementation. The lookup consumer skips inheritance facets whose declared subtype witness references a marked inheritance declaration; this is not a general declaration filter. | `source/slang/slang-ast-modifier.h:276–279` documents an `InheritanceDecl` modifier. `source/slang/slang-lookup.cpp:450–462` explicitly describes the limited handling and checks the inheritance witness. `DeclPassesLookupMask` at lines 40–98 contains no general check for this modifier. | Describe the implemented behavior as excluding the enum tag-type inheritance facet from member lookup. Remove blanket claims that attaching this modifier to any declaration makes it invisible. |
| F-003 | minor | `### Per-keyword semantics`, lines 156–160 | The document cites and describes `getParentAggTypeDecl` in `slang-syntax.cpp`, but that file is absent from this page’s watched paths. This violates the name-resolution family’s scope rule and leaves this explanation outside freshness tracking. | `source/slang/slang-syntax.cpp:1247–1256` owns the cited helper. The visibility entry in `docs/generated/design/_meta/manifest.yaml:689–714` omits this file; `_common.md` requires expanding watched paths when citing outside them and noting the expansion. | Add `source/slang/slang-syntax.cpp` to this document’s watched paths and briefly note that it supplies the ancestor-walk helper used by visibility classification. |