---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T19:00:00+00:00
source_commit: ed8f508fc647eecd788a4bd2bb63a4a6f5c80246
watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_doc: docs/generated/design/ir-reference/structure.md
source_doc_digest: 9bf273dbf878cc2a96031c55a8f3b30e38306d217ebad237b034e1829cb681a2
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/structure

## Intent
Tests verify the per-opcode catalog of the IR structural / hierarchy
family described in
[`docs/generated/design/ir-reference/structure.md`](../../../docs/generated/design/ir-reference/structure.md):
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


## Functional coverage
| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| A user-named helper function lowers to func %helper with the signature on its Func(...) result type and parameters as Param children of the entry block. | functional | [#func](../../../docs/generated/design/ir-reference/structure.md#func) | [`func-helper-signature.slang`](func-helper-signature.slang) |
| An entry-point function lowers to a func parent opcode whose Func(...) result type carries the signature. | functional | [#func](../../../docs/generated/design/ir-reference/structure.md#func) | [`func-entry-point.slang`](func-entry-point.slang) |
| Function-level decorations like NameHint attach to the func opcode itself rather than to its body, per the doc's notable-opcodes note on func. | functional | [#func](../../../docs/generated/design/ir-reference/structure.md#func) | [`func-name-hint-decoration.slang`](func-name-hint-decoration.slang) |
| The entry block of a func owns its function parameters as param children in declaration order. | functional | [#func](../../../docs/generated/design/ir-reference/structure.md#func) | [`func-param-on-entry-block.slang`](func-param-on-entry-block.slang) |
| A function call lowers to a call opcode whose first operand is the callee func and remaining operands are the arguments. | functional | [#functions-and-generics](../../../docs/generated/design/ir-reference/structure.md#functions-and-generics) | [`func-call-link.slang`](func-call-link.slang) |
| A generic body produces a func value as its yield result; the LOWER-TO-IR dump shows this as a return_val(%func) on the generic's single block. | functional | [#generic](../../../docs/generated/design/ir-reference/structure.md#generic) | [`generic-body-yields-func.slang`](generic-body-yields-func.slang) |
| A generic function lowers to a generic parent opcode containing a func child whose signature is parameterised by the type parameter. | functional | [#generic](../../../docs/generated/design/ir-reference/structure.md#generic) | [`generic-function-parent.slang`](generic-function-parent.slang) |
| A `static const int` at module scope lowers to a globalConstant opcode wrapping the literal value. | functional | [#global-state](../../../docs/generated/design/ir-reference/structure.md#global-state) | [`global-constant-value.slang`](global-constant-value.slang) |
| A module-scope `static int` variable lowers to a global_var opcode with a Ptr-typed result and a one-block initializer body. | functional | [#global-state](../../../docs/generated/design/ir-reference/structure.md#global-state) | [`global-var-mutable.slang`](global-var-mutable.slang) |
| A module-scope uniform parameter lowers to a global_param opcode at module scope. | functional | [#global-state](../../../docs/generated/design/ir-reference/structure.md#global-state) | [`global-param-uniform.slang`](global-param-uniform.slang) |
| An interface declaration lowers to an interface opcode whose operands are interface_req_entry rows pairing a requirement key with the requirement's type. | functional | [#interface-internals](../../../docs/generated/design/ir-reference/structure.md#interface-internals) | [`interface-with-requirement.slang`](interface-with-requirement.slang) |
| An interface with multiple method requirements emits one interface_req_entry per requirement, and the interface opcode lists all of them as its operands. | functional | [#interface-internals](../../../docs/generated/design/ir-reference/structure.md#interface-internals) | [`interface-multiple-requirements.slang`](interface-multiple-requirements.slang) |
| The requirementKey operand of an interface_req_entry is the same StructKey value that names the requirement on the witness_table_entry that satisfies it. | functional | [#interface-internals](../../../docs/generated/design/ir-reference/structure.md#interface-internals) | [`interface-req-key-shape.slang`](interface-req-key-shape.slang) |
| A field-name key is a top-level let with an export-linkage decoration so the same field compares equal across compilation units. | functional | [#key-structkey](../../../docs/generated/design/ir-reference/structure.md#key-structkey) | [`struct-key-has-export-linkage.slang`](struct-key-has-export-linkage.slang) |
| Field-access opcodes (get_field_addr) use the StructKey as the selector, confirming that StructKey is the identity of a field rather than a string name. | functional | [#key-structkey](../../../docs/generated/design/ir-reference/structure.md#key-structkey) | [`struct-key-selects-field-access.slang`](struct-key-selects-field-access.slang) |
| A class declaration lowers to a class parent opcode (distinct spelling from struct) owning field and key children. | functional | [#struct-internals](../../../docs/generated/design/ir-reference/structure.md#struct-internals) | [`class-parent-with-field.slang`](class-parent-with-field.slang) |
| A struct declaration lowers to a struct parent opcode whose children are field opcodes pairing a key with a fieldType. | functional | [#struct-internals](../../../docs/generated/design/ir-reference/structure.md#struct-internals) | [`struct-parent-with-fields.slang`](struct-parent-with-fields.slang) |
| A struct with multiple heterogeneous-typed fields emits one field child per declared field, each pairing its own key with its declared type. | functional | [#struct-internals](../../../docs/generated/design/ir-reference/structure.md#struct-internals) | [`struct-with-many-fields.slang`](struct-with-many-fields.slang) |
| A generic body that calls a method on a constrained type parameter emits a lookupWitness opcode whose operands are (witness_table, requirementKey). | functional | [#witness-tables-and-witness-facts](../../../docs/generated/design/ir-reference/structure.md#witness-tables-and-witness-facts) | [`lookup-witness-in-generic.slang`](lookup-witness-in-generic.slang) |
| A struct that implements an interface produces a witness_table parent whose type is witness_table_t(interface)(implementing-type). | functional | [#witness-tables-and-witness-facts](../../../docs/generated/design/ir-reference/structure.md#witness-tables-and-witness-facts) | [`witness-table-parent.slang`](witness-table-parent.slang) |
| Each row of a witness_table is a witness_table_entry pairing a requirementKey with the concrete satisfying function value. | functional | [#witnesstableentry-vs-interfacereqentry](../../../docs/generated/design/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry) | [`witness-table-entry-pairs.slang`](witness-table-entry-pairs.slang) |
| Two distinct struct conformances to the same interface produce two distinct witness_tables both keyed by the same requirementKey. | functional | [#witnesstableentry-vs-interfacereqentry](../../../docs/generated/design/ir-reference/structure.md#witnesstableentry-vs-interfacereqentry) | [`two-impls-share-requirement-key.slang`](two-impls-share-requirement-key.slang) |


## Untested claims
| Claim | Reason | Anchor | Why untested |
| --- | --- | --- | --- |
| **`global_generic_param`** — a `GenericTypeParamDecl` at module level (not nested under a `generic` parent) is non-trivial to produce from natural Slang surface code. | (unclassified) | [#globalgenericparam](../../../docs/generated/design/ir-reference/structure.md#globalgenericparam) | Reason and explanation to be refined by the next regeneration. |
| **`SymbolAlias`** — per the doc, `(synthesized as part of linking)`; not observable at LOWER-TO-IR. The doc itself notes "No `SymbolAlias` should survive past linking". | (unclassified) | [#symbolalias](../../../docs/generated/design/ir-reference/structure.md#symbolalias) | Reason and explanation to be refined by the next regeneration. |
| **`thisTypeWitness`** — `(synthesized inside InterfaceDecl lowering)`; not portably observable in a stable dump form. | (unclassified) | [#thistypewitness](../../../docs/generated/design/ir-reference/structure.md#thistypewitness) | Reason and explanation to be refined by the next regeneration. |
| **`module` / `ModuleInst`** — observable only as the implicit preamble that wraps every IR dump. There is no anchor inside an IR dump that uniquely identifies the module instruction in a FileCheck-able way without coupling to dump preamble formatting. | implementation-detail | [#module](../../../docs/generated/design/ir-reference/structure.md#module) | Internal compiler choice (pass ordering, hoistability decisions, deduplication) with no test-directive that reveals it. |
| **`global_hashed_string_literals`** — `(synthesized)` container; no natural surface anchor. | link-stage-only | [#globalhashedstringliterals](../../../docs/generated/design/ir-reference/structure.md#globalhashedstringliterals) | Synthesized at a later IR pass than this bundle's `pipeline_stage` observes; the test belongs in the bundle whose pipeline stage matches. |
| **`indexedFieldKey`** — `(synthesized)`; the doc lists no AST origin and the natural surface (tuple-like field access) does not produce it portably at LOWER-TO-IR. | link-stage-only | [#indexedfieldkey](../../../docs/generated/design/ir-reference/structure.md#indexedfieldkey) | Synthesized at a later IR pass than this bundle's `pipeline_stage` observes; the test belongs in the bundle whose pipeline stage matches. |
| **`TypeEqualityWitness`** — `(synthesized)`; no AST origin. | link-stage-only | [#typeequalitywitness](../../../docs/generated/design/ir-reference/structure.md#typeequalitywitness) | Synthesized at a later IR pass than this bundle's `pipeline_stage` observes; the test belongs in the bundle whose pipeline stage matches. |


## Doc gaps observed
| Anchor | Kind | Gap | Suggested addition |
| --- | --- | --- | --- |
| [#generic](../../../docs/generated/design/ir-reference/structure.md#generic) | undocumented-behavior | The doc's `### generic` notable-opcode discussion states "Each `generic` has a single block, and that block ends with a `yield` (not a `return_val`) whose operand is the result of the type- level computation." The actual LOWER-TO-IR dump shows `return_val(%func)` rather than a `yield`-spelled terminator — the `yield` opcode is the underlying instruction but the dump prints it as `return_val` at this stage. A clarifying note on the dump-vs-internal spelling would prevent test-author confusion. (See `generic-body-yields-func.slang`.) |  |
| [#key-structkey](../../../docs/generated/design/ir-reference/structure.md#key-structkey) | missing-example | The `### key / StructKey` discussion describes the role of the key as "globally linkable IR value" but does not specify the shape of the `[export(...)]` linkage decoration (the per-field key uses the field-export pattern `_SVR...`, distinct from the `key__...` prefix used for interface requirements). | A worked example showing both forms side-by-side would prevent over- specific FileCheck patterns. |
| [#func](../../../docs/generated/design/ir-reference/structure.md#func) | undocumented-behavior | The `### func` notable-opcode discussion notes "the function signature is on the `func` itself via its type" but does not call out that the printed form is `func %name : Func(retT, paramT...)` — the `Func(...)` constructor wraps the return type followed by parameter types in source order. | A one-line note would anchor consumer tests. |
| [#module](../../../docs/generated/design/ir-reference/structure.md#module) | undocumented-behavior | The `Module` table row lists `module` as the top-level container but does not name a `-dump-ir`-observable anchor for the `module` opcode itself (the dump preamble is large and not stable across versions). | A note that `module` is implicitly observed via the existence of its top-level children, rather than as a named line in the dump, would clarify the testing-vs-internal observability boundary. |
| [#documented-in-detail-in-control-flowmd](../../../docs/generated/design/ir-reference/structure.md#documented-in-detail-in-control-flowmd) | undocumented-behavior | The `Functions and generics` table row for `param` says "Documented in detail in control-flow.md", but the function- signature role (entry-block params holding the declared function parameters) is *not* documented in `control-flow.md` (which anchors `param` in its block-parameter / phi-replacement role only). A brief note in `structure.md` that the entry block of a `func` carries the function's parameters as `param` children — in declaration order — would close the gap. |  |
| [#module-scope-mutable-variable](../../../docs/generated/design/ir-reference/structure.md#module-scope-mutable-variable) | undocumented-behavior | The `Global state` row for `global_var` says "Module-scope mutable variable" but does not call out the dump shape: a one-block body that returns the initializer value (`return_val(<init>)`). | A short example would prevent test-authors from expecting a bare module-scope `let` with the initializer inlined. |
