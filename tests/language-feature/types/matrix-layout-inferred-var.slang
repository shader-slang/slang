// A matrix layout written before a declaration whose type is inferred from its initializer
// applies to the inferred matrix type. No type expression carries the modifier here, so
// `maybeApplyLayoutModifier` applies it.

//TEST:REFLECTION(filecheck=CHECK):-target hlsl -profile sm_6_0 -no-codegen -matrix-layout-column-major

// A row-major `float2x3` is 28 bytes; the column-major default would make it 40.
struct InferredFields
{
    row_major var inferred = float2x3(1, 2, 3, 4, 5, 6);
    var trailing: float;
}

// CHECK: "name": "inferred"
// CHECK: "offset": 0, "size": 28
// CHECK: "name": "trailing"
// CHECK: "offset": 28
ConstantBuffer<InferredFields> fields;
