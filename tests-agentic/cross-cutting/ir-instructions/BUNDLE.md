---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-21T12:00:00+00:00
source_commit: 1655c2bf8d3567fa220a5226769ef5e3917d55e8
watched_paths_digest: a7b1c184243cc33ab7365f1e766ae76123f4e9039f529babd0a030cb03949933
source_doc: docs/llm-generated/cross-cutting/ir-instructions.md
source_doc_digest: a0fb638618164f0a2ef326bfc1eda1d1f8d37d916fbe5842e1ae309d082169f9
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for cross-cutting/ir-instructions

## Intent

Tests verify the observable shape of the Slang IR described in
[`docs/llm-generated/cross-cutting/ir-instructions.md`](../../../docs/llm-generated/cross-cutting/ir-instructions.md):
that representative opcodes from each documented family
(`add`/`sub`/`mul`/`div` arithmetic, `cmpGT` comparison, `intCast`
conversion, `var`/`load`/`store`/`get_field_addr`/`swizzle` memory,
`ifElse`/`unconditionalBranch`/`loop`/`return_val` control-flow,
`func`/`struct`/`field`/`witness_table`/`witness_table_entry`/`global_param`
structure, `specialize` for generic instantiation, `entryPoint` /
`nameHint` decorations, `rwstructuredBufferGetElementPtr` /
`global_hashed_string_literals` resource and value opcodes,
`Vec(elementType, elementCount)` and `Array(elementType, elementCount)`
and `Ptr(valueType)` types) appear in the IR for the obvious source
construct that ought to produce them.

