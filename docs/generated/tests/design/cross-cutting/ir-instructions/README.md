---
generated: true
model: claude-opus-4-8
generated_at: 2026-07-02T00:00:00+00:00
source_commit: 8e2a63cbf22c85332796702fd02a875466486a23
watched_paths_digest: d706db8099a56fe3a824f3e46953e73df9fb84b6b42d989737f1d08526f29e97
source_doc: docs/generated/design/cross-cutting/ir-instructions.md
source_doc_digest: 6d95051b87c06dc35373e8bea70f37f8883df3086f06535f1e3cedc75e1d0bda
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for design/cross-cutting/ir-instructions

## Intent

This bundle exercises the **observable shape of the Slang IR** at the
`LOWER-TO-IR` stage as documented in
[ir-instructions.md](../../../design/cross-cutting/ir-instructions.md).
The doc uses **representative, not exhaustive** per-family tables; the
coverage strategy is one test per documented per-family opcode
example — proving the obvious source construct that ought to produce
each opcode does so in the IR dump, plus a cross-target emit fan-out
for the arithmetic opcodes that lower to predictable C-like operators.
The per-opcode catalog detail belongs to the `ir-reference/*` bundles
and the behaviour of individual IR passes to `pipeline/05-ir-passes`;
this bundle intentionally stays at the "which opcode does this
construct produce" level. All `-dump-ir` tests defeat constant folding
with `uniform` inputs and sink results into a buffer so nothing is
DCE'd before the CHECK observes it.

## Claims

