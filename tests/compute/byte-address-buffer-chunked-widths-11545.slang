// byte-address-buffer-chunked-widths-11545.slang

// Feature test for https://github.com/shader-slang/slang/issues/11545 (point 5). It pins the
// *width* of the chunked accesses the legalizer emits — the documented `half4` (2-byte scalar
// stride) example, the `float3` mixed-width remainder path, and the breaking-change per-component
// lowering of the single-argument 3-component form — which the runtime COMPARE_COMPUTE tests do
// not constrain.
//
// HLSL only: Slang's HLSL emit lowers each chunk to a literal `.Load<T>`, so the widths read
// directly off the output. The SPIRV backend reuses overlapping loads across these same-address
// accesses (it extracts the scalar cases out of the vector loads), which obscures the per-case
// width; SPIRV chunk-width coverage lives in `byte-address-buffer-chunked.slang` (the `float4`
// `OpLoad %v2float` checks) and `byte-address-buffer-aligned.slang` instead.

//TEST:SIMPLE(filecheck=CHECK):-target hlsl -entry computeMain -stage compute

RWByteAddressBuffer buffer;

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 tid : SV_DispatchThreadID)
{
    uint loc = tid.x * 16;

    // half4 (2-byte scalar stride) at alignment 4 covers a `half2` (4 bytes) but not the whole
    // `half4` (8 bytes), so the load is two `half2` chunks.
    // CHECK-COUNT-2: Load<vector<half,2> >
    half4 ha = buffer.LoadAligned<half4>(loc, 4);

    // half4 at alignment 2 only covers a scalar `half`, so it is fully scalarized.
    // CHECK-COUNT-4: Load<half >
    half4 hb = buffer.LoadAligned<half4>(loc, 2);

    // float3 at alignment 8 covers a `float2` (8 bytes) and leaves a 1-component remainder, so it
    // is one `float2` plus one scalar `float` — the mixed-width chunk + remainder path.
    // CHECK: Load<float2 >
    // CHECK: Load<float >
    float3 fc = buffer.LoadAligned<float3>(loc, 8);

    // Breaking change: the single-argument `float3` form forwards `__naturalAlignmentOf<float3>()`
    // == 4 (a 3-component vector's natural stride of 12 is not a power of two), which only covers a
    // scalar `float`, so it lowers to three per-component loads instead of a single wide
    // 3-component access.
    // CHECK-COUNT-3: Load<float >
    float3 fd = buffer.LoadAligned<float3>(loc);

    // Store at alignment 8 is two `float2` stores (not four scalar stores).
    // CHECK-COUNT-2: Store{{.*}}float2(
    buffer.Store<float4>(0, float4(ha) + float4(hb.x, hb.y, fc.z, fd.x), 8);
}
