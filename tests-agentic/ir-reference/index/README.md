---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-20T17:58:43Z
source_commit: 30ae111120515b7406aa6f427a4eaaa28a0903d8
watched_paths_digest: 06d9ac3c0f70e6ea59684d52fb680ade68d2455e3c924214a39cc4442e3fad70
source_doc: docs/llm-generated/ir-reference/index.md
source_doc_digest: c7bb18980b66f41a697cbfaf3565d541d61e59bd49630c18d2eab6e5dba4d8f1
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/index

## Intent

Tests verify the **cross-cutting orientation claims** made by
[`docs/llm-generated/ir-reference/index.md`](../../../docs/llm-generated/ir-reference/index.md):
the ten-family taxonomy assigning every concrete opcode to a
single family page, the AST-to-IR mapping claim that `visit*`
methods in `slang-lower-to-ir.cpp` produce named opcodes (e.g.
`visitVarDecl` -> `Var`, `visitInfixExpr` -> `Add`/`Sub`/`Mul`),
and the Decoration-family claim that decorations attach to
instructions rather than forming a separate stream.

The bundle is intentionally small (6 tests). The index doc is
mostly a set of pointers to per-family pages; the per-opcode
catalog observations belong to the ten peer bundles under
`tests-agentic/ir-reference/`, and we route them there via
`## Out of scope` rather than duplicating.

Strategy: one positive test per cross-cutting claim that the
index doc itself asserts, observed by `-dump-ir` against a
text target with `-o /dev/null` (per `_common.md`'s rules for
IR-dump observation).

## Coverage

