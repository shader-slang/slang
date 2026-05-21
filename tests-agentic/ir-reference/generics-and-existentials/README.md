---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:13:27Z
source_commit: ecefa0388fc4ccf7d14670c7bf1eccc88a7bdd14
watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_doc: docs/llm-generated/ir-reference/generics-and-existentials.md
source_doc_digest: d5381c8e732782fb3c7c867a0de12b1ae29edbcede8a5cb37f8db47276e38b43
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/generics-and-existentials

## Intent

Tests verify the per-opcode catalog of the IR generics-and-existentials
family described in
[`docs/llm-generated/ir-reference/generics-and-existentials.md`](../../../docs/llm-generated/ir-reference/generics-and-existentials.md):
that each opcode the doc names with a non-`(synthesized)` AST origin
(`specialize`, `lookupWitness`, `makeExistential`,
`extractExistentialType`, `extractExistentialValue`,
`extractExistentialWitnessTable`, `witness_table`,
`witness_table_entry`, `interface_req_entry`, `global_generic_param`)
appears in `-dump-ir` output for the obvious AST surface that
produces it, with the operand-shape claim from the doc.

The primary observation mechanism is `-target spirv-asm -dump-ir -o
/dev/null -entry main -stage compute` followed by a FileCheck
against the first `### LOWER-TO-IR:` block of the dump. Anchors are
the opcode name itself (`makeExistential(`, `lookupWitness(`,
`specialize(`, `witness_table_entry(`, `interface_req_entry(`,
`global_generic_param`) and user-named top-level symbols
(`func %main`, `struct %A`, `let %IFoo`, `witness_table %4`).

