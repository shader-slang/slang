---
remediation_report: true
remediator_model: claude-opus-5
remediated_at: 2026-08-04T14:30:00Z
target_doc: ir-reference/generics-and-existentials.md
review_report: ../../reviews/ir-reference/generics-and-existentials.md.review.md
target_doc_source_commit_before: 53b76e6d3009b8e6434d41573524c7ce5c499d23
target_doc_source_commit_after: 53b76e6d3009b8e6434d41573524c7ce5c499d23
actions:
  fixed: 5
  rejected_bogus: 0
  rejected_out_of_scope: 1
  deferred: 0
  escalated: 0
---

# Remediation report for ir-reference/generics-and-existentials.md

## Summary
Five findings were fixed and one was rejected as out of scope, so the document
was edited and `mark-fresh` is needed. The producer-column conversion is
complete: 39 rows were rewritten — 33 retired cells replaced (26 with a named
producing pass, 7 with **no producer at HEAD**) and 6 more had a leftover
`(synthesized) —` prefix stripped from an origin that already named its
producer. No `(synthesized)` or bare `—` remains in the `AST origin` column.
The no-producer inventory grew from four to seven because
`UnboundedGenericElement` (the reviewer's F-003) and the two dispatcher opcodes
found during the F-001 sweep are also uncalled.

## Actions
| Finding ID | Action | Rationale | Fix summary |
| --- | --- | --- | --- |
| F-001 | fixed | Every producer was traced, not guessed. The type-flow clusters are built by the type-flow specialization pass: `source/slang/slang-ir-typeflow-specialize.cpp` builds the four set opcodes through `IRBuilder::getSingletonSet` / `getSet` (`:728`, `:746-780`, `:1601`), the seven set-element opcodes (`:913`, `:920`, `:921`, `:925`, `:2417`, `:3245`, `:3397`, `:3399`, `:3601`, `:3652`, `:7944`), the tagged-union group (`:5910`, `:5934`, `:5995`, `:7185`, `:7301`, `:7360`, `:7565`, `:7650`, `:7756`, `:7950`, `:8017`), the tag-conversion group (`:5846`, `:7461`, `:7523`, `:7282`, `:7879`, `:6518`, `:7163`, `:7906`, `:8020`), `SpecializeExistentialsInFunc` (`:4219`, `:6589`) and `WeakUse` via `IRBuilder::getWeakUse` (`:1383`, `:1411`). `GetTagForSuperSet` and one `GetTypeTagFromTaggedUnion` site are in `source/slang/slang-ir-typeflow-set.cpp:159` and `:134`; `GetTagFromSequentialID` / `GetSequentialIDFromTag` are additionally built by dynamic-dispatch lowering (`source/slang/slang-ir-lower-dynamic-dispatch-insts.cpp:1005`, `:1009`, `:1135`, `:1180`); `GetElementFromTag` and `SpecializeExistentialsInType` come from `source/slang/slang-ir-specialize.cpp:3721` and `:3061`. The four bare-`—` rows are the page's own unproduced opcodes and became **no producer at HEAD**. Two further opcodes turned out to be unproduced: `IRBuilder::emitGetDispatcher` (`source/slang/slang-ir-insts.h:4526`) and `emitGetSpecializedDispatcher` (`:4547`) have no caller anywhere in `source/` — the only other references are the consumer declaration `lowerGetSpecializedDispatcher` in `slang-ir-lower-dynamic-dispatch-insts.h:34` and a dump case in `slang-ir.cpp:9668`. Converted 33 of 33 retired cells plus 6 prefix strips; 0 remain. | 33 `AST origin` cells rewritten (26 named producers, 7 **no producer at HEAD**); 6 cells of the form `(synthesized) — <producer>` had the dead prefix removed; the two dispatcher summaries now state their uncalled emitters. |
| F-002 | rejected-out-of-scope | Correct but not a remediator edit. `docs/generated/design/_meta/prompts/_remediate.md:97-100` reserves `generated_at`, `source_commit`, and `watched_paths_digest` for the operator's `regenerate.py mark-fresh` run and forbids the remediator editing them. This page was edited, so `mark-fresh` will record the current digest. | — |
| F-003 | fixed | Confirmed. `IRBuilder::getUnboundedGenericElement` at `source/slang/slang-ir-insts.h:4610-4613` is the only construction path and has no caller; the remaining references are classification cases at `slang-ir-insts.h:2983` and `:2999`, a consumer check at `slang-ir-typeflow-specialize.cpp:4054`, and a dump case at `slang-ir.cpp:9691`. That matches the page's own criterion for its other unproduced opcodes. | `UnboundedGenericElement` row marked **no producer at HEAD** with the uncalled-emitter explanation in its summary; the `## Source` inventory went from four to seven opcodes (also covering the two dispatchers found under F-001). |
| F-004 | fixed | Confirmed against `docs/generated/design/_meta/prompts/ir-reference-generics-and-existentials.md:70-73`, which lists "Specialization pass details (the `slang-ir-specialize.cpp` pass)" as forbidden content, reinforced by `_common.md:269-271`. Naming the producing pass in the `AST origin` column is still required by the column contract, so only the behavioural descriptions were trimmed. | The `lookupWitness` callout no longer describes when the specialization pass rewrites the lookup; it states the opcode-level fact that the lookup is a first-class unevaluated value and links `../pipeline/05-ir-passes.md`. Deleted the sentence in the `specialize` callout about specialization replacing each application with the generic's `return_val` result. |
| F-005 | fixed | Confirmed against `docs/generated/design/_meta/prompts/_common.md:65-66`, which requires the first body paragraph to say both what the document covers and who it is for. | Merged the intended-reader sentence into the end of the opening paragraph, unchanged apart from the sentence join. |
| F-006 | fixed | Confirmed by inspection of the `interface_req_entry` row: "An associated-type bound such as an associated-type bound such as `associatedtype A : IBar`". | Removed the duplicated clause. |
