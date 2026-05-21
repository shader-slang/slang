---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-21T20:00:00+00:00
source_commit: 690f6a3084801386b77186394e0f6e8c120824a4
watched_paths_digest: 3dbc65f6df020239a91b769be66fa5d3b9d250fac6a4422fc500ef569caf3235
source_doc: docs/llm-generated/pipeline/04-ast-to-ir.md
source_doc_digest: 2786b3ac65f55d3fefdde03d0c432f013d0d8c9686304746f06a7f61fab79197
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for pipeline/04-ast-to-ir

## Intent

Tests verify the AST → IR lowering claims described in
[`docs/llm-generated/pipeline/04-ast-to-ir.md`](../../../docs/llm-generated/pipeline/04-ast-to-ir.md):
that each documented AST node family lowers to the IR construct the
doc names (`FuncDecl` → `IRFunc`, `VarDecl` → `IRVar`/`IRGlobalVar`,
`StructDecl` → `IRStructType` with `IRStructField` children,
`InterfaceDecl` → `IRInterfaceType` + per-method requirement insts,
`GenericDecl` → `IRGeneric`, `BlockStmt` → basic blocks, structured
branches `IfStmt`/`ForStmt`/`WhileStmt`/`SwitchStmt` → `ifElse` /
`loop` / `switch` terminators with explicit join operand,
`ReturnStmt` → `return_val`, `BinaryExpr` → pure value insts,
`InvokeExpr` → `call`, `MemberExpr` → `get_field` / `get_field_addr`
depending on rvalue/lvalue context, `LiteralExpr` → constant inst,
`WitnessTable` → `IRWitnessTable`), that phi-style joining is
encoded as block parameters rather than explicit `phi`, that
specialization is **deferred** to a later IR pass (a generic call
appears as `call specialize(%generic, <Type>)(...)`, not as an
inlined body), and that the lowering step excludes IR for `import`ed
modules (those flow through `[import("...")]` decorations and are
linked in later).

The primary observation mechanism is `-target spirv-asm -dump-ir -o
/dev/null` followed by a FileCheck against the IR dump, anchored on
the user-named function (`func %main` or a helper) to cut through the
preamble of capability sets, core-module imports, and
differentiation glue. All claims here are target-independent IR-shape
claims, so the bundle uses a single SPIR-V `-dump-ir` directive per
test rather than multi-target SIMPLE — the doc never asserts a
target-conditional lowering behavior.

