---
generated: true
model: claude-opus-4-7
generated_at: 2026-05-21T00:00:00+00:00
source_commit: 690f6a3084801386b77186394e0f6e8c120824a4
watched_paths_digest: 4cd2b0ab91da080eb6a16ece95070e661cf2096b991cd6d164bfccb383236671
source_doc: docs/llm-generated/ir-reference/types.md
source_doc_digest: 1c525783aca7f77dc841cbd36fd6911b8ec56b1fee8cffc96d4938b604220a84
warning: "Auto-generated. May drift from source. Do not edit by hand."
---

# Tests for ir-reference/types

## Intent

Tests verify the per-opcode catalog of the IR `Type` family
described in
[`docs/llm-generated/ir-reference/types.md`](../../../docs/llm-generated/ir-reference/types.md):
that each documented IR type opcode (basic scalars
`Bool`/`Int*`/`UInt*`/`Half`/`Float`/`Double`/`Void`,
`Vec(elementType, elementCount)`,
`Mat(elementType, rowCount, columnCount, layout)`,
`Array(elementType, elementCount)` and `UnsizedArray(elementType)`,
`Func(resultType, paramTypes...)`,
`Ptr(valueType, ...)` and rate-qualified pointer types
(`RateQualified(GroupShared, Ptr(<T>))`),
`struct`, `interface` with `interface_req_entry`/`this_type`,
`witness_table_t` with `witness_table_entry`,
`Optional`, `tuple_type`, `type_t`, and the resource family
(`RWStructuredBuffer`, `StructuredBuffer`, `ConstantBuffer`,
`ByteAddressBuffer`, `RWByteAddressBuffer`, `SamplerState`,
`Texture2D` via `TextureType`)) appears in `-dump-ir` output for the
obvious surface construct that produces it, and that target-divergent
type lowerings (vector spelling, matrix wrapper, struct names) emit
the documented per-target text on the text-emit backends (HLSL,
GLSL, Metal, WGSL, CPP).

The primary observation mechanism is
`-target spirv-asm -dump-ir -o /dev/null` followed by a FileCheck
against the IR dump. Anchors are user-named functions, user structs,
and user `global_param`s — the IR dump's preamble is large and any
unanchored pattern risks false positives. For target-divergent
lowerings (vectors, matrices, struct-field wrappers, buffer-type
wrappers) the test carries one SIMPLE directive per feasible
text-emit target with a distinct CHECK prefix, because the doc's
notable-opcodes section does not claim cross-target uniformity for
type spellings.

Cross-cutting type observations that the `cross-cutting/ir-instructions`
bundle already covers at a sample level (`Vec`, `Array`, `Ptr` from a
local `var`) are not duplicated here; this bundle drills into the
rest of the catalog without repeating those.

## Claims enumerated

