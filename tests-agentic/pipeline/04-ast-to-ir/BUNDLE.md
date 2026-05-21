---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-21T18:00:00+00:00
source_commit: 1655c2bf8d3567fa220a5226769ef5e3917d55e8
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
| C-01     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `FuncDecl` lowers to an `IRFunc` containing one or more `IRBlock`s; a call site lowers to a `call` instruction.                               | `funcdecl-lowers-to-irfunc.slang`                              |
| C-02     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A local `VarDecl` whose address is taken (struct field access) lowers to an `IRVar` allocated inside the enclosing block.                       | `vardecl-local-struct-lowers-to-irvar.slang`                   |
| C-03     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A module-scope `uniform VarDecl` lowers to an `IRGlobalVar` (spelled `global_param` in the dump).                                               | `vardecl-global-uniform-lowers-to-global-param.slang`          |
| C-04     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `StructDecl` lowers to an `IRStructType` containing `IRStructField` children.                                                                 | `structdecl-lowers-to-struct-with-fields.slang`                |
| C-05     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | An `InterfaceDecl` lowers to an `IRInterfaceType` plus per-method `interface_req_entry` instructions.                                           | `interfacedecl-lowers-to-interface-with-req-entries.slang`     |
| C-06     | [#generics-and-existentials](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#generics-and-existentials)                                    | A `GenericDecl` lowers to an `IRGeneric` (function-shaped instruction whose body returns the inner function).                                   | `genericdecl-lowers-to-irgeneric.slang`                        |
| C-07     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | An `IfStmt` lowers to an `ifElse` structured-branch terminator whose merge point is an explicit operand.                                        | `ifstmt-lowers-to-ifelse.slang`                                |
| C-08     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `ForStmt` lowers to a `loop` terminator; the induction variable is carried as a block `param`.                                                | `forstmt-lowers-to-loop-with-block-params.slang`               |
| C-09     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `WhileStmt` lowers to a `loop` structured-branch terminator (same opcode as for-loop).                                                        | `whilestmt-lowers-to-loop.slang`                               |
| C-10     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `SwitchStmt` lowers to a `switch` terminator with explicit case-value/case-block operand pairs and a merge operand.                           | `switchstmt-lowers-to-switch.slang`                            |
| C-11     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `ReturnStmt` lowers to an `IRReturn` terminator (spelled `return_val`).                                                                       | `returnstmt-lowers-to-return-val.slang`                        |
| C-12     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `BinaryExpr` arithmetic site lowers to the corresponding pure-value IR opcode (`add`).                                                        | `binaryexpr-arithmetic-lowers-to-add.slang`                    |
| C-13     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `BinaryExpr` comparison site lowers to a comparison IR opcode (`cmpGT` for `>`).                                                              | `binaryexpr-comparison-lowers-to-cmpgt.slang`                  |
| C-14     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | An `InvokeExpr` (function call) lowers to an `IRCall` (spelled `call %<callee>(...)`).                                                          | `invokeexpr-lowers-to-call.slang`                              |
| C-15     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `MemberExpr` in rvalue position lowers to `IRFieldExtract` (spelled `get_field`).                                                             | `memberexpr-rvalue-lowers-to-get-field.slang`                  |
| C-16     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `MemberExpr` used as an assignment target lowers to `IRFieldAddress` (spelled `get_field_addr`) followed by `store`.                          | `memberexpr-lvalue-lowers-to-get-field-addr.slang`             |
| C-17     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `LiteralExpr` lowers to a constant IR instruction visible inline as the literal value.                                                        | `literalexpr-lowers-to-constant.slang`                         |
| C-18     | [#generics-and-existentials](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#generics-and-existentials)                                    | A user-synthesized witness table lowers to an `IRWitnessTable` with `witness_table_entry` children.                                             | `witness-table-lowers-to-ir-witness-table.slang`               |
| C-19     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `BlockStmt` containing branching control flow lowers to multiple basic blocks inside the enclosing function.                                  | `blockstmt-lowers-to-basic-blocks.slang`                       |
| C-20     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Structured `ifElse` encodes its join (merge) point as an explicit operand; both branches reach it via `unconditionalBranch`.                    | `ifelse-merge-block-is-explicit-operand.slang`                 |
| C-21     | [#module-level-outputs](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#module-level-outputs)                                              | A `FuncDecl` registered as an entry point appears in the lowered module with an `entryPoint(...)` decoration on its IRFunc.                     | `entry-point-funcdecl-carries-entry-point-decoration.slang`    |
| C-22     | [#generics-and-existentials](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#generics-and-existentials)                                    | Lowering does not specialize generics; a generic call appears as `call specialize(%generic, <Type>)(...)`.                                      | `specialization-deferred-to-ir-pass.slang`                     |
| C-23     | [#inputs-and-outputs](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#inputs-and-outputs)                                                  | References to symbols defined in imported modules appear with `[import("...")]` decorations rather than full bodies in the lowered IR.          | `import-not-lowered-into-translation-unit.slang`               |
| C-24     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | An explicit constructor-style cast `int(x)` / `float(y)` lowers to a dedicated conversion IR opcode (`castFloatToInt` / `castIntToFloat`).      | `conversion-explicit-cast-lowers-to-castfloattoint.slang`      |
| C-25     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Assignment to a struct field lowers to a `store(%addr, %v)`; reading the field back lowers to a `load(%addr)`.                                  | `load-store-from-local-pointer.slang`                          |
| C-26     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Phi-style joining is encoded as block parameters (`param` at the start of a block); branches carry the next values as arguments, not via a `phi`. | `block-param-replaces-phi.slang`                               |
| C-27     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A subscript-assignment on an `RWStructuredBuffer<T>` lowers to `rwstructuredBufferGetElementPtr` followed by `store`.                           | `rwbuffer-write-lowers-to-getelementptr-store.slang`           |
| C-28     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | A `return` from a void-returning function lowers to `return_val(void_constant)`.                                                                | `return-void-uses-void-constant.slang`                         |
| C-29     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | The "Pure value insts (IRAdd, IRMul, IREq, ...)" row covers all integer arithmetic and comparison operators: `+/-/*///%` and `==/!=/</>`.       | `binaryexpr-{sub,mul,div,mod,eq,lt,ne,bitand,shl}-*.slang`     |
| C-30     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | Unary arithmetic / bitwise operators (`-a`, `~a`) lower to single-operand pure-value insts in the same family (`neg`, `bitnot`).                | `unaryexpr-{neg,bitnot}-*.slang`                               |
| C-31     | [#mapping-ast-constructs-to-ir](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#mapping-ast-constructs-to-ir)                              | The "InvokeExpr -> IRCall" row covers method-call expressions and self-recursive calls as well as free-function calls.                          | `invokeexpr-method-call-...slang`, `funcdecl-recursive-*.slang` |
| C-32     | [#module-level-outputs](../../../docs/llm-generated/pipeline/04-ast-to-ir.md#module-level-outputs)                                              | Only FuncDecls registered as entry points carry an `entryPoint(...)` decoration; non-entry helpers in the same translation unit do not.         | `non-entry-funcdecl-lacks-entry-point-decoration.slang`        |

## Tests in this bundle

| File                                                          | Intent     | Doc anchor                            |
| ------------------------------------------------------------- | ---------- | ------------------------------------- |
| `funcdecl-lowers-to-irfunc.slang`                             | functional | `#mapping-ast-constructs-to-ir`       |
| `vardecl-local-struct-lowers-to-irvar.slang`                  | functional | `#mapping-ast-constructs-to-ir`       |
| `vardecl-global-uniform-lowers-to-global-param.slang`         | functional | `#mapping-ast-constructs-to-ir`       |
| `structdecl-lowers-to-struct-with-fields.slang`               | functional | `#mapping-ast-constructs-to-ir`       |
| `interfacedecl-lowers-to-interface-with-req-entries.slang`    | functional | `#mapping-ast-constructs-to-ir`       |
| `genericdecl-lowers-to-irgeneric.slang`                       | functional | `#generics-and-existentials`          |
| `ifstmt-lowers-to-ifelse.slang`                               | functional | `#mapping-ast-constructs-to-ir`       |
| `forstmt-lowers-to-loop-with-block-params.slang`              | functional | `#mapping-ast-constructs-to-ir`       |
| `whilestmt-lowers-to-loop.slang`                              | functional | `#mapping-ast-constructs-to-ir`       |
| `switchstmt-lowers-to-switch.slang`                           | functional | `#mapping-ast-constructs-to-ir`       |
| `returnstmt-lowers-to-return-val.slang`                       | functional | `#mapping-ast-constructs-to-ir`       |
| `binaryexpr-arithmetic-lowers-to-add.slang`                   | functional | `#mapping-ast-constructs-to-ir`       |
| `binaryexpr-comparison-lowers-to-cmpgt.slang`                 | functional | `#mapping-ast-constructs-to-ir`       |
| `invokeexpr-lowers-to-call.slang`                             | functional | `#mapping-ast-constructs-to-ir`       |
| `memberexpr-rvalue-lowers-to-get-field.slang`                 | functional | `#mapping-ast-constructs-to-ir`       |
| `memberexpr-lvalue-lowers-to-get-field-addr.slang`            | functional | `#mapping-ast-constructs-to-ir`       |
| `literalexpr-lowers-to-constant.slang`                        | functional | `#mapping-ast-constructs-to-ir`       |
| `witness-table-lowers-to-ir-witness-table.slang`              | functional | `#generics-and-existentials`          |
| `blockstmt-lowers-to-basic-blocks.slang`                      | functional | `#mapping-ast-constructs-to-ir`       |
| `ifelse-merge-block-is-explicit-operand.slang`                | functional | `#mapping-ast-constructs-to-ir`       |
| `entry-point-funcdecl-carries-entry-point-decoration.slang`   | functional | `#module-level-outputs`               |
| `specialization-deferred-to-ir-pass.slang`                    | functional | `#generics-and-existentials`          |
| `import-not-lowered-into-translation-unit.slang`              | functional | `#inputs-and-outputs`                 |
| `conversion-explicit-cast-lowers-to-castfloattoint.slang`     | functional | `#mapping-ast-constructs-to-ir`       |
| `load-store-from-local-pointer.slang`                         | functional | `#mapping-ast-constructs-to-ir`       |
| `block-param-replaces-phi.slang`                              | functional | `#mapping-ast-constructs-to-ir`       |
| `rwbuffer-write-lowers-to-getelementptr-store.slang`          | functional | `#mapping-ast-constructs-to-ir`       |
| `return-void-uses-void-constant.slang`                        | functional | `#mapping-ast-constructs-to-ir`       |
| `funcdecl-empty-body-lowers-to-return-val-void.slang`                     | boundary   | `#mapping-ast-constructs-to-ir` |
| `funcdecl-many-params-eight-block-params.slang`                           | boundary   | `#mapping-ast-constructs-to-ir` |
| `funcdecl-default-arg-materialized-at-call-site.slang`                    | boundary   | `#mapping-ast-constructs-to-ir` |
| `funcdecl-returns-struct-lowers-to-return-val-struct.slang`               | boundary   | `#mapping-ast-constructs-to-ir` |
| `blockstmt-single-stmt-block-collapses.slang`                             | boundary   | `#mapping-ast-constructs-to-ir` |
| `blockstmt-nested-five-deep-multiple-blocks.slang`                        | stress     | `#mapping-ast-constructs-to-ir` |
| `structdecl-many-fields-eight-fields.slang`                               | boundary   | `#mapping-ast-constructs-to-ir` |
| `interfacedecl-empty-no-req-entry.slang`                                  | boundary   | `#mapping-ast-constructs-to-ir` |
| `interfacedecl-three-methods-three-req-entries.slang`                     | boundary   | `#mapping-ast-constructs-to-ir` |
| `generic-multi-typearg-call-specialize-two-types.slang`                   | boundary   | `#generics-and-existentials`    |
| `generic-multi-value-args-call-specialize-four-values.slang`              | boundary   | `#generics-and-existentials`    |
| `generic-recursive-depth-five-call-chain-specialize.slang`                | stress     | `#generics-and-existentials`    |
| `literalexpr-uint-max-runtime-add-one-not-folded.slang`                   | boundary   | `#mapping-ast-constructs-to-ir` |
| `literalexpr-uint-max-literal-add-one-constant-folds.slang`               | boundary   | `#mapping-ast-constructs-to-ir` |
| `static-const-lowers-to-global-constant.slang`                            | boundary   | `#mapping-ast-constructs-to-ir` |
| `cbuffer-empty-lowers-to-empty-struct.slang`                              | boundary   | `#module-level-outputs`         |
| `cbuffer-single-scalar-lowers-to-one-field-struct.slang`                  | boundary   | `#module-level-outputs`         |
| `cbuffer-mixed-types-lowers-to-struct-with-mixed-fields.slang`            | boundary   | `#module-level-outputs`         |
| `entry-point-multiple-each-gets-entry-point-decoration.slang`             | boundary   | `#module-level-outputs`         |
| `entry-missing-name-fires-no-function-found.slang`                        | negative   | `#module-level-outputs`         |
| `binaryexpr-sub-lowers-to-sub.slang`                                      | boundary   | `#mapping-ast-constructs-to-ir` |
| `binaryexpr-mul-lowers-to-mul.slang`                                      | boundary   | `#mapping-ast-constructs-to-ir` |
| `binaryexpr-div-lowers-to-div.slang`                                      | boundary   | `#mapping-ast-constructs-to-ir` |
| `binaryexpr-mod-lowers-to-irem.slang`                                     | boundary   | `#mapping-ast-constructs-to-ir` |
| `binaryexpr-eq-lowers-to-cmpeq.slang`                                     | boundary   | `#mapping-ast-constructs-to-ir` |
| `binaryexpr-lt-lowers-to-cmplt.slang`                                     | boundary   | `#mapping-ast-constructs-to-ir` |
| `binaryexpr-ne-lowers-to-cmpne.slang`                                     | boundary   | `#mapping-ast-constructs-to-ir` |
| `binaryexpr-bitand-lowers-to-and.slang`                                   | boundary   | `#mapping-ast-constructs-to-ir` |
| `binaryexpr-shl-lowers-to-shl.slang`                                      | boundary   | `#mapping-ast-constructs-to-ir` |
| `unaryexpr-neg-lowers-to-neg.slang`                                       | boundary   | `#mapping-ast-constructs-to-ir` |
| `unaryexpr-bitnot-lowers-to-bitnot.slang`                                 | boundary   | `#mapping-ast-constructs-to-ir` |
| `funcdecl-zero-params-no-block-params.slang`                              | boundary   | `#mapping-ast-constructs-to-ir` |
| `funcdecl-recursive-call-self-references-irfunc.slang`                    | boundary   | `#mapping-ast-constructs-to-ir` |
| `funcdecl-out-param-lowers-to-outparam-pointer.slang`                     | boundary   | `#mapping-ast-constructs-to-ir` |
| `structdecl-single-field-lowers-to-one-field.slang`                       | boundary   | `#mapping-ast-constructs-to-ir` |
| `structdecl-nested-struct-field-lowers-to-field-of-struct-type.slang`     | boundary   | `#mapping-ast-constructs-to-ir` |
| `interfacedecl-single-method-one-req-entry.slang`                         | boundary   | `#mapping-ast-constructs-to-ir` |
| `interfacedecl-five-methods-five-req-entries.slang`                       | stress     | `#mapping-ast-constructs-to-ir` |
| `witness-table-two-methods-two-entries.slang`                             | boundary   | `#generics-and-existentials`    |
| `ifstmt-no-else-lowers-to-ifelse-merge.slang`                             | boundary   | `#mapping-ast-constructs-to-ir` |
| `loop-nested-two-deep-emits-two-loop-instructions.slang`                  | stress     | `#mapping-ast-constructs-to-ir` |
| `switchstmt-default-only-single-default-arm.slang`                        | boundary   | `#mapping-ast-constructs-to-ir` |
| `switchstmt-five-cases-five-pairs.slang`                                  | stress     | `#mapping-ast-constructs-to-ir` |
| `shortcircuit-and-encodes-as-ifelse-with-block-param.slang`               | boundary   | `#mapping-ast-constructs-to-ir` |
| `cast-uint-from-int-lowers-to-intcast.slang`                              | boundary   | `#mapping-ast-constructs-to-ir` |
| `vardecl-multiple-global-uniforms-each-lowers-to-global-param.slang`      | boundary   | `#mapping-ast-constructs-to-ir` |
| `invokeexpr-method-call-on-struct-lowers-to-call.slang`                   | boundary   | `#mapping-ast-constructs-to-ir` |
| `returnstmt-early-return-emits-multiple-return-val.slang`                 | boundary   | `#mapping-ast-constructs-to-ir` |
| `return-from-inside-loop-emits-return-val-in-loop-block.slang`            | boundary   | `#mapping-ast-constructs-to-ir` |
| `non-entry-funcdecl-lacks-entry-point-decoration.slang`                   | boundary   | `#module-level-outputs`         |
| `generic-call-three-distinct-specializations-each-deferred.slang`         | stress     | `#generics-and-existentials`    |
| `literalexpr-float-literal-lowers-to-float-constant.slang`                | boundary   | `#mapping-ast-constructs-to-ir` |

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
