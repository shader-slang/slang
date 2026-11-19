// The `-target wgsl` row pins the reference std140 layout and runs everywhere. The Tint rows check
// the SPIR-V Tint produces, where `slang-tint` is available, and are ignored elsewhere.
//TEST:SIMPLE(filecheck=WGSL): -target wgsl -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv-asm -entry main -stage compute
//TEST:SIMPLE(filecheck=SPV): -target wgsl-spirv -entry main -stage compute

// Regression test for issues #13391 and #8323.
//
// The Tint targets (`wgsl-spirv` and `wgsl-spirv-asm`) emit WGSL and compile it with Tint, so the
// WGSL they emit must lay out a constant buffer with the same std140 rules as `-target wgsl`, the
// rules reflection reports for all three targets. Here `a` has a 16-byte element stride and `b` is
// at offset 32. `-target wgsl-spirv` must also produce SPIR-V instead of failing with an internal
// error.
//
// tools/slang-unit-test/unit-test-wgsl-spirv-tint-input.cpp checks the WGSL handed to Tint on
// every platform, using a fake `slang-tint`.

cbuffer C
{
    float a[2];
    float b;
}

RWStructuredBuffer<float> o;

// `data_0` is the field of the std140 array wrapper that stands in for `a`.
// WGSL: @align(16) data_0 : array<vec4<f32>, i32(2)>
// WGSL: @align(16) b_0 : f32

// SPV: OpEntryPoint
// SPV-DAG: ArrayStride 16
// SPV-DAG: Offset 32

[numthreads(1, 1, 1)]
void main()
{
    o[0] = a[1] + b;
}