The angle distinguishing this bundle from `ir-reference/structure`
(which anchors `witness_table`, `witness_table_entry`,
`interface_req_entry`, and `lookupWitness` from the
parent-child structural shape side) is: the **dispatch /
operand-shape axis** — every test observes an operand list, a
result type, or a dispatch sequence in a basic block. The
structural bundle observes those same opcodes as parents and
children; this bundle observes them as IR values with operand
shapes.

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| Dispatching through an interface-typed value produces extractExistentialType(%i) whose result type is Type. | functional | [#existential-destructuring](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#existential-destructuring) | [`extract-existential-type.slang`](extract-existential-type.slang) |
| Dispatching through an interface-typed value produces extractExistentialValue(%i) reading the packed concrete-typed value. | functional | [#existential-destructuring](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#existential-destructuring) | [`extract-existential-value.slang`](extract-existential-value.slang) |
| Dispatching through an interface-typed value produces extractExistentialWitnessTable(%i) whose result type is witness_table_t(%I). | functional | [#existential-destructuring](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#existential-destructuring) | [`extract-existential-witness-table.slang`](extract-existential-witness-table.slang) |
| Interface dispatch through an existential follows the sequence extractExistentialType, extractExistentialValue, extractExistentialWitnessTable, lookupWitness, call. | functional | [#existential-destructuring](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#existential-destructuring) | [`existential-dispatch-sequence.slang`](existential-dispatch-sequence.slang) |
| A 5-deep nested Box<Box<Box<Box<Box<int>>>>> lowers to five nested specialize(%Box, ...) applications. | stress | [#generic-application](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#generic-application) | [`stress-recursive-generic-depth-five.slang`](stress-recursive-generic-depth-five.slang) |
| A module-scope type_param T declaration lowers to let %T : Type = global_generic_param. | functional | [#generic-application](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#generic-application) | [`global-generic-param.slang`](global-generic-param.slang) |
| A reference to a generic struct at a concrete type produces a specialize(type, T) type-level value. | functional | [#generic-application](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#generic-application) | [`specialize-generic-type.slang`](specialize-generic-type.slang) |
| Doubly-nested instantiation Box<Box<int>> lowers to a nested specialize(%Box, specialize(%Box, Int)) type-level value. | boundary | [#generic-application](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#generic-application) | [`recursive-generic-depth-two.slang`](recursive-generic-depth-two.slang) |
| Two distinct type_param declarations at module scope each lower to a global_generic_param — pluralised global generic params are independent IR values. | boundary | [#generic-application](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#generic-application) | [`global-generic-param-with-constraint.slang`](global-generic-param-with-constraint.slang) |
| A 3-method interface emits three interface_req_entry instructions, one per method requirement. | boundary | [#interfacereqentry](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry) | [`interface-three-req-entries.slang`](interface-three-req-entries.slang) |
| An interface declaration appears as let %I : Type = interface(%req...) whose operands are the interface_req_entry value-ids. | functional | [#interfacereqentry](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry) | [`interface-lists-req-entries.slang`](interface-lists-req-entries.slang) |
| An interface requirement lowers to interface_req_entry(requirementKey, requirementFuncType) — first operand is the key, second is the function type. | functional | [#interfacereqentry](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry) | [`interface-req-entry-operands.slang`](interface-req-entry-operands.slang) |
| An interface with zero methods lowers to let %IEmpty : Type = interface with no interface_req_entry operands. | boundary | [#interfacereqentry](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry) | [`empty-interface-no-req-entries.slang`](empty-interface-no-req-entries.slang) |
| The function type carried in an interface_req_entry uses this_type(%I) for the receiver parameter. | functional | [#interfacereqentry](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry) | [`interface-req-this-type.slang`](interface-req-this-type.slang) |
| A method call on a generic-constrained value lowers to lookupWitness(witnessTable, requirementKey) with that two-operand shape. | functional | [#lookupwitness-lookupwitnessmethod](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#lookupwitness-lookupwitnessmethod) | [`lookup-witness-operands.slang`](lookup-witness-operands.slang) |
| Two distinct interface methods called on the same generic-constrained value produce two distinct lookupWitness sites keyed by different requirement keys. | boundary | [#lookupwitness-lookupwitnessmethod](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#lookupwitness-lookupwitnessmethod) | [`lookup-witness-multi-method-dispatch.slang`](lookup-witness-multi-method-dispatch.slang) |
| A makeExistential whose concrete-type payload contains a vector field still produces a let %i : %IFoo with the two-operand value/witness shape. | boundary | [#makeexistential](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#makeexistential) | [`make-existential-vector-payload.slang`](make-existential-vector-payload.slang) |
| A makeExistential whose payload is a multi-field struct still emits the same two-operand let %i : %IFoo = makeExistential(value, witness) shape. | boundary | [#makeexistential](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#makeexistential) | [`make-existential-struct-payload.slang`](make-existential-struct-payload.slang) |
| Casting a concrete value to an interface variable lowers to makeExistential(value, witness) with that two-operand shape. | functional | [#makeexistential](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#makeexistential) | [`make-existential-operands.slang`](make-existential-operands.slang) |
| The result type of makeExistential is the target interface type, not a wrapper type. | functional | [#makeexistential](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#makeexistential) | [`make-existential-result-type.slang`](make-existential-result-type.slang) |
| A 0-arg `<>` call to a single-type-param generic with no argument-driven inference is rejected — the specialize operand list cannot be built without a type argument. | negative | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`negative-zero-type-args-no-inference.slang`](negative-zero-type-args-no-inference.slang) |
| A 0-constraint generic call lowers to specialize(%base, T) without any trailing witness operand — pairs with the constrained form's extra witness arg. | boundary | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`specialize-no-constraint.slang`](specialize-no-constraint.slang) |
| A call to a 2-type-param generic produces specialize(%base, T1, T2) — operand list grows with arity per `### specialize`. | boundary | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`specialize-two-type-params.slang`](specialize-two-type-params.slang) |
| A call to a 4-type-param generic produces specialize(%base, T1, T2, T3, T4) — operand list scales with arity per `### specialize`. | boundary | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`specialize-four-type-params.slang`](specialize-four-type-params.slang) |
| A call to a generic function with a concrete type argument lowers to a call whose callee is a specialize(generic, T) opcode. | functional | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`specialize-generic-function.slang`](specialize-generic-function.slang) |
| A generic taking an interface-typed parameter has its specialize call composed alongside a makeExistential argument — the dispatch site interleaves the two opcodes. | stress | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`stress-specialize-with-existential-payload.slang`](stress-specialize-with-existential-payload.slang) |
| A generic with a compile-time integer value parameter lowers to specialize(%base, N : Int) — int literal appears in the operand list. | boundary | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`specialize-value-param.slang`](specialize-value-param.slang) |
| A generic with two interface constraints lowers to specialize(%base, T, %w1, %w2) — one witness operand per constraint. | boundary | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`specialize-multi-constraint.slang`](specialize-multi-constraint.slang) |
| A specialize on an interface-constrained generic passes the type argument followed by the conforming witness table. | functional | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`specialize-with-witness-arg.slang`](specialize-with-witness-arg.slang) |
| An 8-type-parameter generic call still lowers to specialize(%base, T1..T8) with eight argument operands. | stress | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`stress-eight-type-params.slang`](stress-eight-type-params.slang) |
| Five textually identical specialize(%base, Int) call sites all share the same hoistable callee value — the dispatch site spelling is invariant. | stress | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`stress-many-specialize-call-sites.slang`](stress-many-specialize-call-sites.slang) |
| Specializing a constrained generic with a non-conforming type is rejected before lowering — no specialize IR is produced. | negative | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`negative-constraint-not-satisfied.slang`](negative-constraint-not-satisfied.slang) |
| Specializing with an undefined type identifier is rejected at the check stage — no specialize IR is produced. | negative | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`negative-undefined-type-arg.slang`](negative-undefined-type-arg.slang) |
| The specialize opcode is hoistable; two textually-identical specialize references collapse to one value in the IR. | functional | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | [`specialize-hoistable.slang`](specialize-hoistable.slang) |
| The result type of lookupWitness is the requirement's function type, including the receiver as the second parameter. | functional | [#witness-lookup](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#witness-lookup) | [`lookup-witness-result-type.slang`](lookup-witness-result-type.slang) |
| A struct conforming to an interface with zero methods emits an empty witness_table (no witness_table_entry rows). | boundary | [#witness-tables-and-witness-facts](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`empty-interface-witness-table.slang`](empty-interface-witness-table.slang) |
| A struct implementing an interface produces a witness_table whose type is witness_table_t(interface)(implementing-type). | functional | [#witness-tables-and-witness-facts](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | [`witness-table-type-shape.slang`](witness-table-type-shape.slang) |
| A struct conforming to a 3-method interface emits three witness_table_entry rows — one per requirement, in declaration order. | boundary | [#witnesstable-and-witnesstableentry](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#witnesstable-and-witnesstableentry) | [`witness-table-three-method-rows.slang`](witness-table-three-method-rows.slang) |
| Each row inside a witness_table body is witness_table_entry(requirementKey, satisfyingVal) — first operand is the key, second is the satisfying value. | functional | [#witnesstable-and-witnesstableentry](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#witnesstable-and-witnesstableentry) | [`witness-table-entry-operands.slang`](witness-table-entry-operands.slang) |

## Out of scope (no-GPU runner)

The doc explicitly marks these opcodes as `(synthesized)` by later
IR passes. They do not appear at the LOWER-TO-IR stage observed by
this bundle.

- **`bind_global_generic_param`** — `(synthesized)` at link time.
- **`globalValueRef`** — `(synthesized)` to carry references across
  an IR-pass boundary.
- **`makeExistentialWithRTTI`** — `(synthesized)` after
  specialization; `makeExistential` is what the natural cast
  surface produces.
- **`createExistentialObject`** — `(synthesized)` lower-level form
  used after specialization.
- **`wrapExistential`** — `(synthesized)` by the
  `BindExistentialsType` lowering pass.
- **`getValueFromBoundInterface`** — `(synthesized)`.
- **`isNullExistential`** — `(synthesized)`; no portable
  Slang-surface anchor.
- **`extractTaggedUnionTag` / `extractTaggedUnionPayload`** —
  `(synthesized)` by the type-flow specialization pass.
- **`rtti_object` / `GetSequentialID`** — `(synthesized)` by the
  RTTI-object pass; not observable at LOWER-TO-IR.
- **`GetDynamicResourceHeap`** — `(synthesized)`.
- **`packAnyValue` / `unpackAnyValue`** — `(synthesized)` by the
  existential-elimination pass.
- All `### Type-flow specialization` opcodes (`TypeSet`,
  `FuncSet`, `WitnessTableSet`, `GenericSet`,
  `UnboundedTypeElement`, `MakeTaggedUnion`,
  `GetTagFromTaggedUnion`, `GetDispatcher`,
  `SpecializeExistentialsInFunc`, `WeakUse`, etc.) —
  `(synthesized)` by the type-flow pass, post-specialization.
- **`thisTypeWitness`** / **`TypeEqualityWitness`** —
  `(synthesized)` inside `InterfaceDecl` lowering.

## Doc gaps observed

- The `### makeExistential` notable-opcode discussion says the
  opcode "packs a value of some concrete type `C` and a witness
  that `C` conforms to the target interface `I` into a single
  existential value of type `I`", but does not call out the
  dump shape `let %i : %I = makeExistential(%value, %witness)` —
  the result type appearing in the `let` is the target
  interface type, not a constructed `Existential<I>` wrapper. A
  worked example would prevent test-authors from expecting a
  wrapper-typed result.
- The `### specialize` notable-opcode discussion describes the
  operand layout as `specialize(base, arg0, arg1, ...)` but
  does not document that the call-site spelling is
  `call specialize(%base, args...)(callArgs...)` — two
  parenthesised lists, one for the generic arguments and one
  for the runtime arguments. A one-line example would anchor
  consumer tests.
- The `Witness lookup` table row lists the operands as
  `(variadic, min=2)` but the prose under `### lookupWitness /
  LookupWitnessMethod` specifies exactly `witnessTable,
  requirementKey`. The `min=2` is misleading for the natural
  LOWER-TO-IR case; clarifying that the variadic form is for
  later-pass specialized lookups would help.
- The `### Existential destructuring` table lists
  `extractExistentialType` as hoistable (`H` flag) but
  `extractExistentialValue` as not hoistable. The asymmetry is
  the kind of detail a test author would want explained:
  whether `extractExistentialValue` deduplicates across
  identical operands at LOWER-TO-IR is not documented and the
  dump alone cannot answer it (a single use site shows one
  call).
- The doc's `### Generic application` table has a row for
  `global_generic_param` but the AST-origin note
  ("`GenericTypeParamDecl` (at module scope)") does not name
  the actual Slang surface keyword (`type_param`). A two-word
  hint in the AST-origin column would save a search through the
  parser.
- The `witness_table` row in the doc points at
  `structure.md` for its full operand documentation but does
  not summarise that the table's *type* (`witness_table_t(%I)(%S)`)
  is what carries the (interface, implementing-type) pair —
  this is observable at LOWER-TO-IR and is a stable hook for
  FileCheck.
- The `### Generic application` row for `global_generic_param`
  does not say whether a constrained module-scope generic
  (`type_param T : IFoo;`) is supported. Empirically the
  LOWER-TO-IR stage produces a second `let %3 :
  witness_table_t(%IFoo) = global_generic_param` line, but a
  later compiler pass crashes (segfault) before SPIR-V emission
  completes. The doc should either bless the form (and document
  the dual `global_generic_param` shape) or call it out as
  unsupported. A separate `negative-` test was not added because
  the failure mode is a process crash rather than a diagnostic.
- The `### specialize` row's operand list `base, arg, ...` does
  not distinguish type arguments from compile-time integer
  value arguments. Empirically integer literals appear as
  `N : Int` operands inside `specialize(...)`. A one-line note
  in the row would clarify that the operand list is
  type-or-value, not type-only.
- The `### specialize` notable-opcode prose says the opcode is
  hoistable but does not give a per-arity ceiling. Empirically
  the dump accepts arities up to at least 8 type parameters
  without truncation or change-of-spelling. A "no compiler-side
  arity cap" sentence would prevent test authors from probing
  for one.
- The doc does not describe what `### makeExistential` does
  when the concrete-type payload is itself an aggregate
  (struct, vector, array). Empirically the result-type slot
  remains `%I` regardless of payload shape and the value
  operand is the aggregate's IR value. A "payload shape is
  opaque to the opcode" note would clarify the result-type
  invariant.
