// byte-address-buffer-grouped.slang

// Feature test for https://github.com/shader-slang/slang/issues/11593: when the promised
// `alignment` does not cover the whole vector, the access is emitted as the widest
// power-of-two-width groups the alignment permits, with per-component accesses for the remainder.
// A promise that covers the whole vector still gives a single access.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=CHECK-SPV):-target spirv -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint loc = tid.x * 16;

    // Alignment 8 covers a `float2` (8 bytes) but not the whole `float4` (16 bytes), so the load
    // is split into two `float2` groups.
    // CHECK: Load<float2 >
    // CHECK: Load<float2 >
    float4 a = buffer.LoadAligned<float4>(loc, 8);

    // Alignment 4 only covers a scalar `float`, so the load is fully scalarized.
    // CHECK: Load<float >
    // CHECK: Load<float >
    // CHECK: Load<float >
    // CHECK: Load<float >
    float4 b = buffer.LoadAligned<float4>(loc, 4);

    // The single-argument form promises the natural alignment (16), which covers the whole
    // `float4`, so each load is a single `float4` access.
    // CHECK: Load<float4 >
    // CHECK: Load<float4 >
    float4 c = buffer.LoadAligned<float4>(0);
    float4 d = buffer.LoadAligned<float4>(16);

    // Store at alignment 8 is split into two `float2` stores (not four scalar stores); matching the
    // `float2(` payload distinguishes the two cases where a bare store-count would not.
    // CHECK-COUNT-2: Store{{.*}}float2(
    buffer.Store<float4>(0, a + b + c + d, 8);

    // Alignment 16 covers a `double2` (16 bytes) but not the whole `double4` (32 bytes), so the
    // 64-bit load and store are each two `double2` groups.
    // CHECK-COUNT-2: Load<vector<double,2> >
    // CHECK-COUNT-2: Store{{.*}}vector<double,2>(
    double4 e = buffer.LoadAligned<double4>(loc + 32, 16);
    buffer.Store<double4>(64, e, 16);
}

// The `float4` at alignment 8 is two `float2` loads, the alignment-16 loads are single `float4`
// loads, the alignment-8 store is two `float2` stores, and the `double4` load at alignment 16 is two
// `double2` loads.
// CHECK-SPV-COUNT-2: OpLoad %v2float
// CHECK-SPV-NOT: OpLoad %v2float
// CHECK-SPV-COUNT-2: OpLoad %v4float
// CHECK-SPV: OpAccessChain %_ptr_StorageBuffer_v2float %buffer %int_0 %uint_0
// CHECK-SPV-NEXT: OpStore
// CHECK-SPV: OpAccessChain %_ptr_StorageBuffer_v2float %buffer %int_0 %uint_1
// CHECK-SPV-NEXT: OpStore
// CHECK-SPV-COUNT-2: OpLoad %v2double
// CHECK-SPV-NOT: OpLoad %v4double
