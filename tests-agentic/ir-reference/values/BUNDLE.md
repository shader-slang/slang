---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-21T00:00:00+00:00
source_commit: 331a01d9d0cc721d8fc19a46fec17a4a275ba0e0
watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_doc: docs/llm-generated/ir-reference/values.md
source_doc_digest: 0bd66a543db2abee42ed03fc4534c5483860e577fe816355ee527d444cedd80b
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/values

## Intent

Tests verify the per-opcode catalog of the IR value-producing family
described in
[`docs/llm-generated/ir-reference/values.md`](../../../docs/llm-generated/ir-reference/values.md):
that each documented IR value opcode (literal payloads inline on
`IRInst` for `integer_constant`/`float_constant`/`boolConst`/`void_constant`;
arithmetic and bitwise `irem`/`frem`/`neg`/`shl`/`shr`/`and`/`or`/`xor`/`bitnot`/`not`;
the branch-free `select`; comparisons
`cmpEQ`/`cmpNE`/`cmpLT`/`cmpLE`/`cmpGE`;
conversions `intCast`/`floatCast`/`castIntToFloat`/`castFloatToInt`/`bitCast`/`castToVoid`;
memory `get_field` (rvalue), `getElement` (rvalue), `getElementPtr`
(lvalue), `globalConstant`, `swizzle` (multi-component) and
`swizzledStore`;
aggregate constructors `makeVector`/`makeMatrix`/`makeTuple`;
the optional helpers
`makeOptionalValue`/`makeOptionalNone`/`optionalHasValue`/`getOptionalValue`;
the bitfield builtins `bitfieldExtract`/`bitfieldInsert`;
the `out` parameter form `OutParam(<T>)`;
and `return_val(void_constant)` for `void`-returning functions)
appears in `-dump-ir` output for the obvious surface construct that
produces it, and that target-divergent emit lowerings (vector
swizzle, integer/float literals) emit the documented per-target
text on the text-emit backends.

The primary observation mechanism is `-target spirv-asm -dump-ir -o
/dev/null` followed by a FileCheck against the IR dump. Anchors are
user-named functions, user struct fields, and user
`global_param`s — the IR dump's preamble is large and any unanchored
pattern risks false positives. For target-divergent lowerings
(vector swizzle, literal emission) the test carries one SIMPLE
directive per feasible text-emit target with a distinct CHECK
prefix, because the doc does not claim cross-target uniformity for
these lowerings.

