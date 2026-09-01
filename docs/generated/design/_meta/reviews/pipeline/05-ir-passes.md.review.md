---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:10:09+00:00
target_doc: pipeline/05-ir-passes.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 66d53e154eb63196d1c2ca1ee3a1c040e363e14dfbc86e6fad3f9c19cd0f9d21
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: fail
  cross_references: pass
  completeness: pass
  style_consistency: pass
  source_alignment: fail
  front_matter_validity: pass
finding_count: 3
severity_breakdown:
  critical: 0
  major: 2
  minor: 1
  nit: 0
---

# Review report for pipeline/05-ir-passes.md

## Summary

The document satisfies its structure and style contracts, all 27 explicit line-number citations are accurate, and its links and front matter validate at the recorded commit. Three factual issues remain: one watched-path note was not updated after the manifest changed, and four pass-table purpose cells misdescribe their implementations. The most important errors label call specialization as array-generic specialization or tagged-union defunctionalization.

## Items checked

- Read `_review.md`, `_common.md`, the per-document prompt, the target document, dependency `pipeline/04-ast-to-ir.md`, and the resolved watched-file list from `regenerate.py show`.
- Verified all 27 explicit line-number and line-range citations against `source/slang/slang-emit.cpp` at `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Confirmed 162 `slang-ir-*.cpp` files and 180 `SLANG_PASS(...)` call sites in `linkAndOptimizeIR` (181 textual macro occurrences including the macro definition).
- Spot-checked more than 18 factual claims, including all four orchestrator callers, both lowering-set scans, HostVM's early return, SPIR-V legalization order, layout-module linking, specialization options, autodiff cleanup, non-uniform-index modes, coverage counter behavior, metadata collection, SSA simplification, register allocation, DLL wrappers, and representative table purposes.
- Ran the generated-doc linter, resolved the relative links at the recorded commit, and verified the generated peer targets exist in the manifest.
- Verified the required sections, 65,536-byte cap (document size 44,836 bytes), universal style rules, mandatory front matter, and the historical 325-file watched-path digest (`66d53e154eb63196d1c2ca1ee3a1c040e363e14dfbc86e6fad3f9c19cd0f9d21`).

## Findings

| ID    | Severity | Location                                                                                                  | Description                                                                                                                                                                                                                                                                                                                                                                                                                                                         | Evidence                                                                                                                                                                                        | Recommendation                                                                                                                                                                                                                                             |
| ----- | -------- | --------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | minor    | `## How the passes are ordered`, lines 101-107                                                            | The note says `source/slang/slang-emit-spirv.cpp` falls outside this page's watched paths and “should be added,” but the current manifest entry already watches that file. It also says SPIR-V legalization has no category row, although the `SPIR-V legalize` row appears at line 356.                                                                                                                                                                            | `docs/generated/design/_meta/manifest.yaml:196-207` includes `source/slang/slang-emit-spirv.cpp`; the target document's `SPIR-V legalize` row links `source/slang/slang-ir-spirv-legalize.cpp`. | Remove the SPIR-V half of this stale note. Retain only the actionable gap for `source/slang/slang-check-out-of-bound-access.cpp`, or add that source path to the manifest and remove the note entirely.                                                    |
| F-002 | major    | `### Specialization and generics`, `Specialize arrays` and `Defunctionalization` rows (lines 203 and 209) | Both purpose cells describe transformations these files do not implement. `slang-ir-specialize-arrays` specializes calls whose struct parameters contain array fields; it does not specialize “array-typed generic parameters.” The explicitly aspirational `slang-ir-defunctionalization` filename currently contains `specializeHigherOrderParameters`, which specializes calls receiving global functions; it does not convert function values to tagged unions. | `source/slang/slang-ir-specialize-arrays.h:9-20`; `source/slang/slang-ir-defunctionalization.h:1-16`; `source/slang/slang-ir-defunctionalization.cpp:22-33`.                                    | Rename the first purpose to call specialization for struct parameters with array fields. Rename the second pass/purpose to `Specialize higher-order parameters` and describe specializing calls that pass global functions; delete the tagged-union claim. |
| F-003 | major    | `### Layout and binding`, `Late require capability` and `String hash` rows (lines 328 and 331)            | These purpose cells also contradict their APIs. `processLateRequireCapabilityInsts` processes and eliminates late-require instructions while diagnosing unavailable capabilities; it does not add requirements. `slang-ir-string-hash` manages the global hashed-string-literal pool and verifies that `getStringHash` operands are string literals; it does not perform stable hashing “for symbols.”                                                              | `source/slang/slang-ir-late-require-capability.h:12-18`; `source/slang/slang-ir-string-hash.h:13-25`; `source/slang/slang-ir-string-hash.cpp:99-121`.                                           | Describe late capability processing as eliminating deferred checks and diagnosing missing capabilities. Describe string-hash handling as pooling hashed string literals and validating `getStringHash` operands.                                           |
