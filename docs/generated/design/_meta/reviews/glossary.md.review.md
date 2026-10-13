---
review_report: true
reviewer_model: gpt-5.6-sol
reviewed_at: 2026-08-04T12:08:25+00:00
target_doc: glossary.md
target_doc_source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_watched_paths_digest: 8033723409ecbf2551b9a4eb228a4e39356c3fa79164d7d057fb8526b4b0145a
source_commit: 53b76e6d3009b8e6434d41573524c7ce5c499d23
checklist:
  factual_accuracy: partial
  cross_references: pass
  completeness: pass
  style_consistency: partial
  source_alignment: partial
  front_matter_validity: pass
finding_count: 6
severity_breakdown:
  critical: 0
  major: 1
  minor: 4
  nit: 1
---

# Review report for glossary.md

## Summary
The glossary has complete structure, valid links, and mostly source-supported definitions, but six issues remain. Most importantly, the `syntax-decl` entry attributes several C++ parser mechanisms to `core.meta.slang`, which contradicts the parser source.

## Items checked
- Reviewed `_review.md`, `_common.md`, the glossary prompt, all resolved watched paths, and all six dependencies from `regenerate.py show glossary.md`.
- Validated all 228 relative-link occurrences (92 unique targets) at `53b76e6d3009b8e6434d41573524c7ce5c499d23`; none was missing.
- Verified all nine cited source-line locations: `DeclCheckState`, `FrontEndEntryPointRequest`, `Linkage`, both magic-type modifiers, `calcRequiredLoweringPassSet`, `Session`, `linkAndOptimizeIR`, and `TranslationUnitRequest`.
- Spot-checked more than 20 factual claims, including AST allocation, block parameters, conversion costs, capability aliases, declaration checking, entry points, prelinking, lookup, serialization, target legalization, and lowering-pass gates.
- Swept 248 likely backticked identifiers and 46 source filenames against the recorded source tree.
- Confirmed all 72 entries are alphabetized, singly tagged, and have a resolving `See:` link; the cross-reference index covers every watched generated peer.

## Findings

| ID | Severity | Location | Description | Evidence | Recommendation |
| --- | --- | --- | --- | --- | --- |
| F-001 | major | `syntax-decl`, lines 773-779 | The entry says most keywords, including `if`, `for`, `struct`, and `__init`, are bound as syntax-decls in `core.meta.slang`. In fact, `if` and `for` are direct statement-parser lookaheads, `struct` is a direct type-specifier lookahead, and `__init` is registered by the C++ `g_parseSyntaxEntries` table; C++ installs that table into the base-language module. | `source/slang/slang-parser.cpp:3425-3429`, `source/slang/slang-parser.cpp:6921-6933`, `source/slang/slang-parser.cpp:10700-10714`, and `source/slang/slang-parser.cpp:10881-10890`. | Replace the `core.meta.slang` attribution with the actual split: direct parser lookaheads for closed grammar and C++ `SyntaxParseInfo` registration for extensible syntax. |
| F-002 | minor | `decoration`, lines 168-176 | The claim that every opcode in the `Decoration` family ends in `Decoration` confuses opcode names with C++ wrapper names. Family members include `branch`, `loopControl`, `glslFragDepthGreater`, and `public`; their wrappers carry the suffix. | `source/slang/slang-ir-insts.lua:1752-1772` and `source/slang/slang-ir-insts.lua:1932-1937`. | Say that decoration wrapper structs conventionally end in `Decoration`, while opcode names do not uniformly do so. |
| F-003 | minor | `IRBuilder`, lines 313-317 | The entry says `IRBuilder` routes hoistable instructions to module scope automatically. Module scope is only the initial candidate; operands or the result type can force insertion into a deeper parent. | `source/slang/slang-ir.cpp:1788-1818` starts at the module and merges operand/type parents to find the legal insertion parent. | Change “to module scope” to “as far toward module scope as operand visibility permits.” |
| F-004 | minor | `mandatory optimization pass`, lines 487-503 | The opening definition says the pass runs unconditionally, but the listed sequence includes gated work: debug-value insertion, CFG/peephole simplification, and loop inversion. | `source/slang/slang-lower-to-ir.cpp:15565-15586` and `source/slang/slang-lower-to-ir.cpp:15615-15618`. | Define this as the target-independent pre-link mandatory-processing region, then distinguish its unconditional core from option-gated passes. |
| F-005 | minor | `no producer at HEAD`, lines 538-552 | The entry says seven of ten IR-reference content pages carry such rows. At the recorded source commit, the index instead says pages use `(synthesized)` or `—`; the current dependency enumerates only four pages with no-producer entries. | `docs/generated/design/ir-reference/index.md:115-119` at `target_doc_source_commit`; current `docs/generated/design/ir-reference/index.md:195-203`. | Omit the volatile page count and link to the index, which can enumerate the current pages. |
| F-006 | nit | Entry shape, lines 226-240, 463-468, and 828-844 | The prompt requires 2-4 sentences per entry, but `entry point` has five, `lower-to-IR` has one, and `terminator instruction` has six. | `docs/generated/design/_meta/prompts/glossary.md:64-70`. | Trim or split those definitions so each remains a 2-4 sentence lookup entry. |
