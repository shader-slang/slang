// byte-address-buffer-float3-widen-11591.slang

// Feature test for https://github.com/shader-slang/slang/issues/11545 — slice 2 (#11591).
//
// A `float3` has a 12-byte natural span but a 16-byte std430 *base* alignment. The wide-vs-
// scalarized lowering now decides against the base alignment, so a `float3` promised 16-byte
// alignment lowers to a single wide load (`.xyz` of a 16-byte access) instead of three scalar
// element loads. A promise below the base alignment (but at/above the 4-byte scalar floor) is
// still valid, but scalarizes and is reported as a suboptimal-access warning (E41304).
//
// Tested on SPIR-V, where byte-address accesses lower to structured access and the alignment
// promise drives codegen. (On HLSL the access stays a native `ByteAddressBuffer.Load3`, so the
// wide-vs-scalarized distinction is not observable.)

//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Promise 16 == `float3`'s base alignment, so this is a single wide load of a `float3`
    // (three floats loaded together), not three separate scalar loads. No warning.
    float3 wide = buffer.LoadAligned<float3>(tid.x * 16, 16);

    // Promise 8 clears the 4-byte scalar floor but is below the 16-byte base alignment, so the
    // access is valid yet scalarized into three 32-bit element loads, with a below-base warning.
    // CHECK-DAG: warning[E41304]
    float3 narrow = buffer.LoadAligned<float3>(tid.x * 16, 8);

    buffer.StoreAligned<float3>(0, wide + narrow);
}

// The promise-16 load is a single 3-component vector load; the promise-8 load scalarizes into
// three. Match the wide vector load explicitly.
// CHECK: OpLoad %v3float
