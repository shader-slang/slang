---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T08:20:32+00:00
target_doc: name-resolution/scopes.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 2d1b0424ee67205473f3a56c4db750bce6b7847387448b1952bed261cc3cca46
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
  major: 1
  minor: 3
  nit: 0
---

# Review report for name-resolution/scopes.md

## Summary

The page is broadly source-aligned, mostly contract-conformant, and well linked, but its representative parser call-chain explanation misidentifies the helper used by nearly every scope in that chain. Three smaller issues concern an omitted concrete scope-carrying class and two unsupported descriptions of scope behavior.

## Items checked

- Read the target, `_common.md`, the per-document prompt, all nine resolved watched files, and the three dependency documents at the recorded source commit.
- Verified all 90 explicit line-number citation phrases and more than ten behavioral claims against commit `53b76e6d3009b8e6434d41573524c7ce5c499d23`.
- Resolved all 17 unique relative links at the recorded commit and confirmed every linked generated peer is present in the manifest.
- Swept 152 unique backticked terms and all nine source-like file names; qualified-member spellings accounted for the terms without exact whole-string matches.
- Checked the mandatory section order, required concepts/rules/edge cases, front-matter keys, 64-character digest, size cap, and lint result.

## Findings

| ID    | Severity | Location                                       | Description                                                                                                                                                                                                                                                                                                                                     | Evidence                                                                                                                                                                                                                                          | Recommendation                                                                                                                                                                                                                                                                                            |
| ----- | -------- | ---------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| F-001 | major    | `### Parser scope construction`, lines 167-172 | The claim that every function in the representative chain calls `pushScopeAndSetParent` is false. Most declaration scopes use `PushScope`; only the block scope in this chain uses `pushScopeAndSetParent`. This misstates the central construction mechanism a maintainer would need to modify.                                                | `source/slang/slang-parser.cpp:1787-1797` (`parseOptGenericDecl`) and `:5088-5123` (`parseFuncDecl`) call `PushScope`; `:6284-6292` (`parseDeclBody`) also calls `PushScope`; `:7139-7142` (`parseBlockStatement`) calls `pushScopeAndSetParent`. | Replace the final sentence with a step-by-step distinction: declaration parsers use `PushScope` and establish declaration parents through their surrounding construction paths, while statement-created `ScopeDecl`s such as blocks use `pushScopeAndSetParent`; all paths pair the push with `PopScope`. |
| F-002 | minor    | `### Scope-bearing AST nodes`, lines 84-104    | The required concrete-class inventory omits `ThisTypeDecl`, a concrete `AggTypeDecl` descendant and therefore a `ContainerDecl` carrying `ownedScope`. The prompt requires every concrete class in the watched headers that carries `ownedScope` or `ScopeStmt::scopeDecl` to be named.                                                         | `source/slang/slang-ast-decl.h:478-481` declares `ThisTypeDecl : AggTypeDecl`; `AggTypeDeclBase : ContainerDecl` is at `:359-363`, and `ContainerDecl::ownedScope` is at `:132-141`.                                                              | Add `ThisTypeDecl` to the aggregate row, identify it as the synthesized interface `This` container, and state whether its `ownedScope` is populated on the construction paths covered by the watched files.                                                                                               |
| F-003 | minor    | `UsingDecl` edge case, lines 357-359           | The statement that the parse-time scope and eventual injection scope “may differ” is unsupported and contradicted by the shown implementation: checking always passes the same captured `decl->scope` pointer as the destination. Scope wiring may mutate that scope's sibling chain, but it does not substitute a different destination scope. | `source/slang/slang-parser.cpp:4548-4552` stores `parser->currentScope` in `decl->scope`; `source/slang/slang-check-decl.cpp:17348-17376` passes `decl->scope` unchanged to `addAllSiblingScopesFromDecl`.                                        | Delete the “may differ” sentence. If the intended point is later wiring, say that checking augments the captured scope's `nextSibling` chain after namespace resolution.                                                                                                                                  |
| F-004 | minor    | `Empty block scope`, lines 297-306             | The cited `hiddenFromLookup` mechanism does not explain why an empty block's fresh scope matters: an empty block receives an `EmptyStmt`, and `visitBlockStmt` only sets `hiddenFromLookup` on declaration statements in a `SeqStmt`. No per-declaration flag exists in the stated empty-block case.                                            | `source/slang/slang-parser.cpp:7217-7223` creates an `EmptyStmt` when the block has no body; `source/slang/slang-check-stmt.cpp:82-118` sets `hiddenFromLookup` only while iterating `DeclStmt`s in a `SeqStmt`.                                  | Separate the two facts: state that the parser creates a scope uniformly even for an empty block, then explain `hiddenFromLookup` only for blocks that actually contain declaration statements.                                                                                                            |

## No-issues notes

- Every recorded line-number citation points to the stated declaration, helper, diagnostic call, or behavior.
- Sibling-scope wiring for files, imports, namespaces, and `using` declarations matches the four cited call-site families.
- The front-matter digest recomputes to the recorded value, and all relative links resolve at the target source commit.
