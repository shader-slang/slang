// byte-address-buffer-grouped-widths-11593.slang

// Feature test for https://github.com/shader-slang/slang/issues/11593. It pins the width and offset
// of the grouped accesses the legalizer emits: the `half4` (2-byte scalar stride) case, the
// `float3` group-plus-single-component case, the single-argument 3-component form, and plain
// `Load<T>`/`Store<T>` at constant locations. `byte-address-buffer-grouped-values-11593.slang` checks the
// values those accesses produce.
//
// HLSL only: Slang's HLSL emit lowers each access to its own `.Load<T>`, so the widths read
// directly off the output. These cases load overlapping addresses, which the SPIR-V backend
// reuses across cases; SPIR-V group widths are checked in `byte-address-buffer-grouped.slang`.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint loc = tid.x * 16;

    // half4 (2-byte scalar stride) at alignment 4 covers a `half2` (4 bytes) but not the whole
    // `half4` (8 bytes), so the load is two `half2` groups.
    // CHECK-COUNT-2: Load<vector<half,2> >
    half4 ha = buffer.LoadAligned<half4>(loc, 4);

    // half4 at alignment 2 only covers a scalar `half`, so it is fully scalarized.
    // CHECK-COUNT-4: Load<half >
    half4 hb = buffer.LoadAligned<half4>(loc, 2);

    // float3 at alignment 8 covers a `float2` (8 bytes) and leaves a 1-component remainder, so it
    // is one `float2` plus one scalar `float` — the mixed-width group + remainder path.
    // CHECK: Load<float2 >
    // CHECK: Load<float >
    float3 fc = buffer.LoadAligned<float3>(loc, 8);

    // The single-argument `float3` form forwards `__naturalAlignmentOf<float3>()` == 4 (a
    // 3-component vector's natural stride of 12 is not a power of two), which only covers a scalar
    // `float`, so it lowers to three per-component loads.
    // CHECK-COUNT-3: Load<float >
    float3 fd = buffer.LoadAligned<float3>(loc);

    // Plain `Load<T>` has no promise, so a constant location's own alignment decides: 8 covers
    // two `float2` groups, and 4 gives a single `float`, a `float2` at 8 and a single `float`.
    // CHECK: Load<float2 >(8U)
    // CHECK: Load<float2 >(16U)
    float4 pa = buffer.Load<float4>(8);
    // CHECK: Load<float >(4U)
    // CHECK: Load<float2 >(8U)
    // CHECK: Load<float >(16U)
    float4 pb = buffer.Load<float4>(4);

    // A plain `float3` at 12 is a multiple of its 12-byte size, so it stays one access; at 16 it is
    // a `float2` group and a single `float`.
    // CHECK: Load<float3 >(12U)
    // CHECK: Load<float2 >(16U)
    // CHECK: Load<float >(24U)
    float3 pc = buffer.Load<float3>(12);
    float3 pd = buffer.Load<float3>(16);

    // Store at alignment 8 is two `float2` stores (not four scalar stores).
    // CHECK-COUNT-2: Store{{.*}}float2(
    buffer.Store<float4>(0, float4(ha) + float4(hb.x, hb.y, fc.z, fd.x) + pa + pb, 8);

    // Plain `Store<T>` groups the same way as plain `Load<T>`.
    // CHECK: Store(72U,float2(
    // CHECK: Store(80U,float2(
    buffer.Store<float4>(72, float4(pc, pd.x));
    // CHECK: Store(100U,_S
    // CHECK: Store(104U,float2(
    // CHECK: Store(112U,_S
    buffer.Store<float4>(100, float4(pd, pc.x));
}
