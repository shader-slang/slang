//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry hullMain -stage hull

Texture2D<float4> texture;
static Texture2D<float4> cachedTexture = texture;

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
//CHECK:    ^^^^^^^^^ resource declared static at file or namespace scope is unavailable to a function whose invocations cannot all be rewritten
//CHECK:    ^^^^^^^^^ function 'constants' may be invoked without a direct call that Slang can rewrite, so Slang cannot automatically pass the resource that it accesses; pass the resource explicitly instead
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
