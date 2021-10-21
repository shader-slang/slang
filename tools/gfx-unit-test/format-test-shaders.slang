// format-test-shaders.slang - Shaders used by the unit tests in format-uint-tests.cpp

// Copy the contents of "tex" into "buffer". These are for textures containing uint data.
[shader("compute")]
[numthreads(4,1,1)]
void copyTexUint4(
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
[numthreads(4,1,1)]
void copyTexUint3(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<uint3> tex,
    uniform RWStructuredBuffer<uint3> buffer)
{
    uint width;
    uint height;
    tex.GetDimensions(width, height);
    buffer[sv_dispatchThreadID.x] = tex[uint2(sv_dispatchThreadID.x % width, sv_dispatchThreadID.x / height)];
}

[shader("compute")]
[numthreads(4,1,1)]
void copyTexUint2(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<uint2> tex,
    uniform RWStructuredBuffer<uint2> buffer)
{
    uint width;
    uint height;
    tex.GetDimensions(width, height);
    buffer[sv_dispatchThreadID.x] = tex[uint2(sv_dispatchThreadID.x % width, sv_dispatchThreadID.x / height)];
}

[shader("compute")]
[numthreads(4,1,1)]
void copyTexUint(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<uint> tex,
    uniform RWStructuredBuffer<uint> buffer)
{
    uint width;
    uint height;
    tex.GetDimensions(width, height);
    buffer[sv_dispatchThreadID.x] = tex[uint2(sv_dispatchThreadID.x % width, sv_dispatchThreadID.x / height)];
}

// Copy the contents of "tex" into "buffer". These are for textures containing floating point data.
[shader("compute")]
[numthreads(4,1,1)]
void copyTexFloat4(
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
void copyTexFloat3(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<float3> tex,
    uniform RWStructuredBuffer<float3> buffer)
{
    uint width;
    uint height;
    tex.GetDimensions(width, height);
    buffer[sv_dispatchThreadID.x] = tex[uint2(sv_dispatchThreadID.x % width, sv_dispatchThreadID.x / height)];
}

[shader("compute")]
[numthreads(4,1,1)]
void copyTexFloat2(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<float2> tex,
    uniform RWStructuredBuffer<float2> buffer)
{
    uint width;
    uint height;
    tex.GetDimensions(width, height);
    buffer[sv_dispatchThreadID.x] = tex[uint2(sv_dispatchThreadID.x % width, sv_dispatchThreadID.x / height)];
}

[shader("compute")]
[numthreads(4,1,1)]
void copyTexFloat(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<float> tex,
    uniform RWStructuredBuffer<float> buffer)
{
    uint width;
    uint height;
    tex.GetDimensions(width, height);
    buffer[sv_dispatchThreadID.x] = tex[uint2(sv_dispatchThreadID.x % width, sv_dispatchThreadID.x / height)];
}

// Sample from "tex" at texture coordinates (0.5, 0.5) and save the results in "buffer".
[shader("compute")]
[numthreads(4,1,1)]
void sampleTex(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<float4> tex,
    uniform SamplerState sampler,
    uniform RWStructuredBuffer<float4> buffer)
{
    buffer[sv_dispatchThreadID.x] = tex.SampleLevel(sampler, float2(0.5, 0.5), 0);
}

// Sample from "tex" at texture coordinates (0.5, 0.5) and save the resuls in "buffer".
// This should only be used with textures containing two mip levels.
[shader("compute")]
[numthreads(2,1,1)]
void sampleMips(
    uint3 sv_dispatchThreadID : SV_DispatchThreadID,
    uniform Texture2D<float4> tex,
    uniform SamplerState sampler,
    uniform RWStructuredBuffer<float4> buffer)
{
    buffer[sv_dispatchThreadID.x] = tex.SampleLevel(sampler, float2(0.5, 0.5), float(sv_dispatchThreadID.x));
}
