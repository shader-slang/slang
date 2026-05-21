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

## Claims enumerated

| Claim ID | Anchor | Claim (one line) | Tests |
| --- | --- | --- | --- |
| C-01 | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | A call to a generic function with concrete type argument lowers to `call specialize(%generic, T)(%args)`. | `specialize-generic-function.slang` |
| C-02 | [#generic-application](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#generic-application) | A reference to a generic struct instantiated at a concrete type produces a `specialize(%type, T)` type-level value. | `specialize-generic-type.slang` |
| C-03 | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | When a generic carries an interface constraint, the `specialize` call passes the type argument followed by the conforming witness table. | `specialize-with-witness-arg.slang` |
| C-04 | [#specialize](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#specialize) | The `specialize` opcode is hoistable: a single textually-spelled `specialize(%base, args)` site appears in the dump at most once per (base, args) tuple. | `specialize-hoistable.slang` |
| C-05 | [#lookupwitness-lookupwitnessmethod](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#lookupwitness-lookupwitnessmethod) | A method call on a generic-constrained value lowers to `lookupWitness(%witness, %requirementKey)` whose two operands are `(witnessTable, requirementKey)`. | `lookup-witness-operands.slang` |
| C-06 | [#witness-lookup](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#witness-lookup) | The result type of `lookupWitness` is the requirement's function type — for `int run(int x)` the result is `Func(Int, %T, Int)`. | `lookup-witness-result-type.slang` |
| C-07 | [#makeexistential](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#makeexistential) | Casting a concrete value `a : A` to an interface variable `IFoo i = a` produces `makeExistential(%a, %witnessA_IFoo)` — the two operands are `(value, witness)`. | `make-existential-operands.slang` |
| C-08 | [#makeexistential](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#makeexistential) | The result type of `makeExistential` is the target interface type — `let %i : %IFoo = makeExistential(...)`. | `make-existential-result-type.slang` |
| C-09 | [#existential-destructuring](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#existential-destructuring) | Dispatching through an interface-typed value produces `extractExistentialType(%i)` whose result type is `Type`. | `extract-existential-type.slang` |
| C-10 | [#existential-destructuring](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#existential-destructuring) | Dispatching through an interface-typed value produces `extractExistentialValue(%i)` reading the packed concrete-typed value. | `extract-existential-value.slang` |
| C-11 | [#existential-destructuring](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#existential-destructuring) | Dispatching through an interface-typed value produces `extractExistentialWitnessTable(%i)` whose result type is `witness_table_t(%I)`. | `extract-existential-witness-table.slang` |
| C-12 | [#existential-destructuring](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#existential-destructuring) | Interface dispatch on an existential follows the sequence `extractExistentialType` → `extractExistentialValue` + `extractExistentialWitnessTable` → `lookupWitness` → `call`, all four opcodes appearing in order in the same dispatch site. | `existential-dispatch-sequence.slang` |
| C-13 | [#witness-tables-and-witness-facts](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#witness-tables-and-witness-facts) | A `struct S : I` emits a `witness_table %N : witness_table_t(%I)(%S)` whose type's parenthesised pair is `(interface, implementing-type)`. | `witness-table-type-shape.slang` |
| C-14 | [#witnesstable-and-witnesstableentry](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#witnesstable-and-witnesstableentry) | Each row inside a `witness_table` body is `witness_table_entry(%Ix5Freq, %Sx5Freq)` — first operand is the requirement key, second is the satisfying value. | `witness-table-entry-operands.slang` |
| C-15 | [#interfacereqentry](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry) | `interface I { int run(int x); }` emits `interface_req_entry(%Ix5Frun, Func(Int, this_type(%I), Int))` — first operand is the requirement key, second is the requirement's function type. | `interface-req-entry-operands.slang` |
| C-16 | [#interfacereqentry](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry) | The function type carried in an `interface_req_entry` uses `this_type(%I)` for the receiver parameter. | `interface-req-this-type.slang` |
| C-17 | [#interfacereqentry](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#interfacereqentry) | An interface declaration appears as `let %I : Type = interface(%req, ...)` whose operands are the `interface_req_entry` value-ids. | `interface-lists-req-entries.slang` |
| C-18 | [#generic-application](../../../docs/llm-generated/ir-reference/generics-and-existentials.md#generic-application) | A module-scope `type_param T;` declaration lowers to `let %T : Type = global_generic_param`. | `global-generic-param.slang` |

## Tests in this bundle

| File | Intent | Doc anchor |
| --- | --- | --- |
| `specialize-generic-function.slang` | functional | `#specialize` |
| `specialize-generic-type.slang` | functional | `#generic-application` |
| `specialize-with-witness-arg.slang` | functional | `#specialize` |
| `specialize-hoistable.slang` | functional | `#specialize` |
| `lookup-witness-operands.slang` | functional | `#lookupwitness-lookupwitnessmethod` |
| `lookup-witness-result-type.slang` | functional | `#witness-lookup` |
| `make-existential-operands.slang` | functional | `#makeexistential` |
| `make-existential-result-type.slang` | functional | `#makeexistential` |
| `extract-existential-type.slang` | functional | `#existential-destructuring` |
| `extract-existential-value.slang` | functional | `#existential-destructuring` |
| `extract-existential-witness-table.slang` | functional | `#existential-destructuring` |
| `existential-dispatch-sequence.slang` | functional | `#existential-destructuring` |
| `witness-table-type-shape.slang` | functional | `#witness-tables-and-witness-facts` |
| `witness-table-entry-operands.slang` | functional | `#witnesstable-and-witnesstableentry` |
| `interface-req-entry-operands.slang` | functional | `#interfacereqentry` |
| `interface-req-this-type.slang` | functional | `#interfacereqentry` |
| `interface-lists-req-entries.slang` | functional | `#interfacereqentry` |
| `global-generic-param.slang` | functional | `#generic-application` |
| `specialize-two-type-params.slang` | boundary | `#specialize` |
| `specialize-four-type-params.slang` | boundary | `#specialize` |
| `specialize-value-param.slang` | boundary | `#specialize` |
| `specialize-no-constraint.slang` | boundary | `#specialize` |
| `specialize-multi-constraint.slang` | boundary | `#specialize` |
| `global-generic-param-with-constraint.slang` | boundary | `#generic-application` |
| `empty-interface-witness-table.slang` | boundary | `#witness-tables-and-witness-facts` |
| `empty-interface-no-req-entries.slang` | boundary | `#interfacereqentry` |
| `witness-table-three-method-rows.slang` | boundary | `#witnesstable-and-witnesstableentry` |
| `interface-three-req-entries.slang` | boundary | `#interfacereqentry` |
| `lookup-witness-multi-method-dispatch.slang` | boundary | `#lookupwitness-lookupwitnessmethod` |
| `make-existential-vector-payload.slang` | boundary | `#makeexistential` |
| `make-existential-struct-payload.slang` | boundary | `#makeexistential` |
| `recursive-generic-depth-two.slang` | boundary | `#generic-application` |
| `stress-eight-type-params.slang` | stress | `#specialize` |
| `stress-recursive-generic-depth-five.slang` | stress | `#generic-application` |
| `stress-many-specialize-call-sites.slang` | stress | `#specialize` |
| `stress-specialize-with-existential-payload.slang` | stress | `#specialize` |
| `negative-constraint-not-satisfied.slang` | negative | `#specialize` |
| `negative-zero-type-args-no-inference.slang` | negative | `#specialize` |
| `negative-undefined-type-arg.slang` | negative | `#specialize` |

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