The primary observation mechanism is `-target spirv-asm -dump-ir -o
/dev/null` followed by a FileCheck against the IR dump (anchored at
the `### LOWER-TO-IR:` stage, the first dump where opcode spellings
match the doc's "Notes" column most directly). For claims that lower
predictably across text-emit targets (notably `add` → `+`), a
multi-backend test runs the same Slang source through each available
text target and FileChecks the per-target emit.

Implementation-internal claims — opcode-flag bits
(`Hoistable`/`Global`/`Parent`/`UseOther`), the IR-builder's
deduplication decision, the contiguous opcode-range layout that
underlies `as<IRBasicType>()`, the module-version-bump workflow, the
FIDDLE-generated `IROp` enum — are recorded under
`## Out of scope (no-GPU runner)` because they are not observable
through any directive that `slang-test` runs on a CPU.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                       | Claim (one line)                                                                                                  | Tests                                          |
| -------- | ------------------------------------------------------------------------------------------------------------------------------------------------------------ | ----------------------------------------------------------------------------------------------------------------- | ---------------------------------------------- |
| C-01     | [#value-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions)                                                       | `add` is the IR opcode for integer addition; lowers to `+` on every C-like text-emit target.                      | `arithmetic-add-ir.slang`                      |
| C-02     | [#value-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions)                                                       | `sub` is the IR opcode for integer subtraction; lowers to `-` on text-emit targets.                               | `arithmetic-sub-ir.slang`                      |
| C-03     | [#value-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions)                                                       | `mul` is the IR opcode for integer multiplication; lowers to `*` on text-emit targets.                            | `arithmetic-mul-ir.slang`                      |
| C-04     | [#value-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions)                                                       | `div` is the IR opcode for floating-point division.                                                               | `arithmetic-div-ir.slang`                      |
| C-05     | [#value-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions)                                                       | `cmpGT` is the IR opcode for `>` comparison.                                                                      | `comparison-cmpgt-ir.slang`                    |
| C-06     | [#value-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions)                                                       | `intCast` is the IR opcode for an explicit integer cast.                                                          | `conversion-intcast-ir.slang`                  |
| C-07     | [#memory-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#memory-instructions)                                                     | `var`, `load`, `store`, and `get_field_addr` are the memory opcodes for local-variable allocation and field access. | `memory-var-load-store-ir.slang`               |
| C-08     | [#memory-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#memory-instructions)                                                     | `swizzle` performs fixed-position vector element access in the IR.                                                | `memory-getelement-ir.slang`                   |
| C-09     | [#memory-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#memory-instructions)                                                     | A `var` produces a `Ptr<T>`-typed value (the doc's "result is `Ptr<T>`" claim).                                   | `type-ptr-from-var-ir.slang`                   |
| C-10     | [#control-flow-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#control-flow-instructions)                                         | `ifElse` is the conditional-branch terminator; `unconditionalBranch` joins back at the merge block.               | `control-flow-ifelse-ir.slang`                 |
| C-11     | [#control-flow-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#control-flow-instructions)                                         | `loop` is the loop terminator; block `param`s replace SSA `phi`.                                                  | `control-flow-loop-ir.slang`                   |
| C-12     | [#control-flow-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#control-flow-instructions)                                         | `return_val` is the return terminator (with a value, or `void_constant` for a `void` function).                   | `control-flow-return-val-ir.slang`             |
| C-13     | [#function-and-module-structure](../../../docs/llm-generated/cross-cutting/ir-instructions.md#function-and-module-structure)                                 | `func` is the function opcode; its children are basic `block`s; call sites use `call`.                            | `structure-func-ir.slang`                      |
| C-14     | [#type-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#type-instructions)                                                         | A user `struct` lowers to a parent `struct` opcode containing `field` children.                                   | `structure-struct-field-ir.slang`              |
| C-15     | [#function-and-module-structure](../../../docs/llm-generated/cross-cutting/ir-instructions.md#function-and-module-structure)                                 | An interface conformance produces a `witness_table` with `witness_table_entry` rows.                              | `structure-witness-table-ir.slang`             |
| C-16     | [#function-and-module-structure](../../../docs/llm-generated/cross-cutting/ir-instructions.md#function-and-module-structure)                                 | Module-scope `uniform` declarations lower to `global_param`.                                                      | `structure-global-param-ir.slang`              |
| C-17     | [#specialization-and-existentials](../../../docs/llm-generated/cross-cutting/ir-instructions.md#specialization-and-existentials)                             | A generic-function call with a concrete type lowers to a `specialize` opcode.                                     | `specialization-specialize-ir.slang`           |
| C-18     | [#decorations](../../../docs/llm-generated/cross-cutting/ir-instructions.md#decorations)                                                                     | An entry-point function carries `entryPoint(...)` in the IR and emits as `OpEntryPoint` on SPIR-V.                | `decoration-entry-point-ir.slang`              |
| C-19     | [#decorations](../../../docs/llm-generated/cross-cutting/ir-instructions.md#decorations)                                                                     | User-named declarations carry `nameHint("...")` decorations in the IR.                                            | `decoration-name-hint-ir.slang`                |
| C-20     | [#resource-and-shader-io-opcodes](../../../docs/llm-generated/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes)                               | An `RWStructuredBuffer` write lowers through `rwstructuredBufferGetElementPtr` + `store`.                          | `resource-buffer-getelementptr-ir.slang`       |
| C-21     | [#value-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions)                                                       | A string literal appears as a `StringLit` payload (via `getStringHash`) and the module records it under `global_hashed_string_literals`. | `value-string-lit-hash-ir.slang`               |
| C-22     | [#type-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#type-instructions)                                                         | A vector parameter is typed `Vec(elementType, elementCount)` in the IR.                                           | `type-vector-ir.slang`                         |
| C-23     | [#type-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#type-instructions)                                                         | A fixed-size array type appears as `Array(elementType, elementCount)`.                                            | `type-array-ir.slang`                          |
| C-24     | [#control-flow-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#control-flow-instructions)                                         | `discard` is a return/exit terminator in the `TerminatorInst` family.                                             | `control-flow-discard-ir.slang`                |
| C-25     | [#control-flow-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#control-flow-instructions)                                         | `unreachable` is the exit terminator for synthesized join blocks whose predecessors all return.                   | `control-flow-unreachable-ir.slang`            |
| C-26     | [#memory-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#memory-instructions)                                                     | `getElement` is the rvalue indexed-access opcode, operand shape `(base, index)`.                                  | `memory-get-element-array.slang`               |
| C-27     | [#resource-and-shader-io-opcodes](../../../docs/llm-generated/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes)                               | `structuredBufferLoad` is the read-only sibling of `rwstructuredBufferStore` for `StructuredBuffer<T>` reads.     | `resource-structured-buffer-load-ir.slang`     |
| C-28     | [#function-and-module-structure](../../../docs/llm-generated/cross-cutting/ir-instructions.md#function-and-module-structure)                                 | `global_var` is the module-scope storage opcode for mutable `static` module-scope variables.                      | `structure-global-var-ir.slang`                |
| C-29     | [#specialization-and-existentials](../../../docs/llm-generated/cross-cutting/ir-instructions.md#specialization-and-existentials)                             | `lookupWitness(witnessTable, requirementKey)` resolves an interface requirement to the concrete satisfying value. | `specialization-lookup-witness-ir.slang`       |
| C-30     | [#specialization-and-existentials](../../../docs/llm-generated/cross-cutting/ir-instructions.md#specialization-and-existentials)                             | `makeExistential(value, witness)` packs a concrete value plus its witness table into an existential.              | `specialization-make-existential-ir.slang`     |
| C-31     | [#specialization-and-existentials](../../../docs/llm-generated/cross-cutting/ir-instructions.md#specialization-and-existentials)                             | `extractExistentialValue` / `extractExistentialType` / `extractExistentialWitnessTable` project an existential.   | `specialization-extract-existential-ir.slang`  |
| C-32     | [#function-and-module-structure](../../../docs/llm-generated/cross-cutting/ir-instructions.md#function-and-module-structure)                                 | `generic` is the type-level-computation parent opcode (C++ wrapper `IRGeneric`).                                  | `structure-generic-ir.slang`                   |
| C-33     | [#type-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#type-instructions)                                                         | `interface` is the parent type opcode (C++ wrapper `InterfaceType`) whose children are `interface_req_entry`.     | `type-interface-ir.slang`                      |
| C-34     | [#type-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#type-instructions)                                                         | `class` is the parent type opcode (C++ wrapper `ClassType`) for reference-typed user declarations.                | `type-class-ir.slang`                          |
| C-35     | [#type-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#type-instructions)                                                         | `Bool` is one of the basic scalar type opcodes (C++ wrapper `BoolType`).                                          | `type-bool-ir.slang`                           |
| C-36     | [#type-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#type-instructions)                                                         | `Texture` is the type opcode for textures, operand `(elementType, shape, isArray, isMS, sampleCount, ...)`.        | `type-texture2d-ir.slang`                      |
| C-37     | [#value-instructions](../../../docs/llm-generated/cross-cutting/ir-instructions.md#value-instructions)                                                       | `makeVector` is the aggregate-constructor value opcode for vector literals.                                       | `value-make-vector-ir.slang`                   |
| C-38     | [#decorations](../../../docs/llm-generated/cross-cutting/ir-instructions.md#decorations)                                                                     | Source-level function attributes lower to IR `[<name>(...)]` decoration rows (e.g. `numThreads` from `[numthreads]`). | `decoration-num-threads-ir.slang`           |

## Tests in this bundle

| File                                       | Intent     | Doc anchor                            |
| ------------------------------------------ | ---------- | ------------------------------------- |
| `arithmetic-add-ir.slang`                  | functional | `#value-instructions`                 |
| `arithmetic-sub-ir.slang`                  | functional | `#value-instructions`                 |
| `arithmetic-mul-ir.slang`                  | functional | `#value-instructions`                 |
| `arithmetic-div-ir.slang`                  | functional | `#value-instructions`                 |
| `comparison-cmpgt-ir.slang`                | functional | `#value-instructions`                 |
| `conversion-intcast-ir.slang`              | functional | `#value-instructions`                 |
| `memory-var-load-store-ir.slang`           | functional | `#memory-instructions`                |
| `memory-getelement-ir.slang`               | functional | `#memory-instructions`                |
| `type-ptr-from-var-ir.slang`               | functional | `#memory-instructions`                |
| `control-flow-ifelse-ir.slang`             | functional | `#control-flow-instructions`          |
| `control-flow-loop-ir.slang`               | functional | `#control-flow-instructions`          |
| `control-flow-return-val-ir.slang`         | functional | `#control-flow-instructions`          |
| `structure-func-ir.slang`                  | functional | `#function-and-module-structure`      |
| `structure-struct-field-ir.slang`          | functional | `#type-instructions`                  |
| `structure-witness-table-ir.slang`         | functional | `#function-and-module-structure`      |
| `structure-global-param-ir.slang`          | functional | `#function-and-module-structure`      |
| `specialization-specialize-ir.slang`       | functional | `#specialization-and-existentials`    |
| `decoration-entry-point-ir.slang`          | functional | `#decorations`                        |
| `decoration-name-hint-ir.slang`            | functional | `#decorations`                        |
| `resource-buffer-getelementptr-ir.slang`   | functional | `#resource-and-shader-io-opcodes`     |
| `value-string-lit-hash-ir.slang`           | functional | `#value-instructions`                 |
| `type-vector-ir.slang`                     | functional | `#type-instructions`                  |
| `type-array-ir.slang`                      | functional | `#type-instructions`                  |
| `arithmetic-add-uint32-max-overflow.slang` | boundary   | `#value-instructions`                 |
| `arithmetic-sub-int-min-literal.slang`     | boundary   | `#value-instructions`                 |
| `arithmetic-mul-by-zero-literal.slang`     | boundary   | `#value-instructions`                 |
| `arithmetic-div-by-positive-zero-float.slang` | boundary | `#value-instructions`               |
| `arithmetic-add-float-positive-inf.slang`  | boundary   | `#value-instructions`                 |
| `arithmetic-add-float-negative-inf.slang`  | boundary   | `#value-instructions`                 |
| `arithmetic-neg-int.slang`                 | boundary   | `#value-instructions`                 |
| `comparison-cmpeq-ir.slang`                | boundary   | `#value-instructions`                 |
| `comparison-cmpne-ir.slang`                | boundary   | `#value-instructions`                 |
| `comparison-cmplt-ir.slang`                | boundary   | `#value-instructions`                 |
| `comparison-cmple-ir.slang`                | boundary   | `#value-instructions`                 |
| `comparison-cmpge-ir.slang`                | boundary   | `#value-instructions`                 |
| `comparison-nan-greater-than-self.slang`   | boundary   | `#value-instructions`                 |
| `conversion-bitcast-float-to-uint.slang`   | boundary   | `#value-instructions`                 |
| `conversion-floatcast-to-int.slang`        | boundary   | `#value-instructions`                 |
| `conversion-intcast-int-to-uint.slang`     | boundary   | `#value-instructions`                 |
| `buffer-write-runtime-index.slang`         | boundary   | `#resource-and-shader-io-opcodes`     |
| `buffer-aliased-write-and-read.slang`      | boundary   | `#resource-and-shader-io-opcodes`     |
| `atomic-add-stress.slang`                  | stress     | `#resource-and-shader-io-opcodes`     |
| `control-flow-switch-multi-case.slang`     | boundary   | `#control-flow-instructions`          |
| `control-flow-empty-body.slang`            | boundary   | `#control-flow-instructions`          |
| `control-flow-nested-loop.slang`           | stress     | `#control-flow-instructions`          |
| `type-matrix-float-4x4.slang`              | boundary   | `#type-instructions`                  |
| `type-vector-length-one.slang`             | boundary   | `#type-instructions`                  |
| `type-array-length-one.slang`              | boundary   | `#type-instructions`                  |
| `function-call-high-arity.slang`           | stress     | `#function-and-module-structure`      |
| `memory-load-struct-from-buffer.slang`     | boundary   | `#memory-instructions`                |
| `memory-swizzle-single-element.slang`      | boundary   | `#memory-instructions`                |
| `memory-swizzle-full-reverse.slang`        | boundary   | `#memory-instructions`                |
| `specialize-two-type-arguments.slang`      | boundary   | `#specialization-and-existentials`    |
| `structure-empty-struct.slang`             | boundary   | `#type-instructions`                  |
| `structure-global-constant.slang`          | boundary   | `#function-and-module-structure`      |
| `negative-arithmetic-add-struct-no-overload.slang` | negative | `#value-instructions`           |
| `negative-bitcast-size-mismatch.slang`     | negative   | `#value-instructions`                 |
| `control-flow-discard-ir.slang`            | boundary   | `#control-flow-instructions`          |
| `control-flow-unreachable-ir.slang`        | boundary   | `#control-flow-instructions`          |
| `control-flow-block-with-param.slang`      | expansion  | `#control-flow-instructions`          |
| `memory-get-element-array.slang`           | boundary   | `#memory-instructions`                |
| `memory-swizzle-two-elements.slang`        | boundary   | `#memory-instructions`                |
| `memory-ptr-from-var-int.slang`            | boundary   | `#memory-instructions`                |
| `resource-structured-buffer-load-ir.slang` | expansion  | `#resource-and-shader-io-opcodes`     |
| `resource-structured-buffer-load-runtime-index.slang` | boundary | `#resource-and-shader-io-opcodes` |
| `structure-global-var-ir.slang`            | boundary   | `#function-and-module-structure`      |
| `structure-generic-ir.slang`               | expansion  | `#function-and-module-structure`      |
| `structure-witness-table-entry-ir.slang`   | boundary   | `#function-and-module-structure`      |
| `structure-call-void-return.slang`         | expansion  | `#function-and-module-structure`      |
| `specialization-lookup-witness-ir.slang`   | expansion  | `#specialization-and-existentials`    |
| `specialization-make-existential-ir.slang` | expansion  | `#specialization-and-existentials`    |
| `specialization-extract-existential-ir.slang` | expansion | `#specialization-and-existentials` |
| `type-interface-ir.slang`                  | expansion  | `#type-instructions`                  |
| `type-class-ir.slang`                      | expansion  | `#type-instructions`                  |
| `type-bool-ir.slang`                       | boundary   | `#type-instructions`                  |
| `type-texture2d-ir.slang`                  | expansion  | `#type-instructions`                  |
| `type-matrix-layout-operand.slang`         | boundary   | `#type-instructions`                  |
| `conversion-float-to-int-cast-ir.slang`    | expansion  | `#value-instructions`                 |
| `conversion-floatcast-double-to-float.slang` | boundary | `#value-instructions`                 |
| `value-make-vector-ir.slang`               | expansion  | `#value-instructions`                 |
| `value-int-lit-hoistable-stress.slang`     | stress     | `#value-instructions`                 |
| `arithmetic-neg-float.slang`               | boundary   | `#value-instructions`                 |
| `decoration-num-threads-ir.slang`          | expansion  | `#decorations`                        |
| `negative-write-to-static-const.slang`     | negative   | `#function-and-module-structure`      |
| `negative-runtime-sized-local-array.slang` | negative   | `#type-instructions`                  |

## Doc gaps observed

- The doc's per-family tables are explicitly "representative, not
  exhaustive". Behaviors that the doc names but does not list a
  specific opcode for — e.g. the full set of comparison opcodes
  (`cmpEQ`/`cmpNE`/`cmpLE`/`cmpGE`), the full set of conversion ops
  (`floatCast`/`bitCast`/`uintCast`), the `matrix` type, the
  `Texture` type — are deferred to the family-specific bundles
  (`ir-reference/values`, `ir-reference/types`) where they belong.
- The doc cites `alloca` with operand `allocSize` but does not name
  a user-facing language surface that lowers to `alloca`. The
  Slang surface for dynamically-sized stack allocation is unclear
  from the doc alone; a one-line note on the source-level construct
  (`alloca` is currently used internally for some lowering paths)
  would let an agent anchor a test here.
- `RequirePrelude`, `RequireTargetExtension`, `Printf`, `StaticAssert`
  appear in the control-flow row but the doc does not state a
  user-observable consequence for each — only that they are "other
  control-flow / backend-hint opcodes". A one-line "user surface"
  column would let the agent test these.
- The doc's `makeExistential` row lists "Packs a value plus its
  witness" but does not state the source-level construct that
  triggers it (assignment of a concrete type to an interface-typed
  variable). With the user surface stated, the test would be
  straightforward.
- The "Decorations" row says the family has "~180 decorations" but
  the doc itself names only four. Coverage of the long tail belongs
  in `ir-reference/decorations`.
- The doc says `param` is "Block or function parameter; replaces SSA
  `phi`." but does not give the surface-level construct that creates
  block params besides loop induction variables — e.g. that the
  back-edge of a `for` loop carries values via block parameters.
  The connection between source-level loops and IR `param`s is
  implicit.
- The arithmetic family is documented as `add`/`sub`/`mul`/`div` but
  the unary-negation member of the same family (`neg`) is not named,
  even though it is the natural 1-operand sibling of the binary
  operators. Adding `neg` to the value-instructions row would let an
  agent anchor a boundary test for unary arithmetic directly.
- The doc's conversion-family entry names `intCast`, `floatCast`, and
  `bitCast` but does not enumerate the float-to-int and int-to-float
  numeric conversion opcodes (`castFloatToInt`, `castIntToFloat`)
  that the front end actually emits for `int(floatExpr)` and
  `float(intExpr)`. A one-line note on the user surface for each
  conversion opcode would be a meaningful expansion.
- The doc's `swizzle` entry does not state the operand shape; the IR
  actually takes `(base, idx0, idx1, ...)` where the variadic-index
  arity equals the resulting vector length. Stating this in the
  memory-instructions row would let agents pin down the lower- vs
  upper-edge of the swizzle-length axis.
- The doc cites `globalConstant` only in passing alongside
  `global_var` / `global_param` but does not name the surface-level
  construct (`static const`) that produces it. With the surface
  named, the `globalConstant` opcode could be tested directly.
- The doc does not document any error or warning text for invalid
  arithmetic / conversion operands; this restricts the bundle's
  negative tests to a small surface (struct-arithmetic, bit-cast
  size mismatch) that survives by general "no overload" /
  "type mismatch" diagnostics rather than by IR-instruction-specific
  rules. Naming a few canonical diagnostic codes (e.g. for the
  bit-width-mismatch rule of `bitCast`) would let agents tie negative
  tests directly to documented behavior.

## Out of scope (no-GPU runner)

(In this bundle the heading is used for "claims unobservable through
any allowed test directive", not literally GPU-bound claims. The doc
is overwhelmingly about IR-internal structure.)

- The flag-bit layout (`kIROpFlag_Parent`, `kIROpFlag_UseOther`,
  `kIROpFlag_Hoistable`, `kIROpFlag_Global`) — internal to the
  `IROpFlags` enum in `slang-ir.h`. Not surfaced in `-dump-ir`
  output.
- The IR builder's deduplication / hoisting decision for hoistable
  opcodes — the IR dump shows the post-hoist result, not the
  decision path. Two textually identical `vector(Int, 3)` types
  appearing once in the dump is observable; the *reason* (dedup)
  is not.
- The contiguous opcode-range allocation that lets
  `as<IRBasicType>()` be a single integer comparison — entirely
  internal to FIDDLE-generated `slang-ir-insts-enum.h.fiddle`.
- The opcode-bit packing (`kIROpMeta::kIROpMeta_OtherShift = 10`,
  high bits store auxiliary info for `UseOther` ops) — internal to
  `IRInst::m_op` layout.
- The module-version bump required when an opcode is inserted in
  the middle of an existing family range — a serialization invariant
  for `.slang-module` files. The user-facing consequences belong to
  `cross-cutting/serialization`.
- The FIDDLE workflow for adding a new opcode — a developer guide
  ("Adding a new opcode" section in the source doc) rather than a
  user-observable behaviour.
- The C++ wrapper-struct identity for each opcode (e.g. that `add`
  is an `IRAdd*` in C++) — internal API.
- The `IRBuilder` emitter helper for each opcode — internal C++ API.
- Whether `Hoistable` instructions actually float to the outermost
  scope where their operands are available — observable only by
  inspecting the IR-dump position of an instruction relative to its
  defining scope, and the dump's textual order does not directly
  encode the scope tree.