Cross-cutting decoration / opcode catalog coverage (`entryPoint`,
`nameHint`, the per-opcode catalog shape) lives in
[`cross-cutting/ir-instructions`](../../cross-cutting/ir-instructions/);
this bundle anchors on the AST-side of the mapping ("which AST node
lowers to which IR construct").

## Claims enumerated

| Claim ID | Anchor                                                                                                                                          | Claim (one line)                                                                                                                                | Tests                                                          |
| -------- | ----------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------------- | -------------------------------------------------------------- |
| C-01     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `FuncDecl` lowers to an `IRFunc` containing one or more `IRBlock`s; a call site lowers to a `call` instruction.                               | [`funcdecl-lowers-to-irfunc.slang`](funcdecl-lowers-to-irfunc.slang)                              |
| C-02     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A local `VarDecl` whose address is taken (struct field access) lowers to an `IRVar` allocated inside the enclosing block.                       | [`vardecl-local-struct-lowers-to-irvar.slang`](vardecl-local-struct-lowers-to-irvar.slang)                   |
| C-03     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A module-scope `uniform VarDecl` lowers to an `IRGlobalVar` (spelled `global_param` in the dump).                                               | [`vardecl-global-uniform-lowers-to-global-param.slang`](vardecl-global-uniform-lowers-to-global-param.slang)          |
| C-04     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `StructDecl` lowers to an `IRStructType` containing `IRStructField` children.                                                                 | [`structdecl-lowers-to-struct-with-fields.slang`](structdecl-lowers-to-struct-with-fields.slang)                |
| C-05     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | An `InterfaceDecl` lowers to an `IRInterfaceType` plus per-method `interface_req_entry` instructions.                                           | [`interfacedecl-lowers-to-interface-with-req-entries.slang`](interfacedecl-lowers-to-interface-with-req-entries.slang)     |
| C-06     | [#generics-and-existentials](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#generics-and-existentials)                                    | A `GenericDecl` lowers to an `IRGeneric` (function-shaped instruction whose body returns the inner function).                                   | [`genericdecl-lowers-to-irgeneric.slang`](genericdecl-lowers-to-irgeneric.slang)                        |
| C-07     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | An `IfStmt` lowers to an `ifElse` structured-branch terminator whose merge point is an explicit operand.                                        | [`ifstmt-lowers-to-ifelse.slang`](ifstmt-lowers-to-ifelse.slang)                                |
| C-08     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `ForStmt` lowers to a `loop` terminator; the induction variable is carried as a block `param`.                                                | [`forstmt-lowers-to-loop-with-block-params.slang`](forstmt-lowers-to-loop-with-block-params.slang)               |
| C-09     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `WhileStmt` lowers to a `loop` structured-branch terminator (same opcode as for-loop).                                                        | [`whilestmt-lowers-to-loop.slang`](whilestmt-lowers-to-loop.slang)                               |
| C-10     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `SwitchStmt` lowers to a `switch` terminator with explicit case-value/case-block operand pairs and a merge operand.                           | [`switchstmt-lowers-to-switch.slang`](switchstmt-lowers-to-switch.slang)                            |
| C-11     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `ReturnStmt` lowers to an `IRReturn` terminator (spelled `return_val`).                                                                       | [`returnstmt-lowers-to-return-val.slang`](returnstmt-lowers-to-return-val.slang)                        |
| C-12     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `BinaryExpr` arithmetic site lowers to the corresponding pure-value IR opcode (`add`).                                                        | [`binaryexpr-arithmetic-lowers-to-add.slang`](binaryexpr-arithmetic-lowers-to-add.slang)                    |
| C-13     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `BinaryExpr` comparison site lowers to a comparison IR opcode (`cmpGT` for `>`).                                                              | [`binaryexpr-comparison-lowers-to-cmpgt.slang`](binaryexpr-comparison-lowers-to-cmpgt.slang)                  |
| C-14     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | An `InvokeExpr` (function call) lowers to an `IRCall` (spelled `call %<callee>(...)`).                                                          | [`invokeexpr-lowers-to-call.slang`](invokeexpr-lowers-to-call.slang)                              |
| C-15     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `MemberExpr` in rvalue position lowers to `IRFieldExtract` (spelled `get_field`).                                                             | [`memberexpr-rvalue-lowers-to-get-field.slang`](memberexpr-rvalue-lowers-to-get-field.slang)                  |
| C-16     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `MemberExpr` used as an assignment target lowers to `IRFieldAddress` (spelled `get_field_addr`) followed by `store`.                          | [`memberexpr-lvalue-lowers-to-get-field-addr.slang`](memberexpr-lvalue-lowers-to-get-field-addr.slang)             |
| C-17     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `LiteralExpr` lowers to a constant IR instruction visible inline as the literal value.                                                        | [`literalexpr-lowers-to-constant.slang`](literalexpr-lowers-to-constant.slang)                         |
| C-18     | [#generics-and-existentials](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#generics-and-existentials)                                    | A user-synthesized witness table lowers to an `IRWitnessTable` with `witness_table_entry` children.                                             | [`witness-table-lowers-to-ir-witness-table.slang`](witness-table-lowers-to-ir-witness-table.slang)               |
| C-19     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `BlockStmt` containing branching control flow lowers to multiple basic blocks inside the enclosing function.                                  | [`blockstmt-lowers-to-basic-blocks.slang`](blockstmt-lowers-to-basic-blocks.slang)                       |
| C-20     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Structured `ifElse` encodes its join (merge) point as an explicit operand; both branches reach it via `unconditionalBranch`.                    | [`ifelse-merge-block-is-explicit-operand.slang`](ifelse-merge-block-is-explicit-operand.slang)                 |
| C-21     | [#module-level-outputs](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#module-level-outputs)                                              | A `FuncDecl` registered as an entry point appears in the lowered module with an `entryPoint(...)` decoration on its IRFunc.                     | [`entry-point-funcdecl-carries-entry-point-decoration.slang`](entry-point-funcdecl-carries-entry-point-decoration.slang)    |
| C-22     | [#generics-and-existentials](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#generics-and-existentials)                                    | Lowering does not specialize generics; a generic call appears as `call specialize(%generic, <Type>)(...)`.                                      | [`specialization-deferred-to-ir-pass.slang`](specialization-deferred-to-ir-pass.slang)                     |
| C-23     | [#inputs-and-outputs](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#inputs-and-outputs)                                                  | References to symbols defined in imported modules appear with `[import("...")]` decorations rather than full bodies in the lowered IR.          | [`import-not-lowered-into-translation-unit.slang`](import-not-lowered-into-translation-unit.slang)               |
| C-24     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | An explicit constructor-style cast `int(x)` / `float(y)` lowers to a dedicated conversion IR opcode (`castFloatToInt` / `castIntToFloat`).      | [`conversion-explicit-cast-lowers-to-castfloattoint.slang`](conversion-explicit-cast-lowers-to-castfloattoint.slang)      |
| C-25     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Assignment to a struct field lowers to a `store(%addr, %v)`; reading the field back lowers to a `load(%addr)`.                                  | [`load-store-from-local-pointer.slang`](load-store-from-local-pointer.slang)                          |
| C-26     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Phi-style joining is encoded as block parameters (`param` at the start of a block); branches carry the next values as arguments, not via a `phi`. | [`block-param-replaces-phi.slang`](block-param-replaces-phi.slang)                               |
| C-27     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A subscript-assignment on an `RWStructuredBuffer<T>` lowers to `rwstructuredBufferGetElementPtr` followed by `store`.                           | [`rwbuffer-write-lowers-to-getelementptr-store.slang`](rwbuffer-write-lowers-to-getelementptr-store.slang)           |
| C-28     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `return` from a void-returning function lowers to `return_val(void_constant)`.                                                                | [`return-void-uses-void-constant.slang`](return-void-uses-void-constant.slang)                         |
| C-29     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | The "Pure value insts (IRAdd, IRMul, IREq, ...)" row covers all integer arithmetic and comparison operators: `+/-/*///%` and `==/!=/</>`.       | `binaryexpr-{sub,mul,div,mod,eq,lt,ne,bitand,shl}-*.slang`     |
| C-30     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Unary arithmetic / bitwise operators (`-a`, `~a`) lower to single-operand pure-value insts in the same family (`neg`, `bitnot`).                | `unaryexpr-{neg,bitnot}-*.slang`                               |
| C-31     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | The "InvokeExpr -> IRCall" row covers method-call expressions and self-recursive calls as well as free-function calls.                          | `invokeexpr-method-call-...slang`, `funcdecl-recursive-*.slang` |
| C-32     | [#module-level-outputs](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#module-level-outputs)                                              | Only FuncDecls registered as entry points carry an `entryPoint(...)` decoration; non-entry helpers in the same translation unit do not.         | [`non-entry-funcdecl-lacks-entry-point-decoration.slang`](non-entry-funcdecl-lacks-entry-point-decoration.slang)        |
| C-33     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | The element-type axis of the "VarDecl(global) -> IRGlobalVar" row carries each narrow integer (`Int8`/`UInt8`/`Int16`/`UInt16`/`Int64`/`UInt64`) through to the IR-side `global_param` type. | `lower-{int8,uint8,int16,uint16,int64,uint64}-scalar.slang`     |
| C-34     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Narrow integer element types compose with the vector/array/struct-field/buffer-element storage forms — `Vec(IntN, ...)`, `Array(IntN, ...)`, `field(..., IntN)`, `Ptr(IntN)`. | `lower-{int8,uint16,int64}-vector.slang`, `lower-{int8,uint64}-array.slang`, `lower-{int16,uint8}-struct-field.slang`, [`lower-int64-buffer-element.slang`](lower-int64-buffer-element.slang), [`lower-int8-arithmetic.slang`](lower-int8-arithmetic.slang) |
| C-35     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | `half` and `double` lower with IR types `Half` and `Double`, and compose with the vector/matrix/array/struct-field storage forms.               | `lower-half-{scalar,arithmetic,struct-field,matrix,array}.slang`, [`lower-half3-vector.slang`](lower-half3-vector.slang), `lower-double-{scalar,arithmetic,struct-field,matrix,array}.slang`, [`lower-double3-vector.slang`](lower-double3-vector.slang) |
| C-36     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Rectangular matrices (`R != C`) lower to `Mat(<elem>, R : Int, C : Int, 0 : Int)`, preserving row-then-column operand order regardless of orientation or element precision. | [`lower-mat2x4-rectangular.slang`](lower-mat2x4-rectangular.slang), [`lower-mat4x2-rectangular.slang`](lower-mat4x2-rectangular.slang), [`lower-mat-half-rectangular.slang`](lower-mat-half-rectangular.slang), [`lower-mat-double-rectangular.slang`](lower-mat-double-rectangular.slang) |
| C-37     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Composite generic types `Optional<T>` lower to `Optional(<lowered T>)`, and `Tuple<...>` lowers to `tuple_type(...)` with one operand per element type, including narrow precisions. | `lower-optional-{int,half3}.slang`, `lower-tuple-{heterogeneous,narrow-pair}.slang` |
| C-38     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Struct-of-arrays and array-of-structs produce visibly distinct IR layouts: the former has fields whose IR types are `Array(...)`; the latter has IR type `Array(%Struct, N : Int)` at the storage site. | [`lower-struct-of-arrays.slang`](lower-struct-of-arrays.slang), [`lower-array-of-structs.slang`](lower-array-of-structs.slang), [`lower-struct-of-mixed-precision.slang`](lower-struct-of-mixed-precision.slang) |
| C-39     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `ParameterBlock<Narrow>` over a struct of narrow types lowers to `ParameterBlock(%Narrow)`, and `RWStructuredBuffer<Narrow>` to a per-element `Ptr(%Narrow)` — both preserving the inner struct's narrow-typed field rows. | [`lower-parameter-block-narrow-struct.slang`](lower-parameter-block-narrow-struct.slang), [`lower-buffer-of-struct-with-narrow-types.slang`](lower-buffer-of-struct-with-narrow-types.slang) |
| C-40     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Explicit cast lowerings split by source/dest family: `floatCast(...)` for float-to-float (half<->float, double<->float), `intCast(...)` for int-to-int (int8/int64<->int).                                | `lower-cast-{half,double}-to-float.slang`, `lower-cast-{int8,int64}-to-int.slang` |

## Tests in this bundle

| File                                                          | Intent     | Doc anchor                            |
| ------------------------------------------------------------- | ---------- | ------------------------------------- |
| [`funcdecl-lowers-to-irfunc.slang`](funcdecl-lowers-to-irfunc.slang)                             | functional | `#mapping-ast-constructs-to-ir`       |
| [`vardecl-local-struct-lowers-to-irvar.slang`](vardecl-local-struct-lowers-to-irvar.slang)                  | functional | `#mapping-ast-constructs-to-ir`       |
| [`vardecl-global-uniform-lowers-to-global-param.slang`](vardecl-global-uniform-lowers-to-global-param.slang)         | functional | `#mapping-ast-constructs-to-ir`       |
| [`structdecl-lowers-to-struct-with-fields.slang`](structdecl-lowers-to-struct-with-fields.slang)               | functional | `#mapping-ast-constructs-to-ir`       |
| [`interfacedecl-lowers-to-interface-with-req-entries.slang`](interfacedecl-lowers-to-interface-with-req-entries.slang)    | functional | `#mapping-ast-constructs-to-ir`       |
| [`genericdecl-lowers-to-irgeneric.slang`](genericdecl-lowers-to-irgeneric.slang)                       | functional | `#generics-and-existentials`          |
| [`ifstmt-lowers-to-ifelse.slang`](ifstmt-lowers-to-ifelse.slang)                               | functional | `#mapping-ast-constructs-to-ir`       |
| [`forstmt-lowers-to-loop-with-block-params.slang`](forstmt-lowers-to-loop-with-block-params.slang)              | functional | `#mapping-ast-constructs-to-ir`       |
| [`whilestmt-lowers-to-loop.slang`](whilestmt-lowers-to-loop.slang)                              | functional | `#mapping-ast-constructs-to-ir`       |
| [`switchstmt-lowers-to-switch.slang`](switchstmt-lowers-to-switch.slang)                           | functional | `#mapping-ast-constructs-to-ir`       |
| [`returnstmt-lowers-to-return-val.slang`](returnstmt-lowers-to-return-val.slang)                       | functional | `#mapping-ast-constructs-to-ir`       |
| [`binaryexpr-arithmetic-lowers-to-add.slang`](binaryexpr-arithmetic-lowers-to-add.slang)                   | functional | `#mapping-ast-constructs-to-ir`       |
| [`binaryexpr-comparison-lowers-to-cmpgt.slang`](binaryexpr-comparison-lowers-to-cmpgt.slang)                 | functional | `#mapping-ast-constructs-to-ir`       |
| [`invokeexpr-lowers-to-call.slang`](invokeexpr-lowers-to-call.slang)                             | functional | `#mapping-ast-constructs-to-ir`       |
| [`memberexpr-rvalue-lowers-to-get-field.slang`](memberexpr-rvalue-lowers-to-get-field.slang)                 | functional | `#mapping-ast-constructs-to-ir`       |
| [`memberexpr-lvalue-lowers-to-get-field-addr.slang`](memberexpr-lvalue-lowers-to-get-field-addr.slang)            | functional | `#mapping-ast-constructs-to-ir`       |
| [`literalexpr-lowers-to-constant.slang`](literalexpr-lowers-to-constant.slang)                        | functional | `#mapping-ast-constructs-to-ir`       |
| [`witness-table-lowers-to-ir-witness-table.slang`](witness-table-lowers-to-ir-witness-table.slang)              | functional | `#generics-and-existentials`          |
| [`blockstmt-lowers-to-basic-blocks.slang`](blockstmt-lowers-to-basic-blocks.slang)                      | functional | `#mapping-ast-constructs-to-ir`       |
| [`ifelse-merge-block-is-explicit-operand.slang`](ifelse-merge-block-is-explicit-operand.slang)                | functional | `#mapping-ast-constructs-to-ir`       |
| [`entry-point-funcdecl-carries-entry-point-decoration.slang`](entry-point-funcdecl-carries-entry-point-decoration.slang)   | functional | `#module-level-outputs`               |
| [`specialization-deferred-to-ir-pass.slang`](specialization-deferred-to-ir-pass.slang)                    | functional | `#generics-and-existentials`          |
| [`import-not-lowered-into-translation-unit.slang`](import-not-lowered-into-translation-unit.slang)              | functional | `#inputs-and-outputs`                 |
| [`conversion-explicit-cast-lowers-to-castfloattoint.slang`](conversion-explicit-cast-lowers-to-castfloattoint.slang)     | functional | `#mapping-ast-constructs-to-ir`       |
| [`load-store-from-local-pointer.slang`](load-store-from-local-pointer.slang)                         | functional | `#mapping-ast-constructs-to-ir`       |
| [`block-param-replaces-phi.slang`](block-param-replaces-phi.slang)                              | functional | `#mapping-ast-constructs-to-ir`       |
| [`rwbuffer-write-lowers-to-getelementptr-store.slang`](rwbuffer-write-lowers-to-getelementptr-store.slang)          | functional | `#mapping-ast-constructs-to-ir`       |
| [`return-void-uses-void-constant.slang`](return-void-uses-void-constant.slang)                        | functional | `#mapping-ast-constructs-to-ir`       |
| [`funcdecl-empty-body-lowers-to-return-val-void.slang`](funcdecl-empty-body-lowers-to-return-val-void.slang)                     | boundary   | `#mapping-ast-constructs-to-ir` |
| [`funcdecl-many-params-eight-block-params.slang`](funcdecl-many-params-eight-block-params.slang)                           | boundary   | `#mapping-ast-constructs-to-ir` |
| [`funcdecl-default-arg-materialized-at-call-site.slang`](funcdecl-default-arg-materialized-at-call-site.slang)                    | boundary   | `#mapping-ast-constructs-to-ir` |
| [`funcdecl-returns-struct-lowers-to-return-val-struct.slang`](funcdecl-returns-struct-lowers-to-return-val-struct.slang)               | boundary   | `#mapping-ast-constructs-to-ir` |
| [`blockstmt-single-stmt-block-collapses.slang`](blockstmt-single-stmt-block-collapses.slang)                             | boundary   | `#mapping-ast-constructs-to-ir` |
| [`blockstmt-nested-five-deep-multiple-blocks.slang`](blockstmt-nested-five-deep-multiple-blocks.slang)                        | stress     | `#mapping-ast-constructs-to-ir` |
| [`structdecl-many-fields-eight-fields.slang`](structdecl-many-fields-eight-fields.slang)                               | boundary   | `#mapping-ast-constructs-to-ir` |
| [`interfacedecl-empty-no-req-entry.slang`](interfacedecl-empty-no-req-entry.slang)                                  | boundary   | `#mapping-ast-constructs-to-ir` |
| [`interfacedecl-three-methods-three-req-entries.slang`](interfacedecl-three-methods-three-req-entries.slang)                     | boundary   | `#mapping-ast-constructs-to-ir` |
| [`generic-multi-typearg-call-specialize-two-types.slang`](generic-multi-typearg-call-specialize-two-types.slang)                   | boundary   | `#generics-and-existentials`    |
| [`generic-multi-value-args-call-specialize-four-values.slang`](generic-multi-value-args-call-specialize-four-values.slang)              | boundary   | `#generics-and-existentials`    |
| [`generic-recursive-depth-five-call-chain-specialize.slang`](generic-recursive-depth-five-call-chain-specialize.slang)                | stress     | `#generics-and-existentials`    |
| [`literalexpr-uint-max-runtime-add-one-not-folded.slang`](literalexpr-uint-max-runtime-add-one-not-folded.slang)                   | boundary   | `#mapping-ast-constructs-to-ir` |
| [`literalexpr-uint-max-literal-add-one-constant-folds.slang`](literalexpr-uint-max-literal-add-one-constant-folds.slang)               | boundary   | `#mapping-ast-constructs-to-ir` |
| [`static-const-lowers-to-global-constant.slang`](static-const-lowers-to-global-constant.slang)                            | boundary   | `#mapping-ast-constructs-to-ir` |
| [`cbuffer-empty-lowers-to-empty-struct.slang`](cbuffer-empty-lowers-to-empty-struct.slang)                              | boundary   | `#module-level-outputs`         |
| [`cbuffer-single-scalar-lowers-to-one-field-struct.slang`](cbuffer-single-scalar-lowers-to-one-field-struct.slang)                  | boundary   | `#module-level-outputs`         |
| [`cbuffer-mixed-types-lowers-to-struct-with-mixed-fields.slang`](cbuffer-mixed-types-lowers-to-struct-with-mixed-fields.slang)            | boundary   | `#module-level-outputs`         |
| [`entry-point-multiple-each-gets-entry-point-decoration.slang`](entry-point-multiple-each-gets-entry-point-decoration.slang)             | boundary   | `#module-level-outputs`         |
| [`entry-missing-name-fires-no-function-found.slang`](entry-missing-name-fires-no-function-found.slang)                        | negative   | `#module-level-outputs`         |
| [`binaryexpr-sub-lowers-to-sub.slang`](binaryexpr-sub-lowers-to-sub.slang)                                      | boundary   | `#mapping-ast-constructs-to-ir` |
| [`binaryexpr-mul-lowers-to-mul.slang`](binaryexpr-mul-lowers-to-mul.slang)                                      | boundary   | `#mapping-ast-constructs-to-ir` |
| [`binaryexpr-div-lowers-to-div.slang`](binaryexpr-div-lowers-to-div.slang)                                      | boundary   | `#mapping-ast-constructs-to-ir` |
| [`binaryexpr-mod-lowers-to-irem.slang`](binaryexpr-mod-lowers-to-irem.slang)                                     | boundary   | `#mapping-ast-constructs-to-ir` |
| [`binaryexpr-eq-lowers-to-cmpeq.slang`](binaryexpr-eq-lowers-to-cmpeq.slang)                                     | boundary   | `#mapping-ast-constructs-to-ir` |
| [`binaryexpr-lt-lowers-to-cmplt.slang`](binaryexpr-lt-lowers-to-cmplt.slang)                                     | boundary   | `#mapping-ast-constructs-to-ir` |
| [`binaryexpr-ne-lowers-to-cmpne.slang`](binaryexpr-ne-lowers-to-cmpne.slang)                                     | boundary   | `#mapping-ast-constructs-to-ir` |
| [`binaryexpr-bitand-lowers-to-and.slang`](binaryexpr-bitand-lowers-to-and.slang)                                   | boundary   | `#mapping-ast-constructs-to-ir` |
| [`binaryexpr-shl-lowers-to-shl.slang`](binaryexpr-shl-lowers-to-shl.slang)                                      | boundary   | `#mapping-ast-constructs-to-ir` |
| [`unaryexpr-neg-lowers-to-neg.slang`](unaryexpr-neg-lowers-to-neg.slang)                                       | boundary   | `#mapping-ast-constructs-to-ir` |
| [`unaryexpr-bitnot-lowers-to-bitnot.slang`](unaryexpr-bitnot-lowers-to-bitnot.slang)                                 | boundary   | `#mapping-ast-constructs-to-ir` |
| [`funcdecl-zero-params-no-block-params.slang`](funcdecl-zero-params-no-block-params.slang)                              | boundary   | `#mapping-ast-constructs-to-ir` |
| [`funcdecl-recursive-call-self-references-irfunc.slang`](funcdecl-recursive-call-self-references-irfunc.slang)                    | boundary   | `#mapping-ast-constructs-to-ir` |
| [`funcdecl-out-param-lowers-to-outparam-pointer.slang`](funcdecl-out-param-lowers-to-outparam-pointer.slang)                     | boundary   | `#mapping-ast-constructs-to-ir` |
| [`structdecl-single-field-lowers-to-one-field.slang`](structdecl-single-field-lowers-to-one-field.slang)                       | boundary   | `#mapping-ast-constructs-to-ir` |
| [`structdecl-nested-struct-field-lowers-to-field-of-struct-type.slang`](structdecl-nested-struct-field-lowers-to-field-of-struct-type.slang)     | boundary   | `#mapping-ast-constructs-to-ir` |
| [`interfacedecl-single-method-one-req-entry.slang`](interfacedecl-single-method-one-req-entry.slang)                         | boundary   | `#mapping-ast-constructs-to-ir` |
| [`interfacedecl-five-methods-five-req-entries.slang`](interfacedecl-five-methods-five-req-entries.slang)                       | stress     | `#mapping-ast-constructs-to-ir` |
| [`witness-table-two-methods-two-entries.slang`](witness-table-two-methods-two-entries.slang)                             | boundary   | `#generics-and-existentials`    |
| [`ifstmt-no-else-lowers-to-ifelse-merge.slang`](ifstmt-no-else-lowers-to-ifelse-merge.slang)                             | boundary   | `#mapping-ast-constructs-to-ir` |
| [`loop-nested-two-deep-emits-two-loop-instructions.slang`](loop-nested-two-deep-emits-two-loop-instructions.slang)                  | stress     | `#mapping-ast-constructs-to-ir` |
| [`switchstmt-default-only-single-default-arm.slang`](switchstmt-default-only-single-default-arm.slang)                        | boundary   | `#mapping-ast-constructs-to-ir` |
| [`switchstmt-five-cases-five-pairs.slang`](switchstmt-five-cases-five-pairs.slang)                                  | stress     | `#mapping-ast-constructs-to-ir` |
| [`shortcircuit-and-encodes-as-ifelse-with-block-param.slang`](shortcircuit-and-encodes-as-ifelse-with-block-param.slang)               | boundary   | `#mapping-ast-constructs-to-ir` |
| [`cast-uint-from-int-lowers-to-intcast.slang`](cast-uint-from-int-lowers-to-intcast.slang)                              | boundary   | `#mapping-ast-constructs-to-ir` |
| [`vardecl-multiple-global-uniforms-each-lowers-to-global-param.slang`](vardecl-multiple-global-uniforms-each-lowers-to-global-param.slang)      | boundary   | `#mapping-ast-constructs-to-ir` |
| [`invokeexpr-method-call-on-struct-lowers-to-call.slang`](invokeexpr-method-call-on-struct-lowers-to-call.slang)                   | boundary   | `#mapping-ast-constructs-to-ir` |
| [`returnstmt-early-return-emits-multiple-return-val.slang`](returnstmt-early-return-emits-multiple-return-val.slang)                 | boundary   | `#mapping-ast-constructs-to-ir` |
| [`return-from-inside-loop-emits-return-val-in-loop-block.slang`](return-from-inside-loop-emits-return-val-in-loop-block.slang)            | boundary   | `#mapping-ast-constructs-to-ir` |
| [`non-entry-funcdecl-lacks-entry-point-decoration.slang`](non-entry-funcdecl-lacks-entry-point-decoration.slang)                   | boundary   | `#module-level-outputs`         |
| [`generic-call-three-distinct-specializations-each-deferred.slang`](generic-call-three-distinct-specializations-each-deferred.slang)         | stress     | `#generics-and-existentials`    |
| [`literalexpr-float-literal-lowers-to-float-constant.slang`](literalexpr-float-literal-lowers-to-float-constant.slang)                | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-int8-scalar.slang`](lower-int8-scalar.slang)                                                 | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-uint8-scalar.slang`](lower-uint8-scalar.slang)                                                | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-int16-scalar.slang`](lower-int16-scalar.slang)                                                | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-uint16-scalar.slang`](lower-uint16-scalar.slang)                                               | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-int64-scalar.slang`](lower-int64-scalar.slang)                                                | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-uint64-scalar.slang`](lower-uint64-scalar.slang)                                               | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-int8-vector.slang`](lower-int8-vector.slang)                                                 | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-uint16-vector.slang`](lower-uint16-vector.slang)                                               | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-int64-vector.slang`](lower-int64-vector.slang)                                                | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-int8-array.slang`](lower-int8-array.slang)                                                  | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-uint64-array.slang`](lower-uint64-array.slang)                                                | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-int16-struct-field.slang`](lower-int16-struct-field.slang)                                          | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-uint8-struct-field.slang`](lower-uint8-struct-field.slang)                                          | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-int64-buffer-element.slang`](lower-int64-buffer-element.slang)                                        | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-int8-arithmetic.slang`](lower-int8-arithmetic.slang)                                             | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-half-scalar.slang`](lower-half-scalar.slang)                                                 | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-half3-vector.slang`](lower-half3-vector.slang)                                                | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-half-matrix.slang`](lower-half-matrix.slang)                                                 | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-half-array.slang`](lower-half-array.slang)                                                  | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-half-struct-field.slang`](lower-half-struct-field.slang)                                           | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-half-arithmetic.slang`](lower-half-arithmetic.slang)                                             | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-double-scalar.slang`](lower-double-scalar.slang)                                               | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-double3-vector.slang`](lower-double3-vector.slang)                                              | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-double-matrix.slang`](lower-double-matrix.slang)                                               | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-double-array.slang`](lower-double-array.slang)                                                | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-double-struct-field.slang`](lower-double-struct-field.slang)                                         | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-double-arithmetic.slang`](lower-double-arithmetic.slang)                                           | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-uint64-arithmetic.slang`](lower-uint64-arithmetic.slang)                                           | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-mat2x4-rectangular.slang`](lower-mat2x4-rectangular.slang)                                          | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-mat4x2-rectangular.slang`](lower-mat4x2-rectangular.slang)                                          | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-mat-half-rectangular.slang`](lower-mat-half-rectangular.slang)                                        | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-mat-double-rectangular.slang`](lower-mat-double-rectangular.slang)                                      | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-optional-half3.slang`](lower-optional-half3.slang)                                              | expansion  | `#mapping-ast-constructs-to-ir` |
| [`lower-optional-int.slang`](lower-optional-int.slang)                                                | expansion  | `#mapping-ast-constructs-to-ir` |
| [`lower-tuple-heterogeneous.slang`](lower-tuple-heterogeneous.slang)                                         | expansion  | `#mapping-ast-constructs-to-ir` |
| [`lower-tuple-narrow-pair.slang`](lower-tuple-narrow-pair.slang)                                           | expansion  | `#mapping-ast-constructs-to-ir` |
| [`lower-struct-of-mixed-precision.slang`](lower-struct-of-mixed-precision.slang)                                   | expansion  | `#mapping-ast-constructs-to-ir` |
| [`lower-struct-of-arrays.slang`](lower-struct-of-arrays.slang)                                            | expansion  | `#mapping-ast-constructs-to-ir` |
| [`lower-array-of-structs.slang`](lower-array-of-structs.slang)                                            | expansion  | `#mapping-ast-constructs-to-ir` |
| [`lower-parameter-block-narrow-struct.slang`](lower-parameter-block-narrow-struct.slang)                               | expansion  | `#mapping-ast-constructs-to-ir` |
| [`lower-buffer-of-struct-with-narrow-types.slang`](lower-buffer-of-struct-with-narrow-types.slang)                          | expansion  | `#mapping-ast-constructs-to-ir` |
| [`lower-cast-half-to-float.slang`](lower-cast-half-to-float.slang)                                          | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-cast-double-to-float.slang`](lower-cast-double-to-float.slang)                                        | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-cast-int8-to-int.slang`](lower-cast-int8-to-int.slang)                                            | boundary   | `#mapping-ast-constructs-to-ir` |
| [`lower-cast-int64-to-int.slang`](lower-cast-int64-to-int.slang)                                           | boundary   | `#mapping-ast-constructs-to-ir` |

