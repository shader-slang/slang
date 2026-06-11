// byte-address-buffer-chunked.slang

// Feature test for https://github.com/shader-slang/slang/issues/11545 (point 5): when the
// promised `alignment` does not cover the whole vector, the access is emitted as the widest
// power-of-two-width chunks the alignment permits, with per-component accesses for the remainder.
// Also checks the consistency fix (point 3): two single-argument loads at different constant
// locations lower identically.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPV):-target spirv -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint loc = tid.x * 16;

    // Alignment 8 covers a `float2` (8 bytes) but not the whole `float4` (16 bytes), so the load
    // is split into two `float2` chunks.
    // CHECK: Load<float2 >
    // CHECK: Load<float2 >
    float4 a = buffer.LoadAligned<float4>(loc, 8);

    // Alignment 4 only covers a scalar `float`, so the load is fully scalarized.
    // CHECK: Load<float >
    // CHECK: Load<float >
    // CHECK: Load<float >
    // CHECK: Load<float >
    float4 b = buffer.LoadAligned<float4>(loc, 4);

    // Consistency: two single-argument loads at different constant locations both satisfy the
    // natural alignment (16) and lower to the same single wide `float4` load.
    // CHECK: Load<float4 >
    // CHECK: Load<float4 >
    float4 c = buffer.LoadAligned<float4>(0);
    float4 d = buffer.LoadAligned<float4>(16);

    // Store at alignment 8 is split into two `float2` stores.
    // CHECK: Store
    // CHECK: Store
    buffer.Store<float4>(0, a + b + c + d, 8);
}

// CHECK-SPV-DAG: OpLoad %v2float
// CHECK-SPV-DAG: OpLoad %v4float