Value instructions ([#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions)):

- C1 — `add`/`Add`: an addition of two operands lowers to `add`.
- C2 — `sub`/`Sub`: a subtraction lowers to `sub`.
- C3 — `mul`/`Mul`: a multiplication lowers to `mul`.
- C4 — `div`/`Div`: a division lowers to `div`.
- C5 — comparison family (`cmpEQ`, `cmpLT`, ...): a comparison lowers to the matching `cmp*` opcode.
- C6 — conversion family (`bitCast`, `intCast`, `floatCast`): an explicit cast lowers to the matching conversion opcode.
- C7 — aggregate constructors: building a vector from scalars lowers to `makeVector`.

Memory instructions ([#memory-instructions](../../../design/cross-cutting/ir-instructions.md#memory-instructions)):

- C8 — `var`/`load`/`store`: a local variable and its reads/writes lower to `var`, `load`, `store`.
- C9 — `get_field`/`FieldExtract`: an rvalue field read lowers to `get_field`.
- C10 — `getElement`: indexed access into an aggregate value lowers to `getElement`.
- C11 — `rwstructuredBufferGetElementPtr`: a subscript write into an RWStructuredBuffer forms an element pointer then stores.

Control-flow instructions ([#control-flow-instructions](../../../design/cross-cutting/ir-instructions.md#control-flow-instructions)):

- C12 — `ifElse` + `unconditionalBranch`: an `if/else` lowers to the `ifElse` terminator plus branch joins.
- C13 — `loop` + `param`: a `for` loop lowers to the `loop` terminator and header block `param`s.
- C14 — `return_val`: a value-returning function ends its block with `return_val`.
- C15 — `switch`: a `switch` statement lowers to the `switch` terminator.
- C16 — `block`: a function body opens with a `block`.

Function and module structure ([#function-and-module-structure](../../../design/cross-cutting/ir-instructions.md#function-and-module-structure)):

- C17 — `func`/`IRFunc`: a function declaration lowers to a `func` whose children are blocks.
- C18 — `generic`/`IRGeneric`: a generic function declaration lowers to a `generic` inst.
- C19 — `global_param`: a module-scope shader parameter lowers to `global_param`.
- C20 — `witness_table`/`witness_table_entry`: an interface conformance produces a witness table with entry rows.

Type instructions ([#type-instructions](../../../design/cross-cutting/ir-instructions.md#type-instructions)):

- C21 — `Vec`/`VectorType`: a vector value uses `Vec(elementType, elementCount)`.
- C22 — `Array`/`ArrayType`: a fixed array uses `Array(elementType, elementCount)`.
- C23 — `Mat`/`MatrixType`: a matrix value uses `Mat(elementType, rowCount, columnCount, layout)`.
- C24 — `struct`/`StructType`: a struct declaration is a parent of `field` insts keyed by `key`.

Specialization and existentials ([#specialization-and-existentials](../../../design/cross-cutting/ir-instructions.md#specialization-and-existentials)):

- C25 — `specialize`: calling a generic with a concrete type argument lowers to `specialize`.
- C26 — `lookupWitness`/`LookupWitnessMethod`: an interface-method call inside a generic lowers to `lookupWitness`.
- C27 — `makeExistential`/`MakeExistential`: assigning a conformer to an interface-typed location lowers to `makeExistential`.

Decorations ([#decorations](../../../design/cross-cutting/ir-instructions.md#decorations)):

- C28 — `NameHintDecoration`: a user-named declaration carries a `nameHint` decoration.
- C29 — `EntryPointDecoration`: an entry-point function carries an `entryPoint` decoration and lowers to a target entry-point declaration.

Resource and shader-IO opcodes ([#resource-and-shader-io-opcodes](../../../design/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes)):

- C30 — `structuredBufferLoad`: a read from a read-only `StructuredBuffer<T>` lowers to `structuredBufferLoad`.

## Functional coverage

| Claim                                                                                                                                                    | Intent     | Anchor                                                                                                          | Tests                                                                                  |
| -------------------------------------------------------------------------------------------------------------------------------------------------------- | ---------- | -------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------- |
| A user-named declaration carries a `nameHint` decoration in the IR carrying its identifier name.                                                          | functional | [#decorations](../../../design/cross-cutting/ir-instructions.md#decorations)                                   | [`decoration-name-hint-ir.slang`](decoration-name-hint-ir.slang)                       |
| A function selected as a pipeline entry point carries an `entryPoint` decoration in the IR and lowers to a target entry-point declaration.                 | functional | [#decorations](../../../design/cross-cutting/ir-instructions.md#decorations)                                   | [`decoration-entry-point-ir.slang`](decoration-entry-point-ir.slang)                   |
| A generic function declaration lowers to a `generic` inst (type-level computation parent).                                                                | functional | [#function-and-module-structure](../../../design/cross-cutting/ir-instructions.md#function-and-module-structure) | [`structure-generic-ir.slang`](structure-generic-ir.slang)                             |
| A module-scope shader parameter lowers to a `global_param` inst.                                                                                          | functional | [#function-and-module-structure](../../../design/cross-cutting/ir-instructions.md#function-and-module-structure) | [`structure-global-param-ir.slang`](structure-global-param-ir.slang)                   |
| A source function declaration lowers to a `func` inst whose children are `block`s.                                                                        | functional | [#function-and-module-structure](../../../design/cross-cutting/ir-instructions.md#function-and-module-structure) | [`structure-func-ir.slang`](structure-func-ir.slang)                                   |
| A type that conforms to an interface produces a `witness_table` with `witness_table_entry` children mapping requirements to satisfying values.            | functional | [#function-and-module-structure](../../../design/cross-cutting/ir-instructions.md#function-and-module-structure) | [`structure-witness-table-ir.slang`](structure-witness-table-ir.slang)                 |
| A local struct variable lowers to `var` and its field reads/writes lower to `store` / `load` / `get_field_addr` IR opcodes.                               | functional | [#memory-instructions](../../../design/cross-cutting/ir-instructions.md#memory-instructions)                   | [`memory-var-load-store-ir.slang`](memory-var-load-store-ir.slang)                     |
| An rvalue read of a struct field lowers to the `get_field` opcode (`FieldExtract`).                                                                       | functional | [#memory-instructions](../../../design/cross-cutting/ir-instructions.md#memory-instructions)                   | [`memory-get-field-ir.slang`](memory-get-field-ir.slang)                               |
| Indexed access into an aggregate value lowers to the `getElement` opcode.                                                                                 | functional | [#memory-instructions](../../../design/cross-cutting/ir-instructions.md#memory-instructions)                   | [`memory-getelement-ir.slang`](memory-getelement-ir.slang)                             |
| A write through a subscript into an RWStructuredBuffer lowers to `rwstructuredBufferGetElementPtr` plus a `store`.                                         | functional | [#memory-instructions](../../../design/cross-cutting/ir-instructions.md#memory-instructions)                   | [`resource-rwbuffer-getelementptr-ir.slang`](resource-rwbuffer-getelementptr-ir.slang) |
| An `if`/`else` statement lowers to the `ifElse` terminator plus `unconditionalBranch` joins.                                                              | functional | [#control-flow-instructions](../../../design/cross-cutting/ir-instructions.md#control-flow-instructions)       | [`control-flow-ifelse-ir.slang`](control-flow-ifelse-ir.slang)                         |
| A `for` loop lowers to the `loop` terminator and uses block `param`s to carry the SSA induction value.                                                    | functional | [#control-flow-instructions](../../../design/cross-cutting/ir-instructions.md#control-flow-instructions)       | [`control-flow-loop-ir.slang`](control-flow-loop-ir.slang)                             |
| A value-returning function terminates its block with the `return_val` terminator.                                                                         | functional | [#control-flow-instructions](../../../design/cross-cutting/ir-instructions.md#control-flow-instructions)       | [`control-flow-return-val-ir.slang`](control-flow-return-val-ir.slang)                 |
| A `switch` statement lowers to the `switch` terminator.                                                                                                   | functional | [#control-flow-instructions](../../../design/cross-cutting/ir-instructions.md#control-flow-instructions)       | [`control-flow-switch-ir.slang`](control-flow-switch-ir.slang)                         |
| Calling a generic function with a concrete type argument lowers to a `specialize` IR opcode.                                                              | functional | [#specialization-and-existentials](../../../design/cross-cutting/ir-instructions.md#specialization-and-existentials) | [`specialization-specialize-ir.slang`](specialization-specialize-ir.slang)        |
| Calling an interface method inside a generic lowers to a `lookupWitness` IR opcode.                                                                       | functional | [#specialization-and-existentials](../../../design/cross-cutting/ir-instructions.md#specialization-and-existentials) | [`specialization-lookup-witness-ir.slang`](specialization-lookup-witness-ir.slang) |
| Assigning a concrete conformer to an interface-typed location lowers to a `makeExistential` IR opcode.                                                    | functional | [#specialization-and-existentials](../../../design/cross-cutting/ir-instructions.md#specialization-and-existentials) | [`specialization-make-existential-ir.slang`](specialization-make-existential-ir.slang) |
| A read from a read-only `StructuredBuffer<T>` lowers to the documented `structuredBufferLoad(base, index)` opcode.                                        | functional | [#resource-and-shader-io-opcodes](../../../design/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes) | [`resource-structured-buffer-load-ir.slang`](resource-structured-buffer-load-ir.slang) |
| A struct declaration lowers to a `struct` type parent whose children are `field`s keyed by `key`.                                                         | functional | [#type-instructions](../../../design/cross-cutting/ir-instructions.md#type-instructions)                       | [`structure-struct-field-ir.slang`](structure-struct-field-ir.slang)                   |
| A vector-typed value uses the `Vec` IR type with element-type and element-count operands.                                                                 | functional | [#type-instructions](../../../design/cross-cutting/ir-instructions.md#type-instructions)                       | [`type-vector-ir.slang`](type-vector-ir.slang)                                         |
| A fixed-size array type uses the `Array` IR type with element-type and element-count operands.                                                            | functional | [#type-instructions](../../../design/cross-cutting/ir-instructions.md#type-instructions)                       | [`type-array-ir.slang`](type-array-ir.slang)                                           |
| A matrix-typed value uses the `Mat` IR type with element-type, row-count, column-count and layout operands.                                               | functional | [#type-instructions](../../../design/cross-cutting/ir-instructions.md#type-instructions)                       | [`type-matrix-ir.slang`](type-matrix-ir.slang)                                         |
| An integer addition expression lowers to the `add` IR opcode and emits as `+` on every C-like text-emit target.                                           | functional | [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions)                     | [`arithmetic-add-ir.slang`](arithmetic-add-ir.slang)                                   |
| An integer subtraction expression lowers to the `sub` IR opcode and emits as `-` on C-like text-emit targets.                                             | functional | [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions)                     | [`arithmetic-sub-ir.slang`](arithmetic-sub-ir.slang)                                   |
| An integer multiplication expression lowers to the `mul` IR opcode and emits as `*` on C-like text-emit targets.                                          | functional | [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions)                     | [`arithmetic-mul-ir.slang`](arithmetic-mul-ir.slang)                                   |
| An integer division expression lowers to the `div` IR opcode.                                                                                             | functional | [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions)                     | [`arithmetic-div-ir.slang`](arithmetic-div-ir.slang)                                   |
| A greater-than comparison lowers to a comparison opcode in the `cmpEQ`/`cmpLT`/... family.                                                                 | functional | [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions)                     | [`comparison-cmpgt-ir.slang`](comparison-cmpgt-ir.slang)                               |
| An equality comparison lowers to the `cmpEQ` comparison opcode.                                                                                           | functional | [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions)                     | [`comparison-cmpeq-ir.slang`](comparison-cmpeq-ir.slang)                               |
| An explicit integer-width cast lowers to the `intCast` conversion opcode.                                                                                 | functional | [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions)                     | [`conversion-intcast-ir.slang`](conversion-intcast-ir.slang)                           |
| An explicit float-width cast lowers to the `floatCast` conversion opcode.                                                                                 | functional | [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions)                     | [`conversion-floatcast-ir.slang`](conversion-floatcast-ir.slang)                       |
| A `bit_cast` bit reinterpretation lowers to the `bitCast` conversion opcode.                                                                              | functional | [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions)                     | [`conversion-bitcast-ir.slang`](conversion-bitcast-ir.slang)                           |
| Constructing a vector from scalar components lowers to a `makeVector` aggregate-constructor opcode.                                                        | functional | [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions)                     | [`value-make-vector-ir.slang`](value-make-vector-ir.slang)                             |

## Untested claims

| Claim                                                                                                                                                     | Reason                | Anchor                                                                                                                             | Why untested                                                                                                                                                |
| --------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------- | --------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------- |
| The `Parent`, `Hoistable`, `Global`, and `UseOther` flag bits are packed into the opcode word (`kIROpMeta_OtherShift = 10`).                              | internal-source-fact  | [#hoistable--global--deduplicated-values](../../../design/cross-cutting/ir-instructions.md#hoistable--global--deduplicated-values) | The flag-bit layout is internal to `IRInst`'s op encoding; no `-dump-ir` or emitted text reveals the bit positions or the meta shift.                        |
| A `hoistable` inst is deduplicated and floated up to the outermost scope where its operands are defined.                                                   | implementation-detail | [#hoistable--global--deduplicated-values](../../../design/cross-cutting/ir-instructions.md#hoistable--global--deduplicated-values) | The IR dump shows the post-hoist result, not the hoisting decision; comparing two uses of the same `Vec(Int,3)` inst is not surface-visible.                 |
| Child opcodes of a parent (`Void`/`Bool`/`Int` under `BasicType`) are allocated in a contiguous range so `as<IRBasicType>()` is a single integer compare. | internal-source-fact  | [#schema](../../../design/cross-cutting/ir-instructions.md#schema)                                                                | The contiguous-range allocation is produced by the FIDDLE generator from the Lua hierarchy; no compiler output exposes an opcode's integer value.            |
| Inserting a new opcode renumbers downstream entries and requires bumping `k_minSupportedModuleVersion`/`k_maxSupportedModuleVersion`.                       | internal-source-fact  | [#module-versioning-and-opcode-insertion](../../../design/cross-cutting/ir-instructions.md#module-versioning-and-opcode-insertion) | A build-system / serialization invariant; user-facing module deserialization is covered in the `cross-cutting/serialization` bundle.                        |
| The seven-step "adding a new opcode" workflow (edit Lua, declare wrapper, choose flags, add builder, update lowering, extend emit, add tests).             | process-doc           | [#adding-a-new-opcode](../../../design/cross-cutting/ir-instructions.md#adding-a-new-opcode)                                       | A contributor developer guide, not a user-observable compiler behavior.                                                                                     |
| A decoration is attached to its host instruction's decoration list (reached via `getFirstDecoration`) rather than sitting in a block's instruction stream. | internal-source-fact  | [#decorations](../../../design/cross-cutting/ir-instructions.md#decorations)                                                      | The attachment mechanism is a C++ traversal fact; the IR dump renders decorations inline above their host but does not expose the decoration-list linkage.   |
| Image / texture / atomic / barrier / wave / raytracing resource opcodes (`imageLoad`, `atomicAdd`, `ControlBarrier`, `waveMaskBallot`, ...).               | out-of-bundle         | [#resource-and-shader-io-opcodes](../../../design/cross-cutting/ir-instructions.md#resource-and-shader-io-opcodes)                | The doc's resource table is representative; the per-opcode catalog for these families lives in the `ir-reference/resources-and-atomics` bundle.              |

## Doc gaps observed

| Anchor                                                                                     | Kind            | Gap                                                                                                                                                                                                                                                                                     | Suggested addition                                                                                                                                             |
| ------------------------------------------------------------------------------------------ | --------------- | --------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions) | missing-surface | The `bitCast`/`intCast`/`floatCast` row names the conversion opcodes but not which Slang surface produces each. `bit_cast<T>(x)` produces `bitCast`; constructor-style casts produce `intCast`/`floatCast`; `asuint`/`reinterpret` lower to their own `call`/`reinterpret` forms instead. | Add a "Slang surface" column to the conversion row naming `bit_cast<T>()` for `bitCast` and constructor casts (`int(x)`, `float(y)`) for `intCast`/`floatCast`. |
| [#value-instructions](../../../design/cross-cutting/ir-instructions.md#value-instructions) | missing-surface | The Value-instruction section note mentions "aggregate constructors" but names no opcode; a `float3(a,b,c)` constructor produces `makeVector` and an array initializer produces `makeArray`, neither of which appears in the table.                                                        | Add explicit `makeVector` and `makeArray` rows with their constructor surfaces to the value-instruction table.                                                 |

## Sibling-bundle overlap

The per-family opcode catalog (every opcode in each family with operand
shape and AST origin) is owned by the `ir-reference/*` bundles
(`types`, `values`, `control-flow`, `structure`,
`generics-and-existentials`, `resources-and-atomics`, `decorations`,
`differentiation`, `metadata`, `misc`). This bundle deliberately tests
only the one representative opcode per family that the
`ir-instructions.md` overview names, to avoid duplicating that
catalog-level coverage.
