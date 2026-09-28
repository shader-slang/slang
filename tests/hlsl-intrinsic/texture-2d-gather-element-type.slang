// Regression for #12276: the CUDA gather template argument must be the
// four-component result type, else NVRTC rejects a scalar/2-/3-component
// tex2Dgather result assigned into the vector<T,4> the signature returns.
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=PTX): -target ptx -entry computeMain -stage compute

Texture2D<float> texF1;
Texture2D<float2> texF2;
Texture2D<float3> texF3;
Texture2D<int> texI1;
Texture2D<uint> texU1;
SamplerState samp;

RWStructuredBuffer<float4> outF;
RWStructuredBuffer<int4> outI;
RWStructuredBuffer<uint4> outU;

[numthreads(1, 1, 1)]
void computeMain()
{
    // CUDA: tex2Dgather<float4
    // PTX: tld4.r.2d.v4.f32
    outF[0] = texF1.Gather(samp, float2(0.5, 0.5));

    // CUDA: tex2Dgather<float4
    // PTX: tld4.r.2d.v4.f32
    outF[1] = texF2.GatherRed(samp, float2(0.5, 0.5));

    // CUDA: tex2Dgather<float4
    // PTX: tld4.g.2d.v4.f32
    outF[2] = texF3.GatherGreen(samp, float2(0.5, 0.5));

    // CUDA: tex2Dgather<int4
    // PTX: tld4.r.2d.v4.s32
    outI[0] = texI1.Gather(samp, float2(0.5, 0.5));

    // CUDA: tex2Dgather<uint4
    // PTX: tld4.r.2d.v4.u32
    outU[0] = texU1.Gather(samp, float2(0.5, 0.5));

    // Offset overload: a distinct CUDA intrinsic (CUDA ignores the offset), so
    // it needs its own coverage for the four-component result type.
    // CUDA: tex2Dgather<float4
    // PTX: tld4.r.2d.v4.f32
    outF[3] = texF1.Gather(samp, float2(0.5, 0.5), int2(1, 1));
}
