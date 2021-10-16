// format-test-shaders.slang

// Copy the contents of "tex" into "buffer".
[shader("compute")]
[numthreads(16,1,1)]
void copyTexUint(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<uint4> tex,
    uniform RWStructuredBuffer<uint4> buffer)
{
    uint width;
    uint height;
    tex.GetDimensions(width, height);
    buffer[sv_dispatchThreadID.x] = tex[uint2(sv_dispatchThreadID.x % width, sv_dispatchThreadID.x / height)];
}

[shader("compute")]
[numthreads(16,1,1)]
void copyTexFloat(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<float4> tex,
    uniform RWStructuredBuffer<float4> buffer)
{
    uint width;
    uint height;
    tex.GetDimensions(width, height);
    buffer[sv_dispatchThreadID.x] = tex[uint2(sv_dispatchThreadID.x % width, sv_dispatchThreadID.x / height)];
}

[shader("compute")]
[numthreads(4,1,1)]
void sampleTexFloat(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<float4> tex,
    uniform SamplerState sampler,
    uniform RWStructuredBuffer<float4> buffer)
{
    buffer[sv_dispatchThreadID.x] = tex.SampleLevel(sampler, float2(0.5, 0.5), 0);
}

// Sample from "tex" and save the resuls in "buffer". This sampler samples from a specific location
// and only varies by mipmap layer.
[shader("compute")]
[numthreads(2,1,1)]
void sampleMipsFloat(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<float4> tex,
    uniform SamplerState sampler,
    uniform RWStructuredBuffer<float4> buffer)
{
    buffer[sv_dispatchThreadID.x] = tex.SampleLevel(sampler, float2(0.5, 0.5), float(sv_dispatchThreadID.x));
}
