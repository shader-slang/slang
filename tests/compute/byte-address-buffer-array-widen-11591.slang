// byte-address-buffer-array-widen-11591.slang

// Regression for https://github.com/shader-slang/slang/issues/11545 — slice 2 (#11591).
//
// The wide-vs-scalarized decision for a composite must use the same std430 stride the equivalent
// structured buffer indexes by, so an access is only taken as one whole-value wide access when its
// offset is an exact multiple of that stride. An array `float4[2]` has base alignment 16 but a
// std430 stride of 32 (two 16-byte-strided elements). A `LoadAligned`/`StoreAligned` that promises
// only 16 (the base alignment) therefore must NOT take a whole-array wide access indexed by 32 —
// that would mis-index a merely-16-aligned location — but instead scalarize into per-element
// `float4` accesses, each wide at the element's own 16-byte stride.
//
// This is a codegen test on the SPIR-V structured-buffer-translation path, where the mis-index
// would manifest; it pins the element-wise `%uint_16` indexing and forbids a whole-array `%uint 32`
// stride constant.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -entry computeMain -stage compute

RWByteAddressBuffer buffer;

struct Pair
{
    float4 v[2];
}

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Runtime, 16-byte-aligned (but not necessarily 32-aligned) location: promise 16 == Pair's base
    // alignment but is below the array's 32-byte stride, so each `float4` element loads/stores wide
    // at its own 16-byte stride rather than the whole array being accessed as one 32-strided value.
    Pair p = buffer.LoadAligned<Pair>(tid.x * 16, 16);
    buffer.StoreAligned<Pair>(tid.x * 16, p);
}

// The equivalent structured buffer holds the 16-byte-strided element type, and every index division
// uses that element stride (16) — never a whole-array stride of 32.
// CHECK-DAG: OpDecorate %_runtimearr_v4float ArrayStride 16
// CHECK: OpUDiv %uint {{.*}} %uint_16
// CHECK-NOT: OpConstant %uint 32
