// byte-address-buffer-consistency-11591.slang

// Feature test for https://github.com/shader-slang/slang/issues/11545 — slice 2 (#11591).
//
// The access-width lowering decision is driven by the promised `alignment` alone, so a constant
// `location` and a runtime `location` carrying the same `alignment` must lower identically: a
// constant-folded location must not be widened past the promise on the strength of its own offset.
//
// Tested on SPIR-V, where each access is a separate `OpLoad` of its access type, and on HLSL,
// where each access is a separate `Load<T>`.

//TEST:SIMPLE(filecheck=CHECK):-target spirv -emit-spirv-directly -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-HLSL):-target hlsl -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid: SV_DispatchThreadID)
{
    // Promise 8 covers a `float2` (8 bytes) but not the whole 16-byte `float4`, so the access is
    // grouped into two `float2` loads — even though the constant location 16 is itself 16-aligned,
    // it must not be widened to a single `float4` load on the strength of the offset.
    float4 a = buffer.LoadAligned<float4>(16, 8);

    // A runtime location with the same promise must lower identically — two `float2` loads.
    float4 b = buffer.LoadAligned<float4>(tid.x * 16, 8);

    buffer.Store<float4>(0, a + b, 16);
}

// Each promise-8 `float4` load groups into two `float2` loads (four total across the two loads);
// the constant-location load lowers identically to the runtime one, not to a single wide `float4`.
// CHECK-NOT: OpLoad %v4float
// CHECK-COUNT-4: OpLoad %v2float
// CHECK-NOT: OpLoad %v4float
// CHECK-HLSL-NOT: Load<float4 >
// CHECK-HLSL-COUNT-4: Load<float2 >
