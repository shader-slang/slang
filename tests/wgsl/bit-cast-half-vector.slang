//TEST:SIMPLE(filecheck=CHECK): -target wgsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=TINT): -target wgsl-spirv-asm -entry computeMain -stage compute

// WGSL's `bitcast` converts directly between `vec2<f16>` and a 32-bit scalar (`u32`, `i32`,
// `f32`), and between `vec4<f16>` and a `vec2` of 32-bit values. `bit_cast` of those shapes is
// emitted as a WGSL `bitcast` in both directions, not lowered through 16-bit integers, which
// WGSL does not have.

// CHECK-NOT: {{error\[E[0-9]+\]}}
// CHECK-DAG: bitcast<vec2<f16>>(
// CHECK-DAG: bitcast<vec4<f16>>(
// CHECK-DAG: bitcast<u32>(
// CHECK-DAG: bitcast<i32>(
// CHECK-DAG: bitcast<f32>(
// CHECK-DAG: bitcast<vec2<u32>>(
// CHECK-NOT: {{error\[E[0-9]+\]}}
// TINT: OpEntryPoint

RWStructuredBuffer<uint4> words;
RWStructuredBuffer<half4> halves;

[numthreads(1, 1, 1)]
void computeMain(uint3 dispatchThreadID: SV_DispatchThreadID)
{
    const uint i = dispatchThreadID.x;
    const uint4 w = words[i];
    const half4 h = halves[i];

    // 32-bit scalar -> half2 and uint2 -> half4.
    const half2 fromUInt = bit_cast<half2>(w.x);
    const half2 fromInt = bit_cast<half2>(int(w.y));
    const half2 fromFloat = bit_cast<half2>(asfloat(w.z));
    const half4 fromUInt2 = bit_cast<half4>(w.zw);
    halves[i] = half4(fromUInt + fromInt, fromFloat) + fromUInt2;

    // half2 -> 32-bit scalar and half4 -> uint2.
    words[i] = uint4(
        bit_cast<uint>(h.xy),
        uint(bit_cast<int>(h.zw)),
        asuint(bit_cast<float>(h.xw)),
        bit_cast<uint2>(h).x);
}
