---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T13:38:00Z
target_doc: glossary.md
review_report: ../reviews/glossary.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 6
  rejected_bogus: 0
  rejected_out_of_scope: 0
  deferred: 0
  escalated: 0
---

# Remediation report for glossary.md

## Summary

All six findings were verified against source at
`53b76e6d3009b8e6434d41573524c7ce5c499d23` and all six were fixed. The major
item — the `syntax-decl` entry attributing `if`, `for`, `struct`, and `__init`
to `core.meta.slang` — was confirmed wrong at the parser and rewritten around
the actual split between direct lookaheads and the C++ `g_parseSyntaxEntries`
table. The four minor items (`decoration`, `IRBuilder`, `mandatory optimization
pass`, `no producer at HEAD`) and the entry-length nit were likewise confirmed
and corrected. Nothing was rejected, deferred, or escalated; the page remains
far under its 102,400-byte cap at 52,332 bytes.

## Actions

| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Confirmed in `source/slang/slang-parser.cpp`: `struct` is a `LookAheadToken` test in the type-specifier path, `if` and `for` are `LookAheadToken` tests in the statement parser, `__init` is a `_makeParseDecl` row in the C++ `g_parseSyntaxEntries` table, and `populateBaseLanguageModule` installs that table via `addBuiltinSyntaxImpl`. None of it comes from `core.meta.slang`. | `syntax-decl`: replaced the `core.meta.slang` attribution with the closed-grammar-lookahead versus `SyntaxParseInfo`-registration split and named `populateBaseLanguageModule`. |
| F-002 | fixed | Confirmed in `source/slang/slang-ir-insts.lua`: the `Decoration` group contains `branch`, `loopControl`, `glslFragDepthGreater`, and `public`, whose `struct_name` values are `BranchDecoration`, `LoopControlDecoration`, `GLSLFragDepthGreaterDecoration`, and `PublicDecoration`. The suffix convention is on the wrapper, not the opcode. | `decoration`: replaced "(every opcode name ends in `Decoration`)" with a clause distinguishing wrapper naming from opcode naming, citing three counterexample opcodes. |
| F-003 | fixed | Confirmed: `addHoistableInst` in `source/slang/slang-ir.cpp` only *starts* at `getModuleInst()` and then folds in each operand's parent and the result type's parent through `mergeCandidateParentsForHoistableInst`, so a dependent instruction lands in a deeper parent. | `IRBuilder`: changed "routes hoistable instructions to module scope automatically" to hoisting as far toward module scope as operand and result-type visibility permit, naming `addHoistableInst`. |
| F-004 | fixed | Confirmed in `source/slang/slang-lower-to-ir.cpp`: `insertDebugValueStore` is guarded by `context->debugInfoLevel >= DebugInfoLevel::Standard`, and `simplifyCFG` / `peepholeOptimize` by `!minimumOptimizations` from `shouldPerformMinimumOptimizations()`; `invertLoops` is likewise conditional. "Runs unconditionally" contradicted the entry's own list. | `mandatory optimization pass`: redefined the term as the target-independent pre-link region, then separated its unconditional core from the three option-gated members and named their gates. |
| F-005 | fixed | Confirmed volatile and unsupported: `docs/generated/design/ir-reference/index.md` is 161 lines at HEAD and states the column contract without enumerating which pages carry no-producer rows, so the glossary's "seven of the ten" count has no current source. | `no producer at HEAD`: dropped the page count and the "six differentiation opcodes" tally, pointing at `ir-reference/index.md` for the contract and the pages instead. |
| F-006 | fixed | Confirmed against `docs/generated/design/_meta/prompts/glossary.md` lines 66-70, which require one short paragraph of 2-4 sentences per entry: `entry point` had five sentences, `lower-to-IR` one, `terminator instruction` six. | `entry point`: merged two sentences (now four). `lower-to-IR`: added a second sentence on the per-`TranslationUnitRequest` invocation. `terminator instruction`: merged the branch-argument and `yield` sentences and dropped the trailing catalog pointer already carried by the `See:` line (now four). |

## Note for the operator: watched-path coverage

Two facts this page now states are owned by files outside its `watched_paths`:
`source/slang/slang-parser.cpp` (the `syntax-decl` lookahead/`g_parseSyntaxEntries`
split, whose link was already on the page) and `source/slang/slang-ir.cpp` (the
`addHoistableInst` hoisting rule; the entry still links only the watched
`slang-ir.h`). No line numbers are cited for either, so nothing here will rot
silently into a wrong line reference, but a change to those files will not mark
the glossary stale. Adding both paths to the manifest entry would close the gap.
