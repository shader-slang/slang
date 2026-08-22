---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:18:50+00:00
target_doc: pipeline/03-semantic-check.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: a244dfa19ecf6d79ea826d9b14c775491f6a2445e1ddbc0c633a710605a2aec3
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: partial
  style_consistency: pass
  source_alignment: partial
  front_matter_validity: pass
finding_count: 4
severity_breakdown:
  critical: 0
  major: 0
  minor: 4
  nit: 0
---

# Review report for pipeline/03-semantic-check.md

## Summary

The page is structurally complete, all links resolve, and most implementation claims agree with the source at the recorded commit. Four minor findings remain: two source-ownership descriptions are inaccurate, the generic section exceeds its overview-only contract, the binding-modifier gating description overgeneralizes one gate to four diagnostics, and the closing statement invents an errored declaration state.

## Items checked

- Verified the front matter and recomputed the watched-path digest; it matches `a244dfa19ecf6d79ea826d9b14c775491f6a2445e1ddbc0c633a710605a2aec3`.
- Verified every relative link target at `53b76e6d3009b8e6434d41573524c7ce5c499d23` and confirmed all generated-document references are manifest entries.
- Verified all 11 line-number citation passages, covering 15 cited source line numbers.
- Spot-checked more than 25 factual claims, including checker orchestration, deferred body parsing, constraint solving, witness lookup, inheritance linearization, differentiability synthesis, shader checks, and diagnostic recovery.
- Checked every required section and confirmed every watched `slang-check-*.cpp` file is mentioned.

## Findings

| ID    | Severity | Location                                                   | Description                                                                                                                                                                                                                                                                                                                                                                                                                       | Evidence                                                                                                                                                                                                                                                    | Recommendation                                                                                                                                                        |
| ----- | -------- | ---------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | minor    | `## SemanticsVisitor`, lines 65-68                         | Two responsibility rows misstate source ownership: `slang-check-inheritance.cpp` is said to own “member visibility,” and `slang-check-resolve-val.cpp` is said to validate substitutions. The former implements inheritance and extension-facet computation; visibility filtering is implemented in expression/overload checking. The latter resolves and canonicalizes types and decl refs rather than validating substitutions. | `source/slang/slang-check-inheritance.cpp:139-207` and `source/slang/slang-check-inheritance.cpp:608-615`; `source/slang/slang-check-expr.cpp:1136-1275`; `source/slang/slang-check-overload.cpp:272-278`; `source/slang/slang-check-resolve-val.cpp:1-59`. | Remove “member visibility” from the inheritance row, and describe `slang-check-resolve-val.cpp` as resolving/canonicalizing `Type`, `DeclRef`, and witness values.    |
| F-002 | minor    | `## Generic specialization and constraints`, lines 101-247 | The prompt requires an overview only, but this section devotes roughly 140 lines to solver fallback mechanics, associated-type constraint representation, inheritance-cache cycle handling, generic-inference failure payloads, and differentiability synthesis internals.                                                                                                                                                        | `docs/generated/design/_meta/prompts/pipeline-03-semantic-check.md:42-44` says “overview only”; the target's detailed implementation treatment spans lines 113-247.                                                                                         | Condense this section to a short architectural overview and retain links to `docs/design/interfaces.md` and the relevant source files for details.                    |
| F-003 | minor    | `## Shader-specific checks`, lines 329-338                 | The text says all four ignored binding modifiers are gated by `_allTargetsSupportVkBindingOnEntryPointParameters` and `isVkBindingCompatibleEntryPointParameterType`. Those predicates gate only `[[vk::binding(...)]]`; `[[vk::push_constant]]`, `register()`, and `packoffset()` are diagnosed unconditionally when present on an entry-point parameter.                                                                        | `source/slang/slang-check-shader.cpp:2331-2366` applies `supportsVkBindingOnParameter` only in the `GLSLBindingAttribute` branch, followed by three unconditional modifier branches.                                                                        | Restrict the gate explanation to `[[vk::binding(...)]]` and state separately that the other three modifiers are always diagnosed in this entry-point-parameter check. |
| F-004 | minor    | `## Failure modes`, lines 383-385                          | The claim that every declaration is “either fully checked or marked errored” describes a state the checker does not have. `checkModule` drives every declaration through the ordinary check-state sequence, including `DefinitionChecked` and `CapabilityChecked`; errors are represented by diagnostics and error-valued AST nodes, not an alternate errored declaration state.                                                  | `source/slang/slang-check-decl.cpp:5187-5246` and `source/slang/slang-check-decl.cpp:5298-5322`; `source/slang/slang-ast-support-types.h:474-580`.                                                                                                          | Say that `checkModule` drives declarations through `CapabilityChecked`, while recovery records diagnostics and substitutes error types/expressions where needed.      |

## No-issues notes

- Every recorded line citation points at the named symbol or declaration.
- Deferred `UnparsedStmt` handling and parser callbacks match `maybeParseStmt` and `parseUnparsedStmt`.
- The detailed constraint-solver, inheritance-cycle, differentiability, and generic-entry-point claims were source-supported despite the section-scope finding.
