//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -target hlsl -entry hullMain -stage hull -entry constants -stage hull -O0 -validate-ir

// `constants` is both an entry point and the patch-constant function for `hullMain`.
// Resource-global legalization can create and initialize a local for its normal entry-point
// invocation. `IRPatchConstantFuncDecoration` causes an additional invocation without an `IRCall`
// argument list that the pass can extend.

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
//CHECK:    ^^^^^^^^^ entry point may be invoked through a path that cannot receive mutable resource state
//CHECK:    ^^^^^^^^^ entry point 'constants' accesses mutable resource state and may be invoked through another path for which Slang cannot add generated resource arguments; move the shared implementation to an ordinary function and pass the resource explicitly
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