| Claim | Intent | Anchor | Tests |
| --- | --- | --- | --- |
| Per the family taxonomy row for the Decoration family, decorations are metadata attached to instructions; an entry-point function shows a decoration token co-located with its host func opcode in the IR dump. | functional | [#family-taxonomy](../../../docs/llm-generated/ir-reference/index.md#family-taxonomy) | [`decoration-attaches-to-host-inst.slang`](decoration-attaches-to-host-inst.slang) |
| Per the family taxonomy, a struct declaration belongs to the Structure family; the IR dump shows a struct parent opcode. | functional | [#family-taxonomy](../../../docs/llm-generated/ir-reference/index.md#family-taxonomy) | [`family-taxonomy-struct-decl-is-structure.slang`](family-taxonomy-struct-decl-is-structure.slang) |
| Per the family taxonomy, branches and block terminators belong to the Control-flow family; an if statement lowers to an ifElse/conditional terminator with multiple blocks. | functional | [#family-taxonomy](../../../docs/llm-generated/ir-reference/index.md#family-taxonomy) | [`family-taxonomy-if-stmt-is-control-flow.slang`](family-taxonomy-if-stmt-is-control-flow.slang) |
| Per the index family taxonomy, arithmetic opcodes belong to the Values family; an integer add lowers to add in the IR. | functional | [#family-taxonomy](../../../docs/llm-generated/ir-reference/index.md#family-taxonomy) | [`family-taxonomy-arithmetic-is-values.slang`](family-taxonomy-arithmetic-is-values.slang) |
| The index doc names visitInfixExpr as dispatching to arithmetic opcodes like Add, Sub, Mul; mixed-operator code emits each opcode. | functional | [#how-ast-nodes-lower-to-ir](../../../docs/llm-generated/ir-reference/index.md#how-ast-nodes-lower-to-ir) | [`ast-mapping-infix-dispatches-to-arith.slang`](ast-mapping-infix-dispatches-to-arith.slang) |
| The index doc names visitVarDecl as the AST-to-IR mapping that emits a Var opcode; a struct-typed local should produce a var instruction in the IR. | functional | [#how-ast-nodes-lower-to-ir](../../../docs/llm-generated/ir-reference/index.md#how-ast-nodes-lower-to-ir) | [`ast-mapping-var-decl-emits-var.slang`](ast-mapping-var-decl-emits-var.slang) |

## Out of scope

The index doc explicitly delegates these to peer pages; tests
for them belong to peer bundles, not here:

- Per-opcode type catalog (`IntType`, `StructuredBufferType`,
  vector/matrix types, ...) -- see
  `tests-agentic/ir-reference/types/`.
- Per-opcode value catalog (memory ops, aggregate constructors,
  constexpr arithmetic, bit-cast and conversion) -- see
  `tests-agentic/ir-reference/values/`.
- Structural opcodes (`module`, `func`, `generic`, `global_var`,
  `global_param`, `struct`, `interface`, `witness_table`, `key`,
  `field`, `lookupWitness`) catalog -- see
  `tests-agentic/ir-reference/structure/`.
- `block`, `param` (block-parameter role), branches, terminators,
  `Require*` markers -- see
  `tests-agentic/ir-reference/control-flow/`.
- `specialize`, existential pack/unpack, RTTI, type-flow
  dispatchers -- see
  `tests-agentic/ir-reference/generics-and-existentials/`.
- Image/buffer/sampler/atomic/raytracing/wave opcodes -- see
  `tests-agentic/ir-reference/resources-and-atomics/`.
- Autodiff opcodes (differential pairs, forward/backward
  differentiate) -- see
  `tests-agentic/ir-reference/differentiation/`.
- Individual decoration semantics (the ~180 decoration opcodes
  catalog) -- see `tests-agentic/ir-reference/decorations/`.
  The index bundle anchors only the **fact** that decorations
  attach to instructions; it does not catalog individual
  decoration semantics.
- `Layout`, `Attr`, `Debug*`, `SPIRVAsmOperand` -- see
  `tests-agentic/ir-reference/metadata/`.
- `nop`, `Unrecognized`, type queries, size/alignment, storage
  casts, liveness markers, descriptor heaps, kernel launch --
  see `tests-agentic/ir-reference/misc/`.
- IR schema, op-flag bits, hoistable/global deduplication,
  module versioning, and the workflow for adding a new opcode --
  see `cross-cutting/ir-instructions` (the index doc names it
  as the canonical conventions doc, but it is itself a
  cross-cutting topic anchor, not an index-level claim).

## Sibling-bundle overlap

The following peer-bundle behaviors are intentionally not
re-tested here to avoid duplication:

- `ir-reference/values` has many tests for individual arithmetic
  opcodes (`arith-bitwise`, `arith-shifts`, `arith-irem-frem-neg`,
  ...). This bundle's `family-taxonomy-arithmetic-is-values.slang`
  does not duplicate those; it pins the index-level *taxonomy*
  claim (arithmetic belongs to Values) using the simplest
  possible add.
- `ir-reference/structure` has `struct-parent-with-fields.slang`
  exercising the structural parent-child shape in detail. This
  bundle's `family-taxonomy-struct-decl-is-structure.slang`
  uses a struct only to pin the index's taxonomy claim, not to
  observe field-child opcode shapes.
- `ir-reference/decorations` has the full ~180-row decoration
  catalog (`entry-point-decoration`, `name-hint-on-function`,
  ...). This bundle's `decoration-attaches-to-host-inst.slang`
  uses `[entryPoint(...)]` only as the most reliable witness
  of the cross-cutting attachment claim ("decorations attach
  to instructions") and does not duplicate the decorations
  bundle's per-decoration assertions.
- `ir-reference/control-flow` covers branches and terminators
  in detail (block parameters, conditional terminators,
  `Require*` markers). This bundle's
  `family-taxonomy-if-stmt-is-control-flow.slang` exercises
  only the family-assignment claim, not the per-terminator
  catalog.

## Out of scope (no-GPU runner)

(none) -- all tests run via `-dump-ir` against text-emit
targets, which require no GPU.

## Doc gaps observed

- The `#how-ast-nodes-lower-to-ir` section names exactly two
  example mappings (`visitVarDecl -> Var`,
  `visitInfixExpr -> Add/Sub/Mul`). Tests in this bundle pin
  both; a third, distinct family of mappings (e.g. an entry-
  point declaration -> `func` with `[entryPoint(...)]`
  decoration) is *implied* but not enumerated. The doc could
  add one or two more concrete examples to strengthen the
  cross-family AST-to-IR mapping claim without growing the
  table.
- The index doc states "Many opcodes have no direct AST source:
  they are produced by IR passes ... and may even be retired
  before code emission." but does not provide a user-observable
  example of a synthesized opcode that survives into a final
  text dump. We attempted a `specialize`-from-pass observation
  in test design but dropped it: by the time `-dump-ir` emits
  the final IR, target legalization has typically removed
  `specialize` instructions. A natural-surface example that
  reliably leaves a synthesized opcode in the final dump would
  make this claim testable at the index level (it is currently
  only testable inside `pipeline/05-ir-passes`).
- The `#cross-cutting-topics` and `#pages` sections are pure
  pointer tables; they are explicitly out of scope per the
  per-section prompt. No gap, just noted for future readers.
