// On CUDA, a combined Sampler2D has no separate sampler argument, so the tex2Dgather location is
// the call's first argument, with or without an offset (#14073).
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

Sampler2D<float4> stex;
Sampler2DShadow shtex;
uniform float2 uv;
RWStructuredBuffer<float4> outF;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CUDA: [[UV0:[A-Za-z0-9_]+]] = globalParams_0->uv_0;
    // CUDA: tex2Dgather<float4 >((globalParams_0->stex_0), (([[UV0]])).x, (([[UV0]])).y, 0)
    // PTX: tld4.r.2d.v4.f32.f32
    outF[0] = stex.Gather(uv);

    // CUDA: [[UV1:[A-Za-z0-9_]+]] = globalParams_0->uv_0;
    // CUDA: tex2Dgather<float4 >((globalParams_0->stex_0), (([[UV1]])).x, (([[UV1]])).y, 0)
    // PTX: tld4.r.2d.v4.f32.f32
    outF[1] = stex.GatherRed(uv);

    // CUDA: [[UV2:[A-Za-z0-9_]+]] = globalParams_0->uv_0;
    // CUDA: tex2Dgather<float4 >((globalParams_0->stex_0), (([[UV2]])).x, (([[UV2]])).y, 1)
    // PTX: tld4.g.2d.v4.f32.f32
    outF[2] = stex.GatherGreen(uv);

    // CUDA: [[UV3:[A-Za-z0-9_]+]] = globalParams_0->uv_0;
    // CUDA: tex2Dgather<float4 >((globalParams_0->stex_0), (([[UV3]])).x, (([[UV3]])).y, 2)
    // PTX: tld4.b.2d.v4.f32.f32
    outF[3] = stex.GatherBlue(uv);

    // CUDA: [[UV4:[A-Za-z0-9_]+]] = globalParams_0->uv_0;
    // CUDA: tex2Dgather<float4 >((globalParams_0->stex_0), (([[UV4]])).x, (([[UV4]])).y, 3)
    // PTX: tld4.a.2d.v4.f32.f32
    outF[4] = stex.GatherAlpha(uv);

    // CUDA: [[UV5:[A-Za-z0-9_]+]] = globalParams_0->uv_0;
    // CUDA: tex2Dgather<float4 >((globalParams_0->stex_0), (([[UV5]])).x, (([[UV5]])).y
    // PTX: tld4.r.2d.v4.f32.f32
    outF[5] = stex.Gather(uv, int2(3, 7));

    // CUDA: [[UV6:[A-Za-z0-9_]+]] = globalParams_0->uv_0;
    // CUDA: tex2Dgather<float4 >((globalParams_0->stex_0), (([[UV6]])).x, (([[UV6]])).y
    // PTX: tld4.a.2d.v4.f32.f32
    outF[6] = stex.GatherAlpha(uv, int2(3, 7));

    // Only the GatherCmp* variants leave CUDA out; a plain Gather on a shadow texture keeps it.
    // CUDA: [[UV7:[A-Za-z0-9_]+]] = globalParams_0->uv_0;
    // CUDA: tex2Dgather<float4 >((globalParams_0->shtex_0), (([[UV7]])).x, (([[UV7]])).y, 0)
    // PTX: tld4.r.2d.v4.f32.f32
    outF[7] = shtex.Gather(uv);
}