Catalog rows that the `cross-cutting/ir-instructions` bundle already
covers at a sample level (`add`/`sub`/`mul`/`div` for `int`, `cmpGT`,
`intCast` from `uint`, the `var`+`load`+`store`+`get_field_addr`
quartet on a struct local, single-component `swizzle`,
`rwstructuredBufferGetElementPtr`) are not duplicated here; this
bundle drills into the rest of the catalog without repeating those.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                                              | Claim (one line)                                                                                                                              | Tests                                          |
| -------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | --------------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------- |
| C-01     | [#literal-payload-encoding](../../../docs/llm-generated/ir-reference/values.md#literal-payload-encoding)                                                                            | Constant-family opcodes (`IntLit`/`FloatLit`/`BoolLit`/`VoidLit`) store their payload inline; literals appear as `<value> : <Type>` operands. | `literals-inline-payload.slang`                |
| C-02     | [#literals-constant-group](../../../docs/llm-generated/ir-reference/values.md#literals-constant-group)                                                                              | A `void` function's terminator is `return_val(void_constant)`, exposing the `VoidLit` unique value.                                           | `return-val-void-constant.slang`               |
| C-03     | [#arithmetic-and-bitwise](../../../docs/llm-generated/ir-reference/values.md#arithmetic-and-bitwise)                                                                                | Integer `%` lowers to `irem`, float `%` lowers to `frem`, prefix `-` lowers to `neg`.                                                         | `arith-irem-frem-neg.slang`                    |
| C-04     | [#arithmetic-and-bitwise](../../../docs/llm-generated/ir-reference/values.md#arithmetic-and-bitwise)                                                                                | `<<` lowers to `shl` and `>>` lowers to `shr`.                                                                                                | `arith-shifts.slang`                           |
| C-05     | [#arithmetic-and-bitwise](../../../docs/llm-generated/ir-reference/values.md#arithmetic-and-bitwise)                                                                                | Bitwise `&` / `|` / `^` lower to `and` / `or` / `xor`; prefix `~` lowers to `bitnot`.                                                         | `arith-bitwise.slang`                          |
| C-06     | [#arithmetic-and-bitwise](../../../docs/llm-generated/ir-reference/values.md#arithmetic-and-bitwise)                                                                                | Prefix `!` on a `bool` lowers to the `not` opcode.                                                                                            | `logical-not.slang`                            |
| C-07     | [#arithmetic-and-bitwise](../../../docs/llm-generated/ir-reference/values.md#arithmetic-and-bitwise)                                                                                | `bitfieldExtract(v, off, count)` and `bitfieldInsert(b, ins, off, count)` builtins lower to opcodes of the same names.                       | `bitfield-extract-insert.slang`                |
| C-08     | [#select](../../../docs/llm-generated/ir-reference/values.md#select)                                                                                                                | The `select(c, x, y)` builtin lowers to the branch-free `select` IR opcode.                                                                   | `select-builtin.slang`                         |
| C-09     | [#comparison](../../../docs/llm-generated/ir-reference/values.md#comparison)                                                                                                        | `==` lowers to `cmpEQ` and `!=` lowers to `cmpNE`.                                                                                            | `comparison-eq-ne.slang`                       |
| C-10     | [#comparison](../../../docs/llm-generated/ir-reference/values.md#comparison)                                                                                                        | `<` / `<=` / `>=` lower to `cmpLT` / `cmpLE` / `cmpGE`.                                                                                       | `comparison-lt-le-ge.slang`                    |
| C-11     | [#conversions](../../../docs/llm-generated/ir-reference/values.md#conversions)                                                                                                      | `intCast` covers both narrowing and widening integer conversions.                                                                             | `conversion-intcast-narrow.slang`              |
| C-12     | [#conversions](../../../docs/llm-generated/ir-reference/values.md#conversions)                                                                                                      | A precision-changing float-to-float cast lowers to `floatCast`.                                                                               | `conversion-floatcast.slang`                   |
| C-13     | [#conversions](../../../docs/llm-generated/ir-reference/values.md#conversions)                                                                                                      | `float(intVal)` lowers to `castIntToFloat`; `int(floatVal)` lowers to `castFloatToInt`.                                                       | `conversion-int-float.slang`                   |
| C-14     | [#conversions](../../../docs/llm-generated/ir-reference/values.md#conversions)                                                                                                      | The `bit_cast<T>(value)` builtin lowers to the `bitCast` opcode (reinterpret).                                                                | `conversion-bitcast.slang`                     |
| C-15     | [#conversions](../../../docs/llm-generated/ir-reference/values.md#conversions)                                                                                                      | A `(void)expr` statement discards its operand via the `castToVoid` opcode.                                                                    | `conversion-cast-to-void.slang`                |
| C-16     | [#fieldaddress-vs-fieldextract](../../../docs/llm-generated/ir-reference/values.md#fieldaddress-vs-fieldextract)                                                                    | A `MemberExpr` on a struct *value* (rvalue) lowers to `get_field`; the lvalue path uses `get_field_addr`.                                     | `memory-get-field-rvalue.slang`                |
| C-17     | [#getelementptr-vs-getelement](../../../docs/llm-generated/ir-reference/values.md#getelementptr-vs-getelement)                                                                      | Array indexing on a pointer/lvalue lowers to `getElementPtr`; indexing on a value lowers to `getElement`.                                     | `memory-getelement-and-getelementptr.slang`    |
| C-18     | [#memory](../../../docs/llm-generated/ir-reference/values.md#memory)                                                                                                                | A module-scope `static const` declaration lowers to `globalConstant(<value>)`.                                                                | `memory-global-constant.slang`                 |
| C-19     | [#swizzle-swizzleset-swizzledstore](../../../docs/llm-generated/ir-reference/values.md#swizzle-swizzleset-swizzledstore)                                                            | A multi-component swizzle (`v.xyz`) lowers to `swizzle(%v, 0 : Int, 1 : Int, 2 : Int)`.                                                       | `swizzle-multi-component.slang`                |
| C-20     | [#swizzle-swizzleset-swizzledstore](../../../docs/llm-generated/ir-reference/values.md#swizzle-swizzleset-swizzledstore)                                                            | A swizzle in lvalue position (`dst.xy = ...`) lowers to `swizzledStore`.                                                                      | `swizzle-store-lvalue.slang`                   |
| C-21     | [#swizzle-swizzleset-swizzledstore](../../../docs/llm-generated/ir-reference/values.md#swizzle-swizzleset-swizzledstore)                                                            | Vector swizzle emits as `.xyz`-style member access on HLSL/GLSL/Metal/WGSL; CPP renders the same component reads inside a `Vector<>` brace.   | `swizzle-emit-multi-target.slang`              |
| C-22     | [#aggregate-constructors](../../../docs/llm-generated/ir-reference/values.md#aggregate-constructors)                                                                                | A `floatN(...)` constructor call lowers to `makeVector(<components>)`.                                                                        | `aggregate-makevector.slang`                   |
| C-23     | [#aggregate-constructors](../../../docs/llm-generated/ir-reference/values.md#aggregate-constructors)                                                                                | A `floatRxC(...)` constructor lowers to `makeMatrix(<components>)`.                                                                           | `aggregate-makematrix.slang`                   |
| C-24     | [#aggregate-constructors](../../../docs/llm-generated/ir-reference/values.md#aggregate-constructors)                                                                                | `makeTuple(a, b)` lowers to the `makeTuple` opcode producing a `tuple_type(...)` value.                                                       | `aggregate-maketuple.slang`                    |
| C-25     | [#result-optional-conditional-helpers](../../../docs/llm-generated/ir-reference/values.md#result-optional-conditional-helpers)                                                      | `Optional<T>` constructors and accessors lower to `makeOptionalValue` / `makeOptionalNone` / `optionalHasValue` / `getOptionalValue`.         | `optional-make-has-value.slang`                |
| C-26     | [#conversions](../../../docs/llm-generated/ir-reference/values.md#conversions)                                                                                                      | An `out T` parameter has IR type `OutParam(T)`; the caller passes a `Ptr(T)` from a local `var`.                                              | `out-param-type.slang`                         |
| C-27     | [#literal-payload-encoding](../../../docs/llm-generated/ir-reference/values.md#literal-payload-encoding)                                                                            | Integer and floating-point literal payloads survive lowering and emit as recognizable numeric tokens on every text-emit target.               | `literal-emit-multi-target.slang`              |

## Tests in this bundle

| File                                          | Intent     | Doc anchor                                |
| --------------------------------------------- | ---------- | ----------------------------------------- |
| `literals-inline-payload.slang`               | functional | `#literal-payload-encoding`               |
| `return-val-void-constant.slang`              | functional | `#literals-constant-group`                |
| `arith-irem-frem-neg.slang`                   | functional | `#arithmetic-and-bitwise`                 |
| `arith-shifts.slang`                          | functional | `#arithmetic-and-bitwise`                 |
| `arith-bitwise.slang`                         | functional | `#arithmetic-and-bitwise`                 |
| `logical-not.slang`                           | functional | `#arithmetic-and-bitwise`                 |
| `bitfield-extract-insert.slang`               | functional | `#arithmetic-and-bitwise`                 |
| `select-builtin.slang`                        | functional | `#select`                                 |
| `comparison-eq-ne.slang`                      | functional | `#comparison`                             |
| `comparison-lt-le-ge.slang`                   | functional | `#comparison`                             |
| `conversion-intcast-narrow.slang`             | functional | `#conversions`                            |
| `conversion-floatcast.slang`                  | functional | `#conversions`                            |
| `conversion-int-float.slang`                  | functional | `#conversions`                            |
| `conversion-bitcast.slang`                    | functional | `#conversions`                            |
| `conversion-cast-to-void.slang`               | functional | `#conversions`                            |
| `memory-get-field-rvalue.slang`               | functional | `#fieldaddress-vs-fieldextract`           |
| `memory-getelement-and-getelementptr.slang`   | functional | `#getelementptr-vs-getelement`            |
| `memory-global-constant.slang`                | functional | `#memory`                                 |
| `swizzle-multi-component.slang`               | functional | `#swizzle-swizzleset-swizzledstore`       |
| `swizzle-store-lvalue.slang`                  | functional | `#swizzle-swizzleset-swizzledstore`       |
| `swizzle-emit-multi-target.slang`             | functional | `#swizzle-swizzleset-swizzledstore`       |
| `aggregate-makevector.slang`                  | functional | `#aggregate-constructors`                 |
| `aggregate-makematrix.slang`                  | functional | `#aggregate-constructors`                 |
| `aggregate-maketuple.slang`                   | functional | `#aggregate-constructors`                 |
| `optional-make-has-value.slang`               | functional | `#result-optional-conditional-helpers`    |
| `out-param-type.slang`                        | functional | `#conversions`                            |
| `literal-emit-multi-target.slang`             | functional | `#literal-payload-encoding`               |
| `add-int32-max-literal.slang`                 | boundary   | `#literal-payload-encoding`               |
| `add-int32-min-literal.slang`                 | boundary   | `#literal-payload-encoding`               |
| `add-uint32-max-literal.slang`                | boundary   | `#literal-payload-encoding`               |
| `cast-uint8-max-plus-one-wraps.slang`         | boundary   | `#literal-payload-encoding`               |
| `add-int64-max-literal.slang`                 | boundary   | `#literal-payload-encoding`               |
| `store-int8-min-literal.slang`                | boundary   | `#literal-payload-encoding`               |
| `store-uint8-max-literal.slang`               | boundary   | `#literal-payload-encoding`               |
| `store-int16-max-literal.slang`               | boundary   | `#literal-payload-encoding`               |
| `mul-float-pos-zero-literal.slang`            | boundary   | `#literal-payload-encoding`               |
| `mul-float-neg-zero-literal.slang`            | boundary   | `#literal-payload-encoding`               |
| `add-float-max-literal.slang`                 | boundary   | `#literal-payload-encoding`               |
| `add-float-smallest-normal-literal.slang`     | boundary   | `#literal-payload-encoding`               |
| `add-double-overflow-folds-to-inf.slang`      | boundary   | `#literal-payload-encoding`               |
| `int-literal-overflow-any-type-rejected.slang`| negative   | `#literal-payload-encoding`               |
| `cmpEQ-float-pos-zero-literal.slang`          | boundary   | `#comparison`                             |
| `cmpEQ-float-neg-zero-literal.slang`          | boundary   | `#comparison`                             |
| `cmpEQ-self-uniform-float.slang`              | boundary   | `#comparison`                             |
| `cmpNE-self-uniform-float.slang`              | boundary   | `#comparison`                             |
| `cmpLE-cmpGE-float-uniform.slang`             | boundary   | `#comparison`                             |
| `intCast-int-to-uint8-narrowing.slang`        | boundary   | `#conversions`                            |
| `intCast-int-to-uint64-widening.slang`        | boundary   | `#conversions`                            |
| `intCast-int-to-uint-same-width.slang`        | boundary   | `#conversions`                            |
| `floatCast-float-to-half-narrowing.slang`     | boundary   | `#conversions`                            |
| `floatCast-half-to-double-widening.slang`     | boundary   | `#conversions`                            |
| `bitCast-uint-to-float-reinterpret.slang`     | boundary   | `#conversions`                            |
| `swizzle-single-component-from-vec4.slang`    | boundary   | `#swizzle-swizzleset-swizzledstore`       |
| `swizzle-full-length-xyzw.slang`              | boundary   | `#swizzle-swizzleset-swizzledstore`       |
| `swizzle-repeat-component-xxxx.slang`         | boundary   | `#swizzle-swizzleset-swizzledstore`       |
| `swizzle-mixed-xxxy-from-vec2.slang`          | boundary   | `#swizzle-swizzleset-swizzledstore`       |
| `swizzle-z-on-vec2-rejected.slang`            | negative   | `#swizzle-swizzleset-swizzledstore`       |
| `swizzle-w-on-vec3-rejected.slang`            | negative   | `#swizzle-swizzleset-swizzledstore`       |
| `bitfieldExtract-count-zero.slang`            | boundary   | `#arithmetic-and-bitwise`                 |
| `bitfieldExtract-count-full-width.slang`      | boundary   | `#arithmetic-and-bitwise`                 |
| `bitfieldExtract-offset-high-edge.slang`      | boundary   | `#arithmetic-and-bitwise`                 |
| `bitfieldInsert-count-full-width.slang`       | boundary   | `#arithmetic-and-bitwise`                 |
| `shl-uint-by-zero.slang`                      | boundary   | `#arithmetic-and-bitwise`                 |
| `shl-uint-by-width-minus-one.slang`           | boundary   | `#arithmetic-and-bitwise`                 |
| `shl-uint-by-width-overflow.slang`            | boundary   | `#arithmetic-and-bitwise`                 |
| `shr-int-by-zero.slang`                       | boundary   | `#arithmetic-and-bitwise`                 |
| `shr-int-by-width-minus-one.slang`            | boundary   | `#arithmetic-and-bitwise`                 |
| `irem-int-by-one.slang`                       | boundary   | `#arithmetic-and-bitwise`                 |
| `neg-uniform-int.slang`                       | boundary   | `#arithmetic-and-bitwise`                 |
| `neg-uniform-float.slang`                     | boundary   | `#arithmetic-and-bitwise`                 |
| `mul-uint-by-max-literal.slang`               | boundary   | `#arithmetic-and-bitwise`                 |
| `div-float-by-positive-zero-uniform.slang`    | boundary   | `#arithmetic-and-bitwise`                 |
| `int-divide-by-zero-literal-rejected.slang`   | negative   | `#arithmetic-and-bitwise`                 |
| `optional-always-none.slang`                  | boundary   | `#result-optional-conditional-helpers`    |
| `optional-always-value.slang`                 | boundary   | `#result-optional-conditional-helpers`    |
| `makeVector-float2-smallest-vector.slang`     | boundary   | `#aggregate-constructors`                 |
| `makeMatrix-float4x4-largest-documented.slang`| stress     | `#aggregate-constructors`                 |
| `_probe_diag.slang`                           | stress     | `#aggregate-constructors`                 |
| `select-true-literal-bool.slang`              | boundary   | `#select`                                 |
| `intcast-int32-to-int8-narrow.slang`          | expansion  | `#conversions`                            |
| `intcast-int8-to-int16-widen.slang`           | expansion  | `#conversions`                            |
| `intcast-int8-to-int64-widen-across-extend.slang` | expansion | `#conversions`                          |
| `intcast-int64-to-uint16-narrow-across-signedness.slang` | expansion | `#conversions`                   |
| `intcast-uint8-to-int32-zero-extend.slang`    | expansion  | `#conversions`                            |
| `intcast-uint64-to-int64-same-width-flip.slang` | expansion | `#conversions`                          |
| `intcast-uint32-to-int8-narrow-across-signedness.slang` | expansion | `#conversions`                    |
| `intcast-int16-to-uint8-narrow-across-signedness.slang` | expansion | `#conversions`                    |
| `intcast-int32-to-int64-sign-extend.slang`    | expansion  | `#conversions`                            |
| `intcast-int64-to-int32-truncate.slang`       | expansion  | `#conversions`                            |
| `floatcast-double-to-float-narrow.slang`      | expansion  | `#conversions`                            |
| `floatcast-double-to-half-precision-loss.slang` | expansion | `#conversions`                          |
| `floatcast-half-to-float-widen.slang`         | expansion  | `#conversions`                            |
| `floatcast-float-to-double-widen.slang`       | expansion  | `#conversions`                            |
| `castinttofloat-int32-to-float.slang`         | expansion  | `#conversions`                            |
| `castinttofloat-uint32-to-float.slang`        | expansion  | `#conversions`                            |
| `castinttofloat-int64-to-double.slang`        | expansion  | `#conversions`                            |
| `castinttofloat-uint8-to-float.slang`         | expansion  | `#conversions`                            |
| `castinttofloat-int32-to-half.slang`          | expansion  | `#conversions`                            |
| `castinttofloat-int16-to-double.slang`        | expansion  | `#conversions`                            |
| `castfloattoint-float-to-int32.slang`         | expansion  | `#conversions`                            |
| `castfloattoint-double-to-int32.slang`        | expansion  | `#conversions`                            |
| `castfloattoint-double-to-int64.slang`        | expansion  | `#conversions`                            |
| `castfloattoint-double-to-int8-narrow.slang`  | expansion  | `#conversions`                            |
| `castfloattoint-half-to-uint32.slang`         | expansion  | `#conversions`                            |
| `castfloattoint-float-to-uint32.slang`        | expansion  | `#conversions`                            |
| `castfloattoint-half-to-int16.slang`          | expansion  | `#conversions`                            |
| `bitcast-float-to-uint32-reinterpret.slang`   | expansion  | `#conversions`                            |
| `bitcast-uint64-to-double-reinterpret.slang`  | expansion  | `#conversions`                            |
| `bitcast-double-to-uint64-reinterpret.slang`  | expansion  | `#conversions`                            |
| `bitcast-uint16-to-half-reinterpret.slang`    | expansion  | `#conversions`                            |
| `bitcast-half-to-uint16-reinterpret.slang`    | expansion  | `#conversions`                            |
| `bitcast-int32-to-float-reinterpret.slang`    | expansion  | `#conversions`                            |
| `bitcast-different-width-rejected.slang`      | negative   | `#conversions`                            |
| `cmplt-int-vs-float-implicit-conversion.slang` | expansion | `#conversions`                           |
| `cmpeq-int-vs-half-implicit-conversion.slang` | expansion  | `#conversions`                            |
| `select-mixed-int-float-arms-implicit-conversion.slang` | expansion | `#conversions`                    |
| `add-int8-plus-int16-promotes-via-intcast.slang` | expansion | `#conversions`                         |
| `select-vector-float3-arms.slang`             | expansion  | `#select`                                 |
| `select-int64-arms.slang`                     | expansion  | `#select`                                 |
| `select-double-arms.slang`                    | expansion  | `#select`                                 |
| `cmpeq-int8-uniform.slang`                    | expansion  | `#comparison`                             |
| `cmplt-int64-uniform.slang`                   | expansion  | `#comparison`                             |
| `cmpne-uint-uniform.slang`                    | expansion  | `#comparison`                             |
| `cmple-double-uniform.slang`                  | expansion  | `#comparison`                             |

## Housekeeping

- `_probe_diag.slang` is a real stress test (two `makeVector` calls
  feeding a vector `add`); the underscore-prefixed filename is a
  legacy artifact of the expansion pass and should be renamed to
  something like `makeVector-pair-feeding-add-stress.slang` on the
  next regeneration. The lint accepts it as-is.

## Doc gaps observed

- The doc's Logical table lists `logicalAnd` (`InfixExpr` `&&`) and
  `logicalOr` (`InfixExpr` `||`), but the observed lowering of the
  short-circuit operators is a pair of `ifElse` terminator blocks
  joining at a `param` — the same shape the doc describes for the
  ternary `?:`. No portable surface in this bundle reliably produces
  a `logicalAnd` or `logicalOr` IR opcode on the LOWER-TO-IR stage.
  Either the lowering should emit these opcodes (and the doc would
  be honoured) or the doc should be updated to note that the opcodes
  are reserved for future use / produced by an IR pass rather than
  lowering.
- The doc's `select` notable-opcode note says `select` "lowers from
  `SelectExpr` and ternary `cond ? a : b`", but the observed
  lowering of the ternary uses short-circuit `ifElse` + block
  `param`, not `select`. Only the `select(...)` builtin reliably
  surfaces the `select` opcode. The doc should narrow the "AST
  origin" cell to "`SelectExpr` / `select(...)` builtin".
- The Aggregate constructors table lists `makeStruct` with AST
  origin "`MakeStructExpr`, aggregate-initializer lowering", but
  Slang's natural aggregate-initializer surface (`Foo f = { a, b,
  c };` or `Foo f = Foo();`) lowers to a synthesized
  `call %Foox5Fx24init(...)` rather than a `makeStruct(...)` IR
  opcode on the LOWER-TO-IR stage. The doc should either describe
  the synthesized initializer path or name a surface that reliably
  surfaces `makeStruct`.
- The Aggregate constructors table lists `makeArray` with AST
  origin "`MakeArrayExpr`". The observed lowering of an array
  initializer (`int arr[3] = { a, b, c };`) streams the components
  straight into per-index `store` instructions on a local `var`,
  with no `makeArray` opcode in the dump. The same comment applies
  as for `makeStruct`.
- The Aggregate constructors table lists `getTupleElement` for
  "`MemberExpr` on a tuple". The observed lowering of `t._0`
  (where `t` is a `Tuple<int, float>`) is a `swizzle(%t, 0 : Int)`
  — using the same swizzle opcode that handles vector lanes — not
  `getTupleElement`. The doc should clarify which surface produces
  `getTupleElement`.
- The Conversions table lists `outImplicitCast` and
  `inOutImplicitCast` for "Function-arg lowering for `out` / `inout`
  parameters". The observed lowering does not emit either opcode
  when the caller's argument type matches the parameter; an
  implicit-cast at the out-parameter boundary is rejected by
  semantic check (error `E30047` "argument must be l-value") rather
  than producing the cast. The opcodes appear to be reserved for an
  IR pass; a surface that produces them at LOWER-TO-IR is unclear
  from the doc.
- The Conversions table lists `BuiltinCast`, `unmodified`,
  `reinterpret`, `ReinterpretOptional` and `PtrCast` with various
  "synthesized" or `BitCastExpr`-style origins, but does not name
  a portable shader-language surface that produces each. A
  one-sentence "surface" cell per row would let the agent test them.
- The Conversions table lists `CastEnumToInt`, `CastIntToEnum`,
  `EnumCast`. Slang enums do exist as a surface, but the doc does
  not name which assignment / cast triggers which of the three
  opcodes (Slang's `enum`-to-`int` conversion is typically implicit
  on arithmetic; explicit `int(myEnum)` is the natural surface for
  `CastEnumToInt`, but `EnumCast` between two distinct enums of the
  same underlying type has no documented surface).
- The Memory table lists `defaultConstruct` with AST origin
  "`DefaultConstructExpr` and synthesized in IR passes". Observed
  lowering of `Foo f = Foo();` or `Foo f = {};` produces a
  synthesized `call %Foox5Fx24init(0 : Int, 0 : Float)` — that is,
  the `$init` function call with zero literals — not a
  `defaultConstruct` opcode on the LOWER-TO-IR stage. The doc
  should either name a surface that reliably produces
  `defaultConstruct` at lowering, or move the opcode to a list of
  IR-pass-introduced opcodes.
- The Memory table lists `alloca` for "Dynamically-sized stack
  allocation" with AST origin "`AllocaExpr` / dynamic-stack
  lowering". Slang's surface for dynamic-sized stack allocation is
  not documented; on most targets a runtime-size array uses a
  different lowering path entirely.
- The Constexpr arithmetic and casts table lists 28 opcodes
  (`constexprAdd`, `constexprMul`, ..., `constexprCastEnumToInt`)
  with the note "Hoistable variants ... used to lower compile-time
  integer expressions (`IntVal` subclasses such as
  `PolynomialIntVal`) so that identical compile-time values dedupe."
  These are produced by the IR's compile-time integer evaluator,
  not by AST-to-IR lowering. The doc should clarify that they are
  not part of the LOWER-TO-IR catalog and direct the reader to the
  `pipeline/05-ir-passes` bundle (where they belong if anywhere).
- The `MakeUInt64` notable-opcode note says it "constructs a 64-bit
  value from two 32-bit halves" because "several targets do not
  have a direct `uint64` literal form". Observed lowering of a
  `uint64_t x = 0xDEADBEEF12345678uL;` and of `uint64_t(hi) << 32 |
  uint64_t(lo)` compositions does not surface `makeUInt64` at
  LOWER-TO-IR — it appears to be produced later, during emit.
- The Strings and native pointers / Object and CUDA helpers
  families list `makeString`, `getNativeStr`, `getNativePtr`,
  `getManagedPtrWriteRef`, `ManagedPtrAttach`, `ManagedPtrDetach`,
  `allocObj`, `CUDA_LDG` — all marked "(synthesized)" or
  host-side. No portable shader-language surface in the doc
  produces them; per-opcode tests cannot be anchored.
- The Conversions table does **not** mention the HLSL-compatible
  reinterpret builtins `asuint`/`asint`/`asfloat`/`asdouble`. Probing
  reveals that on the LOWER-TO-IR stage these surface as
  `call %asuint(...)` etc. with `targetIntrinsic` decoration; they
  are not lowered to the `bitCast` opcode at lowering time. The
  doc should either name the surface that lowers these intrinsics
  to `bitCast` (presumably an emit-side rewrite) or describe them
  as call-shaped throughout the value pipeline.
- The Aggregate constructors family lists `makeMatrixFromScalar`,
  `MakeVectorFromScalar`, `makeArrayFromElement`, `makeCoopVector`,
  `makeCoopVectorFromValuePack`, `makeCoopMatrixFromScalar`,
  `makeTargetTuple`, `matrixReshape`, `vectorReshape`,
  `SumVectorElements`, `SumMatrixElements`, `updateElement` — all
  with synthesized / implicit origins. The doc should name the
  surface that produces each (e.g. is `float3(x)` the surface for
  `MakeVectorFromScalar` or is it lowered as `makeVector(x, x, x)`?).
- The Result / Optional / Conditional helpers family lists
  `makeResultValue`/`makeResultError`/`isResultError`/
  `getResultValue`/`getResultError` for `Result<T, E>`. The core
  module presumably exposes `Result<T, E>` but the doc does not
  name the surface form of `Result::makeValue(x)` /
  `Result::makeError(e)`.
- The Undefined and default-construct family lists `Poison` and
  `LoadFromUninitializedMemory` as opcodes "synthesized". The
  observed IR dump shows a single `Poison` declaration at the top
  of the LOWER-TO-IR stage (as a sigil), but no surface in the doc
  produces it as an active operand. The doc should clarify whether
  the top-of-dump `Poison` line is the entire surface or whether
  some construct yields an active `Poison`-typed value.

## Out of scope (no-GPU runner)

(In this bundle the heading is used for "claims unobservable through
any allowed test directive", consistent with the
`cross-cutting/ir-instructions`, `ir-reference/types`, and
`pipeline/04-ast-to-ir` bundles.)

- **Hoistability flags** (`H` in the doc's tables) — the IR dump
  shows the post-hoist result, not the decision. Two identical
  `IntLit 42` payloads collapsing to a single operand is observable
  in the dump's textual output, but the *reason* (hoisting via
  `IRBuilder` deduplication) is an internal mechanism.
- The **C++ wrapper struct** identity for each opcode (e.g. that
  `add` is `IRAdd` in C++, `boolConst` is `BoolLit`) — internal API
  visible only through `<inst>->getOp()` / `as<IRBoolLit>()`.
- The **`IRBuilder` emitter** for each opcode (`emitAdd`,
  `emitLoad`, `emitMakeStruct`, ...) — internal C++ API.
- **`Poison`** as an active operand — the LOWER-TO-IR dump shows
  `Poison` only as a top-level sigil declaration, not as an active
  operand of any user instruction. A reading-an-uninitialized-local
  diagnostic is a frontend check, not a positive observation of an
  IR opcode.
- **`LoadFromUninitializedMemory`** — same as `Poison`; the doc
  describes its semantics ("like LLVM's `freeze(undef)`") but no
  surface in the doc produces it at LOWER-TO-IR.
- **`alloca`** — no documented portable Slang surface produces it
  at LOWER-TO-IR. Runtime-size arrays use a different lowering on
  most targets.
- **`copyLogical`, `assumeAddress`, `getAddr`, `getOffsetPtr`,
  `swizzleSet`, `matrixSwizzleStore`, `updateElement`** —
  "(synthesized)" per the doc; no portable Slang surface produces
  them at LOWER-TO-IR.
- **`makeString`, `getNativeStr`, `getNativePtr`,
  `getManagedPtrWriteRef`, `ManagedPtrAttach`, `ManagedPtrDetach`,
  `allocObj`** — host-side or COM-pointer machinery, not
  exercisable from a shader.
- **`CUDA_LDG`** — emitted by the CUDA backend's read-only buffer
  lowering, not by AST-to-IR lowering. The cross-cutting bundle
  notes the `__ldg` factoring as a CUDA emit consequence.
- **`makeMatrixFromScalar`, `MakeVectorFromScalar`,
  `makeArrayFromElement`** — implicit lowerings whose surface is
  not documented. `float3(x)` may or may not produce them; the
  observed lowering produces `makeVector(x, x, x)` instead.
- **`makeCoopVector`, `makeCoopVectorFromValuePack`,
  `makeCoopMatrixFromScalar`** — cooperative vector / matrix
  intrinsics; no portable Slang surface in the doc names them.
- **`makeTargetTuple`, `getTargetTupleElement`** — target-keyed
  tuple values used by `targetSwitch`; "(synthesized)" per the doc.
- **`SumVectorElements`, `SumMatrixElements`, `matrixReshape`,
  `vectorReshape`** — all "(synthesized)" per the doc.
- **`Result<T, E>` helpers** (`makeResultValue`/`makeResultError`/
  `isResultError`/`getResultValue`/`getResultError`) — the doc
  cites `Result<T, E>::makeValue` etc. but the surface form on the
  no-GPU runner is unclear from the doc.
- **`makeConditionalValue`/`getConditionalValue`** —
  "(synthesized)" per the doc; no portable surface.
- **`extractTaggedUnionTag`/`extractTaggedUnionPayload`** — the
  doc itself cross-references `generics-and-existentials.md`;
  produced by existential elimination.
- **`makeUInt64`** — produced during emit / late lowering, not at
  LOWER-TO-IR.
- **`Constexpr*` family** (`constexprAdd`, `constexprMul`, ...,
  `constexprCastEnumToInt`) — produced by the IR's compile-time
  integer evaluator (`IntVal` lowering), not by AST-to-IR.
- **`ReinterpretOptional`, `unmodified`, `outImplicitCast`,
  `inOutImplicitCast`, `BuiltinCast`, `PtrCast`, `CastPtrToBool`,
  `CastPtrToInt`, `CastIntToPtr`, `CastUInt2ToDescriptorHandle`,
  `CastDescriptorHandleToUInt2`** — "(synthesized)" or host-side;
  no portable Slang surface in the doc names them.
- **`CastEnumToInt`, `CastIntToEnum`, `EnumCast`** — the doc lists
  them but does not name which surface produces which (Slang's
  enum-to-int may be implicit in arithmetic context, blurring the
  per-opcode boundary).
- **Hoistable instruction deduplication** — observable only by
  comparing `IRInst*` identity, not visible in the dump.
- **The `_debugUID`/`m_op` packed bit layout** — internal to
  `IRInst::m_op`.