| Claim ID | Anchor                                                                                                                                                                                                | Claim (one line)                                                                                                                          | Tests                                          |
| -------- | ----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ----------------------------------------------------------------------------------------------------------------------------------------- | ---------------------------------------------- |
| C-01     | [#basic-scalar-types](../../../docs/llm-generated/ir-reference/types.md#basic-scalar-types)                                                                                                           | The signed-integer scalar opcodes are `Int8`, `Int16`, `Int`, `Int64`.                                                                    | [`basic-scalars-int-family.slang`](basic-scalars-int-family.slang)               |
| C-02     | [#basic-scalar-types](../../../docs/llm-generated/ir-reference/types.md#basic-scalar-types)                                                                                                           | The unsigned-integer scalar opcodes are `UInt8`, `UInt16`, `UInt`, `UInt64`.                                                              | [`basic-scalars-uint-family.slang`](basic-scalars-uint-family.slang)              |
| C-03     | [#basic-scalar-types](../../../docs/llm-generated/ir-reference/types.md#basic-scalar-types)                                                                                                           | The floating-point scalar opcodes are `Half`, `Float`, `Double`.                                                                          | [`basic-scalars-float-family.slang`](basic-scalars-float-family.slang)             |
| C-04     | [#basic-scalar-types](../../../docs/llm-generated/ir-reference/types.md#basic-scalar-types)                                                                                                           | The `Bool` scalar opcode is the IR encoding of `bool`; a `void` function's IR type uses `Void`.                                           | [`basic-bool-and-void.slang`](basic-bool-and-void.slang)                    |
| C-05     | [#vec-and-mat](../../../docs/llm-generated/ir-reference/types.md#vec-and-mat)                                                                                                                         | `Vec(elementType, elementCount)` is the IR encoding of HLSL `vector<T,N>`; a `uintN` parameter is typed `Vec(UInt, N : Int)` in the dump. | [`vec-elementtype-elementcount.slang`](vec-elementtype-elementcount.slang)           |
| C-06     | [#vec-and-mat](../../../docs/llm-generated/ir-reference/types.md#vec-and-mat)                                                                                                                         | A `float3` value lowers to per-target spellings: HLSL `float3`, GLSL `vec3`, Metal `float3`, WGSL `vec3<f32>`, CPP `Vector<float, 3>`.    | [`vec-emit-multi-target.slang`](vec-emit-multi-target.slang)                  |
| C-07     | [#vec-and-mat](../../../docs/llm-generated/ir-reference/types.md#vec-and-mat)                                                                                                                         | `Mat(elementType, rowCount, columnCount, layout)` adds two shape operands and a layout int literal; a `float4x4` field is `Mat(Float, 4 : Int, 4 : Int, L : Int)`. | [`mat-shape-operands.slang`](mat-shape-operands.slang)                     |
| C-08     | [#vec-and-mat](../../../docs/llm-generated/ir-reference/types.md#vec-and-mat)                                                                                                                         | A `float4x4` matrix lowers to per-target spellings: HLSL `float4x4`, GLSL `mat4x4`, WGSL `mat4x4<f32>`.                                    | [`mat-emit-multi-target.slang`](mat-emit-multi-target.slang)                  |
| C-09     | [#arrays](../../../docs/llm-generated/ir-reference/types.md#arrays)                                                                                                                                   | A fixed-size array type appears as `Array(elementType, elementCount)`.                                                                    | [`array-fixed-size.slang`](array-fixed-size.slang)                       |
| C-10     | [#array-vs-unsizedarray](../../../docs/llm-generated/ir-reference/types.md#array-vs-unsizedarray)                                                                                                     | `UnsizedArray(elementType)` omits the element-count operand and represents a runtime-extent array.                                        | [`array-unsized.slang`](array-unsized.slang)                          |
| C-11     | [#func](../../../docs/llm-generated/ir-reference/types.md#func)                                                                                                                                       | `Func(resultType, paramTypes...)` carries the result type as the leading operand; an `int helper(int, float)` shows `Func(Int, Int, Float)`. | [`func-result-then-params.slang`](func-result-then-params.slang)                |
| C-12     | [#ptr-and-the-access-qualifier-address-space-operands](../../../docs/llm-generated/ir-reference/types.md#ptr-and-the-access-qualifier-address-space-operands)                                         | `Ptr(valueType, ...)` is the pointer-type opcode; a local `var` produces a value of type `Ptr<T>`.                                        | [`ptr-from-struct-var.slang`](ptr-from-struct-var.slang)                    |
| C-13     | [#rates-and-rate-qualified-types](../../../docs/llm-generated/ir-reference/types.md#rates-and-rate-qualified-types)                                                                                   | A `groupshared` declaration's IR type is `RateQualified(GroupShared, Ptr(<T>))`.                                                          | [`ptr-groupshared-rate-qualified.slang`](ptr-groupshared-rate-qualified.slang)         |
| C-14     | [#struct-and-class-containers](../../../docs/llm-generated/ir-reference/types.md#struct-and-class-containers)                                                                                         | A user `struct` lowers to a parent `struct %Name` opcode containing `field(%key, <T>)` rows.                                              | [`struct-with-fields.slang`](struct-with-fields.slang)                     |
| C-15     | [#existentials-and-interfaces](../../../docs/llm-generated/ir-reference/types.md#existentials-and-interfaces)                                                                                         | An `interface` declaration produces `interface_req_entry(...)` instructions whose method-type uses `this_type(%IFoo)`.                    | [`interface-and-this-type.slang`](interface-and-this-type.slang)                |
| C-16     | [#witness-table-types](../../../docs/llm-generated/ir-reference/types.md#witness-table-types)                                                                                                         | A conformance produces a `witness_table %... : witness_table_t(%IFoo)(%S)` value with `witness_table_entry(...)` rows.                    | [`witness-table-type.slang`](witness-table-type.slang)                     |
| C-17     | [#vectors-matrices-and-composite](../../../docs/llm-generated/ir-reference/types.md#vectors-matrices-and-composite)                                                                                   | `Optional(valueType)` is the IR encoding of `Optional<T>`.                                                                                | [`optional-type.slang`](optional-type.slang)                          |
| C-18     | [#tuples-packs-and-target-tuples](../../../docs/llm-generated/ir-reference/types.md#tuples-packs-and-target-tuples)                                                                                   | `tuple_type(types...)` is the IR encoding of a heterogeneous tuple constructed by `makeTuple`.                                            | [`tuple-type.slang`](tuple-type.slang)                             |
| C-19     | [#spir-v-literals-and-kinds](../../../docs/llm-generated/ir-reference/types.md#spir-v-literals-and-kinds)                                                                                             | A generic type-parameter is typed as `type_t` in the IR dump.                                                                             | [`type-t-generic-param.slang`](type-t-generic-param.slang)                   |
| C-20     | [#resource-and-texture-types](../../../docs/llm-generated/ir-reference/types.md#resource-and-texture-types)                                                                                           | `RWStructuredBuffer<T>` lowers to `RWStructuredBuffer(<T>, <layout>, ...)` as a `global_param`.                                           | [`rwstructuredbuffer-type.slang`](rwstructuredbuffer-type.slang)                |
| C-21     | [#resource-and-texture-types](../../../docs/llm-generated/ir-reference/types.md#resource-and-texture-types)                                                                                           | Read-only `StructuredBuffer<T>` lowers to `StructuredBuffer(<T>, <layout>, ...)` as a `global_param`.                                     | [`structuredbuffer-type.slang`](structuredbuffer-type.slang)                  |
| C-22     | [#resource-and-texture-types](../../../docs/llm-generated/ir-reference/types.md#resource-and-texture-types)                                                                                           | `ConstantBuffer<T>` lowers to `ConstantBuffer(<T>, <layout>, ...)` as a `global_param`.                                                   | [`constantbuffer-type.slang`](constantbuffer-type.slang)                    |
| C-23     | [#resource-and-texture-types](../../../docs/llm-generated/ir-reference/types.md#resource-and-texture-types)                                                                                           | `ByteAddressBuffer` and `RWByteAddressBuffer` lower as IR opcodes of the same spelling, with no element-type operand.                     | [`byteaddressbuffer-type.slang`](byteaddressbuffer-type.slang)                 |
| C-24     | [#resource-and-texture-types](../../../docs/llm-generated/ir-reference/types.md#resource-and-texture-types)                                                                                           | A `uniform SamplerState` declaration lowers to a `global_param` of IR type `SamplerState`.                                                | [`samplerstate-type.slang`](samplerstate-type.slang)                      |
| C-25     | [#resource-and-texture-types](../../../docs/llm-generated/ir-reference/types.md#resource-and-texture-types)                                                                                           | A `uniform Texture2D<float4>` declaration lowers to a `global_param` whose IR type starts with `Texture2D` (a `TextureType` instance).    | [`texture-type.slang`](texture-type.slang)                           |
| C-26     | [#vec-and-mat](../../../docs/llm-generated/ir-reference/types.md#vec-and-mat)                                                                                                                         | Assigning a `vector<int,3>` to a `vector<int,4>` is rejected as a type mismatch.                                                          | [`vector-size-mismatch-negative.slang`](vector-size-mismatch-negative.slang)          |
| C-27     | [#basic-scalar-types](../../../docs/llm-generated/ir-reference/types.md#basic-scalar-types)                                                                                                           | Arithmetic and comparison on the narrow integer scalars (`Int8`, `UInt8`, `Int16`, `Int64`) preserve the operand width in the result type. | [`int8-arithmetic-add.slang`](int8-arithmetic-add.slang), [`uint8-comparison-eq.slang`](uint8-comparison-eq.slang), [`int16-comparison-lt.slang`](int16-comparison-lt.slang), [`int64-comparison-lt.slang`](int64-comparison-lt.slang) |
| C-28     | [#basic-scalar-types](../../../docs/llm-generated/ir-reference/types.md#basic-scalar-types)                                                                                                           | A narrowing integer conversion (`int -> int8_t`, `uint64_t -> uint16_t`) produces an `intCast` whose result type is the narrower scalar opcode. | [`int32-to-int8-narrow.slang`](int32-to-int8-narrow.slang), [`uint64-to-uint16-narrow.slang`](uint64-to-uint16-narrow.slang) |
| C-29     | [#basic-scalar-types](../../../docs/llm-generated/ir-reference/types.md#basic-scalar-types)                                                                                                           | A widening integer conversion (`uint8_t -> uint`) produces an `intCast` whose result type is the wider scalar opcode.                     | [`uint8-to-uint-widen.slang`](uint8-to-uint-widen.slang)                    |
| C-30     | [#basic-scalar-types](../../../docs/llm-generated/ir-reference/types.md#basic-scalar-types)                                                                                                           | Floating-point narrowing and widening (`double -> half`, `half -> float`) produce `floatCast` instructions whose result type matches the destination width. | [`double-to-half-narrow.slang`](double-to-half-narrow.slang), [`half-to-float-widen.slang`](half-to-float-widen.slang) |
| C-31     | [#vec-and-mat](../../../docs/llm-generated/ir-reference/types.md#vec-and-mat)                                                                                                                         | `Vec(elementType, elementCount)` composes with each narrow integer scalar (`Int8`, `UInt8`, `Int16`, `UInt16`) as the element type.       | [`int8-vector3-global-param.slang`](int8-vector3-global-param.slang), [`uint8-vector4-global-param.slang`](uint8-vector4-global-param.slang), [`int16-vector2-global-param.slang`](int16-vector2-global-param.slang), [`uint16-vector3-global-param.slang`](uint16-vector3-global-param.slang) |
| C-32     | [#vec-and-mat](../../../docs/llm-generated/ir-reference/types.md#vec-and-mat)                                                                                                                         | `Vec(elementType, elementCount)` composes with each 64-bit integer scalar (`Int64`, `UInt64`) as the element type.                        | [`int64-vector2-global-param.slang`](int64-vector2-global-param.slang), [`uint64-vector2-global-param.slang`](uint64-vector2-global-param.slang) |
| C-33     | [#vec-and-mat](../../../docs/llm-generated/ir-reference/types.md#vec-and-mat)                                                                                                                         | `Vec(elementType, elementCount)` composes with `Half` to produce `Vec(Half, N)` at the documented element counts 2 / 3 / 4.               | [`half-vector2-global-param.slang`](half-vector2-global-param.slang), [`half-vector3-global-param.slang`](half-vector3-global-param.slang), [`half-vector4-global-param.slang`](half-vector4-global-param.slang) |
| C-34     | [#vec-and-mat](../../../docs/llm-generated/ir-reference/types.md#vec-and-mat)                                                                                                                         | `Vec(elementType, elementCount)` composes with `Double` to produce `Vec(Double, N)` at the documented element counts 2 / 3 / 4.            | [`double-vector2-global-param.slang`](double-vector2-global-param.slang), [`double-vector3-global-param.slang`](double-vector3-global-param.slang), [`double-vector4-global-param.slang`](double-vector4-global-param.slang) |
| C-35     | [#vec-and-mat](../../../docs/llm-generated/ir-reference/types.md#vec-and-mat)                                                                                                                         | Vector arithmetic on `Vec(Half, N)` and `Vec(Double, N)` operands yields a result of the same vector type.                                | [`half2-arithmetic.slang`](half2-arithmetic.slang), [`half-vector3-arithmetic.slang`](half-vector3-arithmetic.slang), [`double-vector3-arithmetic.slang`](double-vector3-arithmetic.slang) |
| C-36     | [#vec-and-mat](../../../docs/llm-generated/ir-reference/types.md#vec-and-mat)                                                                                                                         | Element-wise narrowing of a vector (`double4 -> float4`) yields a vector value typed `Vec(<narrower>, N)`.                                | [`double4-narrow-to-float4.slang`](double4-narrow-to-float4.slang)               |
| C-37     | [#arrays](../../../docs/llm-generated/ir-reference/types.md#arrays)                                                                                                                                   | `Array(elementType, elementCount)` composes with the narrow integer (`Int8`, `UInt8`, `Int16`, `UInt16`), 64-bit integer (`Int64`, `UInt64`), and `Half` / `Double` floating-point scalars. | [`int8-array-element.slang`](int8-array-element.slang), [`uint8-array-element.slang`](uint8-array-element.slang), [`int16-array-element.slang`](int16-array-element.slang), [`uint16-array-element.slang`](uint16-array-element.slang), [`int64-array-element.slang`](int64-array-element.slang), [`uint64-array-element.slang`](uint64-array-element.slang), [`half-array-element.slang`](half-array-element.slang), [`double-array-element.slang`](double-array-element.slang) |
| C-38     | [#struct-and-class-containers](../../../docs/llm-generated/ir-reference/types.md#struct-and-class-containers)                                                                                         | A `struct` field of each narrow / wide / non-`Float` floating-point scalar (`Int8`, `UInt8`, `Int16`, `UInt16`, `Int64`, `UInt64`, `Half`, `Double`) lowers to a `field(%key, <Scalar>)` row. | [`int8-struct-field.slang`](int8-struct-field.slang), [`uint8-struct-field.slang`](uint8-struct-field.slang), [`int16-struct-field.slang`](int16-struct-field.slang), [`uint16-struct-field.slang`](uint16-struct-field.slang), [`int64-struct-field.slang`](int64-struct-field.slang), [`uint64-struct-field.slang`](uint64-struct-field.slang), [`half-struct-field.slang`](half-struct-field.slang), [`double-struct-field.slang`](double-struct-field.slang) |
| C-39     | [#resource-and-texture-types](../../../docs/llm-generated/ir-reference/types.md#resource-and-texture-types)                                                                                           | `RWStructuredBuffer` accepts each narrow / 64-bit / `Half` / `Double` scalar (and `half4` / `double4` vectors) as its element-type operand. | [`int8-rwstructuredbuffer-element.slang`](int8-rwstructuredbuffer-element.slang), [`uint8-rwstructuredbuffer-element.slang`](uint8-rwstructuredbuffer-element.slang), [`int16-rwstructuredbuffer-element.slang`](int16-rwstructuredbuffer-element.slang), [`uint16-rwstructuredbuffer-element.slang`](uint16-rwstructuredbuffer-element.slang), [`int64-rwstructuredbuffer-element.slang`](int64-rwstructuredbuffer-element.slang), [`uint64-rwstructuredbuffer-element.slang`](uint64-rwstructuredbuffer-element.slang), [`half-rwstructuredbuffer-element.slang`](half-rwstructuredbuffer-element.slang), [`half4-rwstructuredbuffer-element.slang`](half4-rwstructuredbuffer-element.slang), [`double-rwstructuredbuffer-element.slang`](double-rwstructuredbuffer-element.slang), [`double4-rwstructuredbuffer-element.slang`](double4-rwstructuredbuffer-element.slang) |
| C-40     | [#basic-scalar-types](../../../docs/llm-generated/ir-reference/types.md#basic-scalar-types)                                                                                                           | A scalar `Int16` / `UInt16` global_param surfaces the narrow integer opcode as the IR type, complementing the existing `Int8` / `UInt64` boundary tests. | [`int16-global-param.slang`](int16-global-param.slang), [`uint16-global-param.slang`](uint16-global-param.slang) |

## Tests in this bundle

| File                                          | Intent     | Doc anchor                                            |
| --------------------------------------------- | ---------- | ----------------------------------------------------- |
| [`basic-scalars-int-family.slang`](basic-scalars-int-family.slang)              | functional | `#basic-scalar-types`                                 |
| [`basic-scalars-uint-family.slang`](basic-scalars-uint-family.slang)             | functional | `#basic-scalar-types`                                 |
| [`basic-scalars-float-family.slang`](basic-scalars-float-family.slang)            | functional | `#basic-scalar-types`                                 |
| [`basic-bool-and-void.slang`](basic-bool-and-void.slang)                   | functional | `#basic-scalar-types`                                 |
| [`vec-elementtype-elementcount.slang`](vec-elementtype-elementcount.slang)          | functional | `#vec-and-mat`                                        |
| [`vec-emit-multi-target.slang`](vec-emit-multi-target.slang)                 | functional | `#vec-and-mat`                                        |
| [`mat-shape-operands.slang`](mat-shape-operands.slang)                    | functional | `#vec-and-mat`                                        |
| [`mat-emit-multi-target.slang`](mat-emit-multi-target.slang)                 | functional | `#vec-and-mat`                                        |
| [`array-fixed-size.slang`](array-fixed-size.slang)                      | functional | `#arrays`                                             |
| [`array-unsized.slang`](array-unsized.slang)                         | functional | `#array-vs-unsizedarray`                              |
| [`func-result-then-params.slang`](func-result-then-params.slang)               | functional | `#func`                                               |
| [`ptr-from-struct-var.slang`](ptr-from-struct-var.slang)                   | functional | `#ptr-and-the-access-qualifier-address-space-operands`|
| [`ptr-groupshared-rate-qualified.slang`](ptr-groupshared-rate-qualified.slang)        | functional | `#rates-and-rate-qualified-types`                     |
| [`struct-with-fields.slang`](struct-with-fields.slang)                    | functional | `#struct-and-class-containers`                        |
| [`interface-and-this-type.slang`](interface-and-this-type.slang)               | functional | `#existentials-and-interfaces`                        |
| [`witness-table-type.slang`](witness-table-type.slang)                    | functional | `#witness-table-types`                                |
| [`optional-type.slang`](optional-type.slang)                         | functional | `#vectors-matrices-and-composite`                     |
| [`tuple-type.slang`](tuple-type.slang)                            | functional | `#tuples-packs-and-target-tuples`                     |
| [`type-t-generic-param.slang`](type-t-generic-param.slang)                  | functional | `#spir-v-literals-and-kinds`                          |
| [`rwstructuredbuffer-type.slang`](rwstructuredbuffer-type.slang)               | functional | `#resource-and-texture-types`                         |
| [`structuredbuffer-type.slang`](structuredbuffer-type.slang)                 | functional | `#resource-and-texture-types`                         |
| [`constantbuffer-type.slang`](constantbuffer-type.slang)                   | functional | `#resource-and-texture-types`                         |
| [`byteaddressbuffer-type.slang`](byteaddressbuffer-type.slang)                | functional | `#resource-and-texture-types`                         |
| [`samplerstate-type.slang`](samplerstate-type.slang)                     | functional | `#resource-and-texture-types`                         |
| [`texture-type.slang`](texture-type.slang)                          | functional | `#resource-and-texture-types`                         |
| [`vector-size-mismatch-negative.slang`](vector-size-mismatch-negative.slang)         | negative   | `#vec-and-mat`                                        |
| [`scalar-bool-boundary.slang`](scalar-bool-boundary.slang)                  | boundary   | `#basic-scalar-types`                                 |
| [`scalar-int8-smallest.slang`](scalar-int8-smallest.slang)                  | boundary   | `#basic-scalar-types`                                 |
| [`scalar-int64-largest.slang`](scalar-int64-largest.slang)                  | boundary   | `#basic-scalar-types`                                 |
| [`scalar-uint64-largest.slang`](scalar-uint64-largest.slang)                 | boundary   | `#basic-scalar-types`                                 |
| [`scalar-half-smallest-float.slang`](scalar-half-smallest-float.slang)            | boundary   | `#basic-scalar-types`                                 |
| [`scalar-double-largest.slang`](scalar-double-largest.slang)                 | boundary   | `#basic-scalar-types`                                 |
| [`vec1-minimum-dimension.slang`](vec1-minimum-dimension.slang)                | boundary   | `#vec-and-mat`                                        |
| [`vec2-minimum-multi-element.slang`](vec2-minimum-multi-element.slang)            | boundary   | `#vec-and-mat`                                        |
| [`vec-int3-explicit-uintcount.slang`](vec-int3-explicit-uintcount.slang)           | boundary   | `#vec-and-mat`                                        |
| [`vec-double4-largest-element.slang`](vec-double4-largest-element.slang)           | boundary   | `#vec-and-mat`                                        |
| [`mat1x1-minimum-shape.slang`](mat1x1-minimum-shape.slang)                  | boundary   | `#vec-and-mat`                                        |
| [`mat-rectangular-2x3.slang`](mat-rectangular-2x3.slang)                   | boundary   | `#vec-and-mat`                                        |
| [`vector-size-narrowing-negative.slang`](vector-size-narrowing-negative.slang)        | negative   | `#vec-and-mat`                                        |
| [`matrix-shape-mismatch-negative.slang`](matrix-shape-mismatch-negative.slang)        | negative   | `#vec-and-mat`                                        |
| [`array-size-one.slang`](array-size-one.slang)                        | boundary   | `#arrays`                                             |
| [`array-of-array-nesting.slang`](array-of-array-nesting.slang)                | stress     | `#arrays`                                             |
| [`array-static-const-bound-256.slang`](array-static-const-bound-256.slang)          | stress     | `#arrays`                                             |
| [`array-of-struct-stress.slang`](array-of-struct-stress.slang)                | stress     | `#arrays`                                             |
| [`array-size-mismatch-negative.slang`](array-size-mismatch-negative.slang)          | negative   | `#arrays`                                             |
| [`func-zero-params-void-return.slang`](func-zero-params-void-return.slang)          | boundary   | `#func`                                               |
| [`func-eight-params-stress.slang`](func-eight-params-stress.slang)              | stress     | `#func`                                               |
| [`func-mixed-vector-params.slang`](func-mixed-vector-params.slang)              | boundary   | `#func`                                               |
| [`func-struct-result.slang`](func-struct-result.slang)                    | boundary   | `#func`                                               |
| [`struct-empty-boundary.slang`](struct-empty-boundary.slang)                 | boundary   | `#struct-and-class-containers`                        |
| [`struct-single-field-boundary.slang`](struct-single-field-boundary.slang)          | boundary   | `#struct-and-class-containers`                        |
| [`struct-nested-five-deep.slang`](struct-nested-five-deep.slang)               | stress     | `#struct-and-class-containers`                        |
| [`struct-with-vector-field.slang`](struct-with-vector-field.slang)              | boundary   | `#struct-and-class-containers`                        |
| [`ptr-from-array-var.slang`](ptr-from-array-var.slang)                    | boundary   | `#ptr-and-the-access-qualifier-address-space-operands`|
| [`ptr-to-nested-struct-field.slang`](ptr-to-nested-struct-field.slang)            | boundary   | `#ptr-and-the-access-qualifier-address-space-operands`|
| [`rwbuf-struct-element.slang`](rwbuf-struct-element.slang)                  | boundary   | `#resource-and-texture-types`                         |
| [`rwbuf-vector-element.slang`](rwbuf-vector-element.slang)                  | boundary   | `#resource-and-texture-types`                         |
| [`rwbuf-struct-of-vectors-stress.slang`](rwbuf-struct-of-vectors-stress.slang)        | stress     | `#resource-and-texture-types`                         |
| [`optional-vector-payload.slang`](optional-vector-payload.slang)               | boundary   | `#vectors-matrices-and-composite`                     |
| [`tuple-three-element-stress.slang`](tuple-three-element-stress.slang)            | stress     | `#tuples-packs-and-target-tuples`                     |
| [`witness-table-two-conformances.slang`](witness-table-two-conformances.slang)        | stress     | `#witness-table-types`                                |
| [`generic-on-vector-element.slang`](generic-on-vector-element.slang)             | stress     | `#spir-v-literals-and-kinds`                          |
| [`int8-vector3-global-param.slang`](int8-vector3-global-param.slang)             | expansion  | `#vec-and-mat`                                        |
| [`int8-array-element.slang`](int8-array-element.slang)                    | expansion  | `#arrays`                                             |
| [`int8-struct-field.slang`](int8-struct-field.slang)                     | expansion  | `#struct-and-class-containers`                        |
| [`int8-rwstructuredbuffer-element.slang`](int8-rwstructuredbuffer-element.slang)       | expansion  | `#resource-and-texture-types`                         |
| [`int32-to-int8-narrow.slang`](int32-to-int8-narrow.slang)                  | expansion  | `#basic-scalar-types`                                 |
| [`int8-arithmetic-add.slang`](int8-arithmetic-add.slang)                   | expansion  | `#basic-scalar-types`                                 |
| [`uint8-vector4-global-param.slang`](uint8-vector4-global-param.slang)            | expansion  | `#vec-and-mat`                                        |
| [`uint8-array-element.slang`](uint8-array-element.slang)                   | expansion  | `#arrays`                                             |
| [`uint8-struct-field.slang`](uint8-struct-field.slang)                    | expansion  | `#struct-and-class-containers`                        |
| [`uint8-rwstructuredbuffer-element.slang`](uint8-rwstructuredbuffer-element.slang)      | expansion  | `#resource-and-texture-types`                         |
| [`uint8-comparison-eq.slang`](uint8-comparison-eq.slang)                   | expansion  | `#basic-scalar-types`                                 |
| [`uint8-to-uint-widen.slang`](uint8-to-uint-widen.slang)                   | expansion  | `#basic-scalar-types`                                 |
| [`int16-global-param.slang`](int16-global-param.slang)                    | expansion  | `#basic-scalar-types`                                 |
| [`int16-vector2-global-param.slang`](int16-vector2-global-param.slang)            | expansion  | `#vec-and-mat`                                        |
| [`int16-array-element.slang`](int16-array-element.slang)                   | expansion  | `#arrays`                                             |
| [`int16-struct-field.slang`](int16-struct-field.slang)                    | expansion  | `#struct-and-class-containers`                        |
| [`int16-rwstructuredbuffer-element.slang`](int16-rwstructuredbuffer-element.slang)      | expansion  | `#resource-and-texture-types`                         |
| [`int16-comparison-lt.slang`](int16-comparison-lt.slang)                   | expansion  | `#basic-scalar-types`                                 |
| [`uint16-global-param.slang`](uint16-global-param.slang)                   | expansion  | `#basic-scalar-types`                                 |
| [`uint16-vector3-global-param.slang`](uint16-vector3-global-param.slang)           | expansion  | `#vec-and-mat`                                        |
| [`uint16-array-element.slang`](uint16-array-element.slang)                  | expansion  | `#arrays`                                             |
| [`uint16-struct-field.slang`](uint16-struct-field.slang)                   | expansion  | `#struct-and-class-containers`                        |
| [`uint16-rwstructuredbuffer-element.slang`](uint16-rwstructuredbuffer-element.slang)     | expansion  | `#resource-and-texture-types`                         |
| [`uint64-to-uint16-narrow.slang`](uint64-to-uint16-narrow.slang)               | expansion  | `#basic-scalar-types`                                 |
| [`int64-vector2-global-param.slang`](int64-vector2-global-param.slang)            | expansion  | `#vec-and-mat`                                        |
| [`int64-array-element.slang`](int64-array-element.slang)                   | expansion  | `#arrays`                                             |
| [`int64-struct-field.slang`](int64-struct-field.slang)                    | expansion  | `#struct-and-class-containers`                        |
| [`int64-rwstructuredbuffer-element.slang`](int64-rwstructuredbuffer-element.slang)      | expansion  | `#resource-and-texture-types`                         |
| [`int64-comparison-lt.slang`](int64-comparison-lt.slang)                   | expansion  | `#basic-scalar-types`                                 |
| [`uint64-vector2-global-param.slang`](uint64-vector2-global-param.slang)           | expansion  | `#vec-and-mat`                                        |
| [`uint64-array-element.slang`](uint64-array-element.slang)                  | expansion  | `#arrays`                                             |
| [`uint64-struct-field.slang`](uint64-struct-field.slang)                   | expansion  | `#struct-and-class-containers`                        |
| [`uint64-rwstructuredbuffer-element.slang`](uint64-rwstructuredbuffer-element.slang)     | expansion  | `#resource-and-texture-types`                         |
| [`half-vector2-global-param.slang`](half-vector2-global-param.slang)             | expansion  | `#vec-and-mat`                                        |
| [`half-vector3-global-param.slang`](half-vector3-global-param.slang)             | expansion  | `#vec-and-mat`                                        |
| [`half-vector4-global-param.slang`](half-vector4-global-param.slang)             | expansion  | `#vec-and-mat`                                        |
| [`half-vector3-arithmetic.slang`](half-vector3-arithmetic.slang)               | expansion  | `#vec-and-mat`                                        |
| [`half2-arithmetic.slang`](half2-arithmetic.slang)                      | expansion  | `#vec-and-mat`                                        |
| [`half-array-element.slang`](half-array-element.slang)                    | expansion  | `#arrays`                                             |
| [`half-struct-field.slang`](half-struct-field.slang)                     | expansion  | `#struct-and-class-containers`                        |
| [`half-rwstructuredbuffer-element.slang`](half-rwstructuredbuffer-element.slang)       | expansion  | `#resource-and-texture-types`                         |
| [`half4-rwstructuredbuffer-element.slang`](half4-rwstructuredbuffer-element.slang)      | expansion  | `#resource-and-texture-types`                         |
| [`half-to-float-widen.slang`](half-to-float-widen.slang)                   | expansion  | `#basic-scalar-types`                                 |
| [`double-to-half-narrow.slang`](double-to-half-narrow.slang)                 | expansion  | `#basic-scalar-types`                                 |
| [`double-vector2-global-param.slang`](double-vector2-global-param.slang)           | expansion  | `#vec-and-mat`                                        |
| [`double-vector3-global-param.slang`](double-vector3-global-param.slang)           | expansion  | `#vec-and-mat`                                        |
| [`double-vector4-global-param.slang`](double-vector4-global-param.slang)           | expansion  | `#vec-and-mat`                                        |
| [`double-vector3-arithmetic.slang`](double-vector3-arithmetic.slang)             | expansion  | `#vec-and-mat`                                        |
| [`double-array-element.slang`](double-array-element.slang)                  | expansion  | `#arrays`                                             |
| [`double-struct-field.slang`](double-struct-field.slang)                   | expansion  | `#struct-and-class-containers`                        |
| [`double-rwstructuredbuffer-element.slang`](double-rwstructuredbuffer-element.slang)     | expansion  | `#resource-and-texture-types`                         |
| [`double4-narrow-to-float4.slang`](double4-narrow-to-float4.slang)              | expansion  | `#vec-and-mat`                                        |
| [`double4-rwstructuredbuffer-element.slang`](double4-rwstructuredbuffer-element.slang)    | expansion  | `#resource-and-texture-types`                         |

## Doc gaps observed

- The doc's "Basic scalar types" table lists `IntPtr` and `UIntPtr`
  ("Signed/Unsigned integer with pointer-equivalent width") but does
  not name a Slang surface construct that produces them. The
  user-level type names (if any — typically host-side only) are not
  reachable from a shader, so a `.slang` test cannot anchor here.
- "Storage-only floating-point" (`FloatE4M3Type`, `FloatE5M2Type`,
  `BFloat16Type`) lists three opcodes but does not state a portable
  surface construct that reliably produces them on the no-GPU runner.
  A user-level surface example (`_E4M3 x;`, `BFloat16 y;`) per row
  would let the agent anchor a test.
- "Differentiation types" (`DiffPair`, `DiffRefPair`,
  `ForwardDiffFuncType`, `BackwardDiffFuncType`, and the seven
  context-channel types) are produced by the autodiff pass when
  `[Differentiable]` is in play. The catalog rows do not name the
  user-level construct that triggers each context-type variant
  (`Minimal` vs ordinary vs `Trivial`); a one-line "surface" column
  would let the agent test them.
- "Existentials and interfaces" lists `BindExistentials`,
  `BoundInterface`, `AnyValueType`, `DynamicType`, and `rtti_type`
  / `rtti_handle_type` as opcodes but tags most as "(synthesized)" —
  they are produced by the existential-elimination IR pass, not by
  lowering. The doc does not state which `-dump-ir` stage to inspect
  to observe them; without that hint the doc reader cannot anchor a
  test in this bundle. Coverage belongs to
  `ir-reference/generics-and-existentials`.
- "Pointer types" lists `RefParam`, `BorrowInParam`,
  `BorrowInOutParam`, `OutParam`, `ComPtr`, `NativePtr`,
  `DescriptorHandle`, and `PseudoPtr`. The doc gives AST origin
  rows (`RefType`, `BorrowInType`, etc.) but does not name a
  user-language keyword for each (Slang's surface for borrow / ref
  parameters is documented elsewhere). A cross-reference to
  `docs/llm-generated/ast-reference/types.md` row by row would let
  the agent anchor tests for each pointer kind.
- "Sampler and buffer-layout types" — `Std140Layout`,
  `Std430Layout`, `D3DConstantBufferLayout`,
  `MetalParameterBlockLayout`, `CUDALayout`, `LLVMLayout`,
  `CLayout`, `ScalarLayout`, `DefaultLayout`,
  `DefaultPushConstantLayout` — the doc names each marker but does
  not state where in the IR they appear (they appear as the third
  operand of a `RWStructuredBuffer(...)` IR type, for example).
  A one-line "where to observe" note would let the agent test each
  marker individually; the current bundle uses one test
  (`rwstructuredbuffer-type.slang`) to cover the family.
- "Set-theoretic types" (`UntaggedUnionType`, `ElementOfSetType`,
  `SetTagType`, `TaggedUnionType`, `OptionalNoneType`) are listed
  with "(synthesized)" AST origin. The doc does not state a
  language surface that triggers each; they appear to be produced
  by the existential-elimination pass. Without a documented
  surface, no `.slang` test in this bundle can anchor here.
- "Tensor and torch-tensor types" — Python-binding lowering only;
  no shader-language surface reaches it.
- "SPIR-V literals and kinds" — the `Type` (TypeKind),
  `TypeParameterPack`, `Rate`, `Generic` kind opcodes are listed
  but their observation requires inspecting the type of a
  generic-parameter's type (`type_t`'s type is `Type`). The doc
  does not name a `-dump-ir` line that displays the kind directly;
  one test (`type-t-generic-param.slang`) covers `type_t` only.
- The doc's `Mat` row says the fourth operand is a "layout int
  literal selecting row-major / column-major" but does not name
  which integer value selects which layout. The current bundle
  uses a wildcard pattern in the matrix test rather than asserting
  a specific value.

## Out of scope (no-GPU runner)

(In this bundle the heading is used for "claims unobservable through
any allowed test directive", consistent with the
`cross-cutting/ir-instructions` and `pipeline/04-ast-to-ir` bundles.)

- **Hoistability flags** (`H` in the doc's tables) — the dump shows
  the post-hoist result, not the decision. Two textually identical
  `Vec(Float, 3 : Int)` types appearing once is observable; the
  *reason* (hoisting) is not.
- The **C++ wrapper struct** identity for each opcode (e.g. that
  `Vec` is a `VectorType` in C++, `Func` is a `FuncType`) —
  internal API, not surface-visible.
- The IR's structural type-equality-by-pointer (the
  `IRInst*`-compare optimisation enabled by hoistable types). This
  is a runtime property of `IRBuilder` deduplication, not visible
  in the dump.
- **Storage-only floats** (`FloatE4M3Type`, `FloatE5M2Type`,
  `BFloat16Type`) — surface support varies by target and core
  module; the doc does not name a portable surface for them.
- **Differentiation context types**
  (`BackwardDiffIntermediateContextType`,
  `TrivialBackwardDiffIntermediateContextType`, the `*Minimal*`
  variants, the legacy-bridge variants, `ForwardDiffFuncType`,
  `BackwardDiffFuncType`, `ApplyForBwdFuncType`,
  `BwdCallableFuncType`, `RematFuncType`) — only produced by the
  autodiff pass with `[Differentiable]` annotations; coverage
  belongs to `ir-reference/differentiation`.
- **Existential / RTTI types** (`BindExistentials`, `BoundInterface`,
  `AnyValueType`, `DynamicType`, `rtti_type`, `rtti_handle_type`,
  `RTTIPointerType`, `RTTIHandleType`) — produced by the
  existential-elimination IR pass; coverage belongs to
  `ir-reference/generics-and-existentials`.
- **Set-theoretic types** (`UntaggedUnionType`, `ElementOfSetType`,
  `SetTagType`, `TaggedUnionType`, `OptionalNoneType`) — also
  pass-produced; not anchored to a surface in the source doc.
- **Tensor and torch-tensor types** (`TensorView`, `TorchTensor`,
  `ArrayListVector`, `TensorAddressingTensorLayoutType`,
  `TensorAddressingTensorViewType`, `MakeTensorAddressingTensor*`)
  — Python-binding lowering, no shader-language surface.
- **Host-side pointer types** (`ComPtr`, `NativePtr`,
  `DescriptorHandle`) — host-side API types, not exercisable from
  a shader.
- **Buffer-layout marker family** as individual opcodes
  (`Std140Layout`, `Std430Layout`, `D3DConstantBufferLayout`,
  `MetalParameterBlockLayout`, `CUDALayout`, `LLVMLayout`,
  `CLayout`, `ScalarLayout`, `DefaultPushConstantLayout`) — they
  appear as operands inside the typed-buffer family IR shapes; the
  doc lists them but does not provide a per-marker surface trigger.
  `DefaultLayout` is covered through the typed-buffer tests; per-
  marker tests are not warranted from the doc claims.
- The **kinds** (`Type`, `TypeParameterPack`, `Rate`, `Generic`)
  beyond `type_t` — the doc lists them as kinds but does not name a
  `-dump-ir` observation that surfaces them directly.
- **GLSL/HLSL geometry-shader streams** (`PointStream`, `LineStream`,
  `TriangleStream`, `Vertices`, `Indices`, `Primitives`,
  `metal::mesh`, `mesh_grid_properties`) — geometry / mesh shader
  stage; this bundle's tests are compute-stage only by convention,
  and the no-GPU runner cannot exercise the geometry/mesh pipeline.
- **HLSL patch types** (`InputPatch`, `OutputPatch`,
  `GLSLInputAttachment`, `SubpassInputType`) — tessellation /
  subpass-input stage; same reason.
- **`Atomic`, `CoopVector`, `CoopMatrix`, `DynamicResource`,
  `RaytracingAccelerationStructure`, `RayQuery`, `HitObject`,
  `TextureFootprintType`** — specialised surfaces; coverage
  belongs to `ir-reference/resources-and-atomics`.
- **String types** (`String`, `NativeString`, `Char`) — `String`
  is observable as `String` in IR for a string literal, but tests
  that use string operations on the no-GPU runner depend on the
  core module exposing the relevant methods; the
  `cross-cutting/ir-instructions` bundle already covers the
  string-literal claim. Per-type-opcode tests for `Char` and
  `NativeString` require host-side or DOS-style surfaces that are
  not portable here.
