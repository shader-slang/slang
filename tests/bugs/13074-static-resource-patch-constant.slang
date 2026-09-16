//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry hullMain -stage hull

Texture2D<float4> texture;
static Texture2D<float4> cachedTexture = texture;
//CHECK:                 ^^^^^^^^^^^^^ resource-dependent static state is not supported in an independently invoked function
//CHECK:                 ^^^^^^^^^^^^^ static variable 'cachedTexture' depends on per-invocation resource state and cannot be used by function 'constants', which may be invoked without an entry-point caller; pass the required state explicitly instead

struct HullOutput
{
    float2 position;
};

struct PatchOutput
{
    float edgeFactors[4] : SV_TessFactor;
    float insideFactors[2] : SV_InsideTessFactor;
    uint value;
};

[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_ccw")]
[outputcontrolpoints(4)]
[patchconstantfunc("constants")]
HullOutput hullMain()
{
    HullOutput result;
    result.position = 1;
    return result;
}

PatchOutput constants()
{
    PatchOutput result;
    result.edgeFactors[0] = 1;
    result.edgeFactors[1] = 2;
    result.edgeFactors[2] = 3;
    result.edgeFactors[3] = 4;
    result.insideFactors[0] = 0.5;
    result.insideFactors[1] = 0.5;
    result.value = uint(cachedTexture.Load(int3(0, 0, 0)).x);
    return result;
}
