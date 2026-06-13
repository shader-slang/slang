// byte-address-buffer-singlearg-float3-11592.slang

// Feature test for https://github.com/shader-slang/slang/issues/11545 — Slice 3 (#11592).
//
// Breaking change: the single-argument `LoadAligned<float3>` forwards
// `__naturalAlignmentOf<float3>()` == 4 (a 3-component vector's natural stride of 12 is not a
// power of two), which only covers a scalar `float`, so it lowers to per-component loads instead
// of a single wide 3-component access.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint loc = tid.x * 16;

    // CHECK-COUNT-3: Load<float >
    float3 v = buffer.LoadAligned<float3>(loc);

    // The single-argument store forwards `__naturalAlignmentOf<float3>()` == 4 the same way, so it
    // also lowers to per-component stores.
    // CHECK-COUNT-3: .Store(
    buffer.StoreAligned<float3>(loc, v);
}
