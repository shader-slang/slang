// shaders.slang

struct BwdTextureContext
{
    RWStructuredBuffer<int> accumulateBuffer;
    Texture2D texture;
    float minLOD;
    void writeDiffToTexel(uint offset, uint layerW, uint layerH, float x, float y, float3 val)
    {
        int4 uval = int4(int3(val * 65536), 1);
        InterlockedAdd(accumulateBuffer[offset + ((uint)y * layerW + (uint)x) * 4 + 0], uval.x);
        InterlockedAdd(accumulateBuffer[offset + ((uint)y * layerW + (uint)x) * 4 + 1], uval.y);
        InterlockedAdd(accumulateBuffer[offset + ((uint)y * layerW + (uint)x) * 4 + 2], uval.z);
        InterlockedAdd(accumulateBuffer[offset + ((uint)y * layerW + (uint)x) * 4 + 3], uval.w);
    }

    void broadcastDiffToLayer(uint lod, float3 diff, float2 uv, uint w, uint h)
    {
        var offset = mipOffset[lod / 4][lod % 4];
        w >>= lod;
        h >>= lod;
        uv = uv - floor(uv);
        float2 loc = uv * float2(w, h) - float2(0.5);
        float x0 = floor(loc.x);
        float y0 = floor(loc.y);
        float fracX = loc.x - x0;
        float fracY = loc.y - y0;
        float x1 = x0 + 1;
        float y1 = y0 + 1;
        if (x0 < 0) x0 += w;
        if (y0 < 0) y0 += h;
        if (x1 >= w) x1 -= w;
        if (y1 >= h) y1 -= h;
        float weight0 = 1.0f - fracY;
        float weight1 = fracY;
        float weight00 = weight0 * (1.0f - fracX);
        float weight01 = weight0 * fracX;
        float weight10 = weight1 * (1.0f - fracX);
        float weight11 = weight1 * fracX;

        writeDiffToTexel(offset, w, h, x0, y0, weight00 * diff);
        writeDiffToTexel(offset, w, h, x1, y0, weight01 * diff);
        writeDiffToTexel(offset, w, h, x0, y1, weight10 * diff);
        writeDiffToTexel(offset, w, h, x1, y1, weight11 * diff);
    }

    void sampleTexture_trilinear_bwd(uint w, uint h, uint levels, float2 uv, float2 dX, float2 dY, float4 dOut)
    {
        dX = dX * float2(w, h);
        dY = dY * float2(w, h);

        // Isotropic filter.
        float lengthX = length(dX);
        float lengthY = length(dY);
        float LOD = log2(max(lengthX, lengthY));
        float maxLOD = levels - 1;
        float clampedLOD = max(minLOD, (min(maxLOD, LOD)));

        float lodFrac = clampedLOD - floor(clampedLOD);
        uint lod0 = (uint)floor(clampedLOD);
        uint lod1 = min(levels - 1, lod0 + 1);
        float weightLod0 = 1.0 - lodFrac;
        float weightLod1 = lodFrac;
        weightLod0 = 1.0;
        broadcastDiffToLayer(lod0, dOut.xyz * weightLod0, uv, w, h);
        broadcastDiffToLayer(lod1, dOut.xyz * weightLod1, uv, w, h);
    }

    float4 sampleTexture_linear(uint lod, float2 uv, uint w, uint h)
    {
        w >>= lod;
        h >>= lod;
        uv = uv - floor(uv);
        float2 loc = uv * float2(w, h) - float2(0.5);
        float x0 = floor(loc.x);
        float y0 = floor(loc.y);
        float fracX = loc.x - x0;
        float fracY = loc.y - y0;
        float x1 = x0 + 1;
        float y1 = y0 + 1;
        if (x0 < 0) x0 += w;
        if (y0 < 0) y0 += h;
        if (x1 >= w) x1 -= w;
        if (y1 >= h) y1 -= h;
        float weight0 = 1.0f - fracY;
        float weight1 = fracY;
        float weight00 = weight0 * (1.0f - fracX);
        float weight01 = weight0 * fracX;
        float weight10 = weight1 * (1.0f - fracX);
        float weight11 = weight1 * fracX;
        return texture.Load(int3(int(x0), int(y0), int(lod)), int2(0)) * weight00 +
               texture.Load(int3(int(x1), int(y0), int(lod)), int2(0)) * weight01 +
               texture.Load(int3(int(x0), int(y1), int(lod)), int2(0)) * weight10 +
               texture.Load(int3(int(x1), int(y1), int(lod)), int2(0)) * weight11;
    }

