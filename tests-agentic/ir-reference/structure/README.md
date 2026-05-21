---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T19:00:00+00:00
source_commit: ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_doc: docs/llm-generated/ir-reference/structure.md
source_doc_digest: 9bf273dbf878cc2a96031c55a8f3b30e38306d217ebad237b034e1829cb681a2
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/structure

## Intent

Tests verify the per-opcode catalog of the IR structural / hierarchy
family described in
[`docs/llm-generated/ir-reference/structure.md`](../../../docs/llm-generated/ir-reference/structure.md):
that each documented structural opcode (`func`, `generic`, `param`
[in function-signature role], `global_var`, `global_param`,
`globalConstant`, `struct`, `class`, `field`, `key`, `interface`,
`interface_req_entry`, `witness_table`, `witness_table_entry`,
`lookupWitness`) appears in `-dump-ir` output for the obvious AST
declaration that produces it, and that the parent-child structural
shape matches the doc.

The primary observation mechanism is `-target spirv-asm -dump-ir -o
/dev/null -entry main -stage compute` followed by a FileCheck
against the LOWER-TO-IR section. Anchors are user-named top-level
symbols (`func %main`, `func %addPair`, `struct %Point`,
`class %Counter`, `let %gK`, `witness_table %N`); the IR-dump
preamble is large and any unanchored pattern risks false positives.

The angle distinguishing this bundle from
`cross-cutting/ir-instructions` (which samples each IR family
once) and `ir-reference/values` (which catalogs value-producing
opcodes) is: **the structural / hierarchy axis** — every test
observes a parent / child relationship, a top-level declaration's
linkage, or a structural cross-link (interface-side and
witness-side sharing a key, generic body yielding a func).

## Claims enumerated