## Doc gaps observed

- The doc's "Mapping AST constructs to IR" table is explicitly
  "illustrative, not exhaustive" — a number of AST families that
  appear in practice are absent: `IndexExpr` (resource subscript /
  array element access), `AssignExpr` (the assignment itself, vs.
  the lvalue-side `get_field_addr` + `store`), `TypeCastExpr`
  (cast lowering — only mentioned implicitly through the
  arithmetic/comparison row), `BreakStmt` / `ContinueStmt` (loop
  exit / continue terminators), `ConditionalExpr` (ternary). A
  single-line addition per row would let agents anchor tests for
  each. The bundle covers explicit casts and buffer indexing under
  the existing rows but with a stretched citation.
- The doc says "structured branches whose join point is an
  explicit operand on the terminator" but does not say which
  operand position is the merge block for each terminator. The
  `cross-cutting/ir-instructions` doc has more detail on this; an
  explicit cross-reference would help.
- "The lowering visitor descends the AST top-down" — the doc names
  the algorithmic style but does not list any user-observable
  consequence (e.g. ordering of nameHint decoration emission). An
  observability hook would let an agent test it.
- The doc says "Lowering errors flow through the same
  DiagnosticSink used by the rest of the front-end" but lists no
  specific construct that produces a lowering-only diagnostic. A
  single example (e.g. "interface conformance with a missing
  witness that semantic check could not detect") would unlock
  diagnostic tests for this stage; without one, all rejection-style
  tests belong to `pipeline/03-semantic-check`.
- `generateIRForSpecializedComponentType` and
  `generateIRForTypeConformance` are described as entry points but
  the doc does not name a `.slang`-observable surface that
  triggers each path. They are reachable only through C++ API.
- The doc says global parameters carry "layout intent" markers,
  but does not state what those markers look like in the
  `### LOWER-TO-IR:` stage (the layout decoration appears in later
  stages). A row of the form "global params carry decoration X at
  lowering" would let an agent test it; otherwise the topic
  belongs to `pipeline/04c-layout-ir`.
- **Type-lowering claims are implicit rather than enumerated.** The
  doc's table only names which AST-family produces which IR-family
  (e.g. `StructDecl -> IRStructType`). It does not enumerate the
  per-scalar-type IR-side spellings that lowering must preserve —
  `Int8`/`UInt8`/`Int16`/`UInt16`/`Int64`/`UInt64`/`Half`/`Double`,
  vector/array/matrix composition with each, and the rectangular
  matrix `Mat(<elem>, R : Int, C : Int, 0 : Int)` shape with row/col
  order. A "Element types and storage shapes" subsection under
  "Mapping AST constructs to IR" — or a cross-reference to
  `cross-cutting/ir-instructions.md` — would let agents anchor
  tests for these without stretching the existing rows.
- **`Optional<T>` and `Tuple<...>` are not in the AST-family table.**
  Optional lowers to `Optional(<T>)` and Tuple to `tuple_type(...)`;
  both are observable through `-dump-ir`. The doc references the
  hash-consing rules in `design/ir.md` but no row in the lowering
  table names these generic-composite types, even though their
  syntax is part of the documented surface in
  `pipeline/03-semantic-check`.
- **`ParameterBlock<T>` lowering is not mentioned by name.** The
  doc says global VarDecls become `IRGlobalVar`, but
  `ParameterBlock(%T)` is a distinct IR type, not a plain global.
  A row "ParameterBlock<T> VarDecl -> IRGlobalParam of type
  `ParameterBlock(%T)`" would close this gap.
- **Cast lowering is illustrated by name but not by family.** The
  doc table mentions `BinaryExpr -> IRAdd/IRMul/IREq, ...`. Explicit
  type-cast lowering follows a parallel family — `floatCast`,
  `intCast`, `castFloatToInt`, `castIntToFloat` — with one opcode
  per source/dest type family. A row "TypeCastExpr -> family of
  cast pure-value insts" would tie these to the doc directly; the
  bundle currently anchors them under the more general
  "Mapping AST constructs to IR" header.

## Out of scope (no-GPU runner)

(This heading is used here for "claims not observable through any
allowed slang-test directive", consistent with the
`cross-cutting/ir-instructions` bundle convention. The doc is about
lowering, so most claims are observable via `-dump-ir`; the items
below are genuinely outside that observability.)

- Internal lowering visitor structure (which C++ visitor method
  handles which AST node) — implementation detail of
  `slang-lower-to-ir.cpp`, not surface-visible.
- `IRBuilder` hash-consing decisions — the dump shows the post-
  dedup result, not the decision path.
- `kIROpFlag_Hoistable` / `kIROpFlag_Global` flag bits — internal
  to `IRInst`, not in the dump.
- Side artefacts on the surrounding `Module` (entry-point list,
  type-conformance bookkeeping, layout-intent markers) — these
  are C++ container contents, not observable through slangc CLI.
- `generateIRForSpecializedComponentType` and
  `generateIRForTypeConformance` — component-type API entry
  points invoked through the `Session`, not through the command
  line. A C++ unit test would exercise them.
- "Lowering errors flow through DiagnosticSink" — true, but the
  doc does not name a specific construct that produces a
  lowering-only diagnostic, so no `.slang` test can probe it
  here. Diagnostic claims belong to `pipeline/03-semantic-check`.
- The `slang-ir-insts-enum.h` FIDDLE-generated `IROp` enum — a
  build-time artefact.
- Order of pass invocation inside `linkAndOptimizeIR` — belongs
  to `pipeline/05-ir-passes`.