    float4 sampleTexture_trilinear(uint w, uint h, uint levels, float2 uv, float2 dX, float2 dY)
    {
        dX = dX * float2(w, h);
        dY = dY * float2(w, h);

        // Isotropic filter.
        float lengthX = length(dX);
        float lengthY = length(dY);
        float LOD = log2(max(lengthX, lengthY));
        float maxLOD = levels - 1;
        float clampedLOD = max(minLOD, (min(maxLOD, LOD)));

        float lodFrac = clampedLOD - floor(clampedLOD);
        uint lod0 = (uint)floor(clampedLOD);
        uint lod1 = min(levels - 1, lod0 + 1);
        float weightLod0 = 1.0 - lodFrac;
        float weightLod1 = lodFrac;

        let v0 = sampleTexture_linear(lod0, uv, w, h) * weightLod0;
        let v1 = sampleTexture_linear(lod1, uv, w, h) * weightLod1;
        return v0 + v1;
    }
}

cbuffer Uniforms
{
    float4x4 modelViewProjection;
    uint4 mipOffset[16];

    Texture2D texRef;
    SamplerState sampler;
    BwdTextureContext bwdTexture;
}

struct AssembledVertex
{
    float3	position : POSITION;
};

struct Fragment
{
    float4 color;
};

struct VertexStageOutput
{
    float2 uv : UV;
    float4          sv_position     : SV_Position;
};

float4 sampleAlbedo(no_diff float2 uv)
{
    return bwdTexture.texture.Sample(sampler, uv);
}

[ForwardDerivativeOf(sampleAlbedo)]
DifferentialPair<float4> sampleAlbedo_fwd(float2 uv)
{
    return DifferentialPair<float4>(no_diff bwdTexture.texture.Sample(sampler, uv), 0.0);
}

[BackwardDerivativeOf(sampleAlbedo)]
void sampleAlbedo_bwd(float2 uv, float4 dOut)
{
    float2 dX = ddx_coarse(uv);
    float2 dY = ddy_coarse(uv);
    uint w;
    uint h;
    uint levels;
    bwdTexture.texture.GetDimensions(0, w, h, levels);
    bwdTexture.sampleTexture_trilinear_bwd(w, h, levels, uv, dX, dY, dOut);
}

[BackwardDifferentiable]
float4 shadeFragment(no_diff float2 uv)
{
    float3 color = sampleAlbedo(uv * 2).xyz;
    return float4(color, 1.0);
}

[BackwardDifferentiable]
float3 loss(no_diff float2 uv, no_diff float4 screenPos)
{
    float3 refColor = (no_diff texRef.Load(int3(int2(screenPos.xy), 0))).xyz;
    float3 rs = shadeFragment(uv).xyz - refColor;
    rs *= rs;
    return rs;
}

[shader("vertex")]
VertexStageOutput vertexMain(
    AssembledVertex assembledVertex)
{
    VertexStageOutput output;

    float3 position = assembledVertex.position;

    output.uv = position.xy;
    output.sv_position = mul(modelViewProjection, float4(position, 1.0));

    return output;
}

float3 sqr(float3 v) { return v * v; }

[shader("fragment")]
float4 fragmentMain(
    float2 uv : UV) : SV_Target
{
    return shadeFragment(uv);
}

[shader("fragment")]
float4 diffFragmentMain(
    float2 uv : UV,
    float4 screenPos : SV_POSITION) : SV_Target
{
    __bwd_diff(loss)(uv, screenPos, float3(1.0));
    return float4(loss(uv, screenPos), 1.0);
}
