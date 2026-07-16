// byte-address-buffer-float3-widen-index-11591.slang

// Regression for https://github.com/shader-slang/slang/issues/11545 — slice 2 (#11591).
//
// On targets that translate byte-address buffers into structured buffers (GLSL/SPIR-V), the
// equivalent `StructuredBuffer<float3>` runtime array is laid out with the std430 array-element
// stride, which pads a `float3` to 16 bytes (`OpDecorate ... ArrayStride 16`). A base-aligned wide
// `float3` access now takes the whole-value structured-buffer path, so the byte-offset-to-index
// division must divide by that storage stride (16), not the 12-byte natural stride — otherwise a
// runtime location indexes the wrong element (byte offset 48 -> 48/12 = 4 instead of 3).
//
// This is a codegen test rather than a value test because the divisor bug only manifests on the
// structured-buffer-translation path, which the CPU/interpreter backends do not exercise.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Runtime, base-aligned location: the wide access divides the byte offset by the std430 array
    // stride (16), matching the structured buffer's `ArrayStride`, not the 12-byte natural stride.
    float3 v = buffer.LoadAligned<float3>(tid.x * 16, 16);
    buffer.StoreAligned<float3>(tid.x * 16, v + 1.0f);
}

// The equivalent structured buffer's element array is 16-byte-strided, and the index division uses
// that same 16 (`%uint_16`) — never the natural stride 12. If the divisor regressed to 12 a
// `%uint_12` constant would appear and feed the `OpUDiv`.
// CHECK-DAG: OpDecorate %_runtimearr_v3float ArrayStride 16
// CHECK-DAG: %uint_16 = OpConstant %uint 16
// CHECK: OpUDiv %uint {{.*}} %uint_16
// CHECK-NOT: %uint_12 = OpConstant %uint 12
