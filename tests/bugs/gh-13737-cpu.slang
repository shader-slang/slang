//TEST:SIMPLE(filecheck=CPP): -target cpp -entry computeMain -stage compute
//TEST:SIMPLE(filecheck=SO): -target shader-sharedlib -entry computeMain -stage compute

// shader-slang/slang#13737: on CPU a combined texture-sampler that is also sampled is lowered to a
// struct holding the texture and the sampler. `Load`, the subscript and `GetDimensions` must call
// the texture field, because the struct itself has no such members and the C++ compile fails.

Sampler2D<float4> u_tex;
Sampler2DArray<float4> u_texArray;
RWStructuredBuffer<float4> outBuf;

// CPP: u_texArray_{{[0-9]+}}.texture_{{[0-9]+}}.GetDimensions(
// CPP: u_tex_{{[0-9]+}}.texture_{{[0-9]+}}.Load(
// SO: result code = 0

[shader("compute")]
[numthreads(1, 1, 1)]
void computeMain(uint3 id: SV_DispatchThreadID)
{
    uint w, h, e;
    u_texArray.GetDimensions(w, h, e);
    outBuf[0] = u_tex.Load(int3(int2(id.xy), 0)) + u_tex[id.xy] + u_texArray.Load(int4(int2(id.xy), 1, 0))
        + u_tex.SampleLevel(float2(0.5), 0) + u_texArray.SampleLevel(float3(0.5), 0) + float4(w, h, e, 0);
}