| Claim ID | Anchor | Claim (one line) | Tests |
| --- | --- | --- | --- |
| C-01 | [#func](../../../docs/llm-generated/ir-reference/structure.md#func) | An entry-point function lowers to a `func` parent opcode with the signature on its `Func(...)` result type. | [`func-entry-point.slang`](func-entry-point.slang) |
| C-02 | [#func](../../../docs/llm-generated/ir-reference/structure.md#func) | A user-named helper function lowers to `func %helper : Func(retT, paramT...)` with `param` children on the entry block. | [`func-helper-signature.slang`](func-helper-signature.slang) |
| C-03 | [#functions-and-generics](../../../docs/llm-generated/ir-reference/structure.md#functions-and-generics) | A call lowers to a `call` opcode whose first operand is the callee `func` and remaining operands are the arguments. | [`func-call-link.slang`](func-call-link.slang) |
| C-04 | [#func](../../../docs/llm-generated/ir-reference/structure.md#func) | The entry block of a `func` owns its function parameters as `param` children in declaration order. | [`func-param-on-entry-block.slang`](func-param-on-entry-block.slang) |
| C-05 | [#func](../../../docs/llm-generated/ir-reference/structure.md#func) | `NameHintDecoration` attaches to the `func` opcode itself, immediately preceding the `func %name` header line in the dump. | [`func-name-hint-decoration.slang`](func-name-hint-decoration.slang) |
| C-06 | [#struct-internals](../../../docs/llm-generated/ir-reference/structure.md#struct-internals) | A `struct` declaration lowers to a `struct` parent opcode owning `field(key, fieldType)` children. | [`struct-parent-with-fields.slang`](struct-parent-with-fields.slang) |
| C-07 | [#struct-internals](../../../docs/llm-generated/ir-reference/structure.md#struct-internals) | A `class` declaration lowers to a `class` parent opcode (distinct spelling from `struct`) with the same `field`/`key` child shape. | [`class-parent-with-field.slang`](class-parent-with-field.slang) |
| C-08 | [#struct-internals](../../../docs/llm-generated/ir-reference/structure.md#struct-internals) | A struct with multiple heterogeneous-typed fields emits one `field(key, T)` child per declared field with matching element type. | [`struct-with-many-fields.slang`](struct-with-many-fields.slang) |
| C-09 | [#key-structkey](../../../docs/llm-generated/ir-reference/structure.md#key-structkey) | A field-name key is a top-level `let %name : _ = key` carrying an `[export(...)]` decoration recording its mangled linkage name. | [`struct-key-has-export-linkage.slang`](struct-key-has-export-linkage.slang) |
| C-10 | [#key-structkey](../../../docs/llm-generated/ir-reference/structure.md#key-structkey) | Field-access opcodes (`get_field_addr`) use the `StructKey` as their selector. | [`struct-key-selects-field-access.slang`](struct-key-selects-field-access.slang) |
| C-11 | [#interface-internals](../../../docs/llm-generated/ir-reference/structure.md#interface-internals) | An `interface` declaration lowers to `interface(...)` whose operands are `interface_req_entry(key, requirementType)` rows. | [`interface-with-requirement.slang`](interface-with-requirement.slang) |
| C-12 | [#interface-internals](../../../docs/llm-generated/ir-reference/structure.md#interface-internals) | An interface with multiple method requirements emits one `interface_req_entry` per requirement and lists all of them as operands of the `interface` opcode. | [`interface-multiple-requirements.slang`](interface-multiple-requirements.slang) |
| C-13 | [#witnesstableentry-vs-interfacereqentry](../../../docs/llm-generated/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry) | The `requirementKey` operand of an `interface_req_entry` is the same `StructKey` value used by the satisfying `witness_table_entry`. | [`interface-req-key-shape.slang`](interface-req-key-shape.slang) |
| C-14 | [#witness-tables-and-witness-facts](../../../docs/llm-generated/ir-reference/structure.md#witness-tables-and-witness-facts) | A struct implementing an interface produces a `witness_table` whose type is `witness_table_t(interface)(implementing-type)`. | [`witness-table-parent.slang`](witness-table-parent.slang) |
| C-15 | [#witnesstableentry-vs-interfacereqentry](../../../docs/llm-generated/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry) | Each row of a `witness_table` is a `witness_table_entry(requirementKey, satisfyingVal)`. | [`witness-table-entry-pairs.slang`](witness-table-entry-pairs.slang) |
| C-16 | [#witnesstableentry-vs-interfacereqentry](../../../docs/llm-generated/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry) | Two distinct struct conformances to the same interface produce two distinct `witness_table`s both keyed by the same `requirementKey`. | [`two-impls-share-requirement-key.slang`](two-impls-share-requirement-key.slang) |
| C-17 | [#witness-tables-and-witness-facts](../../../docs/llm-generated/ir-reference/structure.md#witness-tables-and-witness-facts) | A generic body that calls a method on a constrained type parameter emits a `lookupWitness(witnessTable, requirementKey)` opcode. | [`lookup-witness-in-generic.slang`](lookup-witness-in-generic.slang) |
| C-18 | [#generic](../../../docs/llm-generated/ir-reference/structure.md#generic) | A generic function lowers to a `generic` parent opcode containing a `func` child whose signature is parameterised by the type parameter. | [`generic-function-parent.slang`](generic-function-parent.slang) |
| C-19 | [#generic](../../../docs/llm-generated/ir-reference/structure.md#generic) | The generic body produces a `func` value as its yield result; the LOWER-TO-IR dump shows `return_val(%func)` on the generic's single block. | [`generic-body-yields-func.slang`](generic-body-yields-func.slang) |
| C-20 | [#global-state](../../../docs/llm-generated/ir-reference/structure.md#global-state) | A module-scope `static int` variable lowers to a `global_var %name : Ptr(T)` opcode with a one-block initializer body. | [`global-var-mutable.slang`](global-var-mutable.slang) |
| C-21 | [#global-state](../../../docs/llm-generated/ir-reference/structure.md#global-state) | A module-scope `uniform` parameter lowers to `let %name : T = global_param`. | [`global-param-uniform.slang`](global-param-uniform.slang) |
| C-22 | [#global-state](../../../docs/llm-generated/ir-reference/structure.md#global-state) | A module-scope `static const` lowers to `let %name : T = globalConstant(literal)`. | [`global-constant-value.slang`](global-constant-value.slang) |

## Tests in this bundle

| File | Intent | Doc anchor |
| --- | --- | --- |
| [`func-entry-point.slang`](func-entry-point.slang) | functional | `#func` |
| [`func-helper-signature.slang`](func-helper-signature.slang) | functional | `#func` |
| [`func-call-link.slang`](func-call-link.slang) | functional | `#functions-and-generics` |
| [`func-param-on-entry-block.slang`](func-param-on-entry-block.slang) | functional | `#func` |
| [`func-name-hint-decoration.slang`](func-name-hint-decoration.slang) | functional | `#func` |
| [`struct-parent-with-fields.slang`](struct-parent-with-fields.slang) | functional | `#struct-internals` |
| [`class-parent-with-field.slang`](class-parent-with-field.slang) | functional | `#struct-internals` |
| [`struct-with-many-fields.slang`](struct-with-many-fields.slang) | functional | `#struct-internals` |
| [`struct-key-has-export-linkage.slang`](struct-key-has-export-linkage.slang) | functional | `#key-structkey` |
| [`struct-key-selects-field-access.slang`](struct-key-selects-field-access.slang) | functional | `#key-structkey` |
| [`interface-with-requirement.slang`](interface-with-requirement.slang) | functional | `#interface-internals` |
| [`interface-multiple-requirements.slang`](interface-multiple-requirements.slang) | functional | `#interface-internals` |
| [`interface-req-key-shape.slang`](interface-req-key-shape.slang) | functional | `#witnesstableentry-vs-interfacereqentry` |
| [`witness-table-parent.slang`](witness-table-parent.slang) | functional | `#witness-tables-and-witness-facts` |
| [`witness-table-entry-pairs.slang`](witness-table-entry-pairs.slang) | functional | `#witnesstableentry-vs-interfacereqentry` |
| [`two-impls-share-requirement-key.slang`](two-impls-share-requirement-key.slang) | functional | `#witnesstableentry-vs-interfacereqentry` |
| [`lookup-witness-in-generic.slang`](lookup-witness-in-generic.slang) | functional | `#witness-tables-and-witness-facts` |
| [`generic-function-parent.slang`](generic-function-parent.slang) | functional | `#generic` |
| [`generic-body-yields-func.slang`](generic-body-yields-func.slang) | functional | `#generic` |
| [`global-var-mutable.slang`](global-var-mutable.slang) | functional | `#global-state` |
| [`global-param-uniform.slang`](global-param-uniform.slang) | functional | `#global-state` |
| [`global-constant-value.slang`](global-constant-value.slang) | functional | `#global-state` |

## Out of scope (no-GPU runner)

- **`module` / `ModuleInst`** — observable only as the implicit
  preamble that wraps every IR dump. There is no anchor inside an
  IR dump that uniquely identifies the module instruction in a
  FileCheck-able way without coupling to dump preamble formatting.
- **`SymbolAlias`** — per the doc, `(synthesized as part of linking)`;
  not observable at LOWER-TO-IR. The doc itself notes "No
  `SymbolAlias` should survive past linking".
- **`indexedFieldKey`** — `(synthesized)`; the doc lists no AST
  origin and the natural surface (tuple-like field access) does
  not produce it portably at LOWER-TO-IR.
- **`thisTypeWitness`** — `(synthesized inside InterfaceDecl
  lowering)`; not portably observable in a stable dump form.
- **`TypeEqualityWitness`** — `(synthesized)`; no AST origin.
- **`global_hashed_string_literals`** — `(synthesized)` container;
  no natural surface anchor.
- **`global_generic_param`** — a `GenericTypeParamDecl` at module
  level (not nested under a `generic` parent) is non-trivial to
  produce from natural Slang surface code.

## Doc gaps observed

- The doc's `### generic` notable-opcode discussion states "Each
  `generic` has a single block, and that block ends with a `yield`
  (not a `return_val`) whose operand is the result of the type-
  level computation." The actual LOWER-TO-IR dump shows
  `return_val(%func)` rather than a `yield`-spelled terminator —
  the `yield` opcode is the underlying instruction but the dump
  prints it as `return_val` at this stage. A clarifying note on
  the dump-vs-internal spelling would prevent test-author
  confusion. (See `generic-body-yields-func.slang`.)
- The `### key / StructKey` discussion describes the role of the
  key as "globally linkable IR value" but does not specify the
  shape of the `[export(...)]` linkage decoration (the per-field
  key uses the field-export pattern `_SVR...`, distinct from the
  `key__...` prefix used for interface requirements). A worked
  example showing both forms side-by-side would prevent over-
  specific FileCheck patterns.
- The `### func` notable-opcode discussion notes "the function
  signature is on the `func` itself via its type" but does not
  call out that the printed form is `func %name : Func(retT,
  paramT...)` — the `Func(...)` constructor wraps the return type
  followed by parameter types in source order. A one-line note
  would anchor consumer tests.
- The `Module` table row lists `module` as the top-level
  container but does not name a `-dump-ir`-observable anchor for
  the `module` opcode itself (the dump preamble is large and not
  stable across versions). A note that `module` is implicitly
  observed via the existence of its top-level children, rather
  than as a named line in the dump, would clarify the
  testing-vs-internal observability boundary.
- The `Functions and generics` table row for `param` says
  "Documented in detail in control-flow.md", but the function-
  signature role (entry-block params holding the declared
  function parameters) is *not* documented in `control-flow.md`
  (which anchors `param` in its block-parameter / phi-replacement
  role only). A brief note in `structure.md` that the entry block
  of a `func` carries the function's parameters as `param`
  children — in declaration order — would close the gap.
- The `Global state` row for `global_var` says "Module-scope
  mutable variable" but does not call out the dump shape: a
  one-block body that returns the initializer value
  (`return_val(<init>)`). A short example would prevent
  test-authors from expecting a bare module-scope `let` with the
  initializer inlined.
