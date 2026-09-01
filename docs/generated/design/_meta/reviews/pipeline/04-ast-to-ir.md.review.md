---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:08:18+00:00
target_doc: pipeline/04-ast-to-ir.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: bfaba4260e5950b0732424070a24791f1265e6debdda1d5c6493fbe0abe1e140
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: partial
finding_count: 3
severity_breakdown:
  critical: 0
  major: 1
  minor: 2
  nit: 0
---

# Review report for pipeline/04-ast-to-ir.md

## Summary

The document satisfies the required structure and style contract, all links resolve, all 42 explicit line-number references are accurate, and the large majority of checked source claims are supported. The most important finding is that its recorded watched-path digest no longer matches the seven files resolved by the current manifest, which prevents the front matter from identifying the source set actually under review. Two smaller source-alignment issues concern an AST mutation omitted from the claimed output boundary and an overly narrow description of the separate layout module.

## Items checked

- Read `_review.md`, `_common.md`, the per-document prompt, the target, and both dependency documents; resolved all seven watched files with `regenerate.py show pipeline/04-ast-to-ir.md`.
- Confirmed the watched source files have no working-tree differences from recorded source commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Spot-checked more than 30 factual claims, including the driver signatures, lowering visitors and caches, `IRBuilder` flags, AST mappings, builtin operators, witness lowering, generic requirement handling, diagnostics, layout generation, entry-point decorations, and debug-info behavior.
- Re-derived all 42 explicit line-number references and the cited range endpoints; every cited number points at the claimed symbol or behavior.
- Resolved all 34 relative-link occurrences and confirmed the referenced generated peers are present in the manifest; `regenerate.py lint pipeline/04-ast-to-ir.md` completed without structural or link errors.
- Confirmed the required sections and size cap, and recomputed the watched-path digest; the mismatch is reported below.

## Findings

| ID    | Severity | Location                                                                                      | Description                                                                                                                                                                                                                                                                                                                               | Evidence                                                                                                                                                                                                                                                                                                                                            | Recommendation                                                                                                                                                                                                                                                               |
| ----- | -------- | --------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | major    | Front matter, line 6                                                                          | `watched_paths_digest` is recorded as `bfaba426...`, but the current manifest resolves seven watched files and `regenerate.py digest pipeline/04-ast-to-ir.md` produces `b5b08783...` against the unchanged source snapshot. The front matter therefore does not identify the watched source set currently associated with this document. | `docs/generated/design/_meta/freshness.json:219-223` records the authoritative digest `b5b087832fec0e249e88a0d170e419896ef6e42e51a41f7730860c109aee1cd2`; the target front matter records `bfaba4260e5950b0732424070a24791f1265e6debdda1d5c6493fbe0abe1e140`. `git diff` confirms the seven watched source files match source commit `53b76e6d...`. | Refresh the target through the generated-doc workflow so its front matter records `b5b087832fec0e249e88a0d170e419896ef6e42e51a41f7730860c109aee1cd2`, then ensure review metadata is regenerated for that digest before recording the review. Do not repair only the ledger. |
| F-002 | minor    | `## Module-level outputs`, lines 440-455; `### Entry-point-scoped decorations`, lines 468-476 | The absolute claim that `generateIRForTranslationUnit` has “no additional side artefacts” overlooks a persistent AST mutation: when a registered entry point has no explicit `EntryPointAttribute`, lowering creates one and attaches it to the AST function before lowering the function.                                                | `source/slang/slang-lower-to-ir.cpp:15215-15223` creates an `EntryPointAttribute`, fills its capability set from the entry-point profile, and calls `addModifier(entryPointFuncDecl, entryPointAttr)`.                                                                                                                                              | Qualify the output-boundary statement and mention that lowering may add an implicit entry-point attribute to the checked AST so ordinary function lowering recognizes the registered entry point; keep the returned `IRModule` as the sole separate output object.           |
| F-003 | minor    | `## Adjacent pipelines`, lines 537-544                                                        | The layout module is described as having “only” `IRLayoutDecoration`s on stub globals and entry points. It also decorates the module root, emits the supporting type/variable-layout instructions used by those decorations, and can attach `IRRequireCapabilityAtomDecoration`s to entry-point stubs.                                    | `source/slang/slang-lower-to-ir.cpp:16395-16450` creates imported global stubs plus type/variable layouts and decorates both globals and the module root; lines 16470-16498 create entry-point stubs, add capability-atom decorations, and add their layout decorations.                                                                            | Describe it as a layout-focused module containing imported global/entry-point stubs, module/global/entry-point layout decorations and their supporting layout metadata, plus relevant capability decorations; avoid saying those decorations are its only contents.          |
