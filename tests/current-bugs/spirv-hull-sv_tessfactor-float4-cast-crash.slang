//TEST:SIMPLE(filecheck=SPIRV):-target spirv-asm -stage hull -entry hullMain -allow-glsl

/*
Repro for SPIR-V emit crash when using vector types for tessellation-factor builtins.

HLSL requires:
- SV_TessFactor: float[4] for quad (float[3] tri, float[2] line)
- SV_InsideTessFactor: float[2] for quad (float[1] tri)

Slang currently allows authoring `float4`/`float2` here (and docs show this form),
but SPIR-V legalization introduces a BuiltinCast (vector -> array) which the SPIR-V
emitter doesn't handle, leading to an internal error like:

(0): error 99999: Slang compilation aborted due to an exception of ...: unimplemented:
Unhandled local inst in spirv-emit:
let  %1 : %2    = BuiltinCast(%3)
*/

struct PerVertex
{
    float4 position : SV_Position;
};

// SPIRV: OpCapability Tessellation
// SPIRV: OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
// SPIRV: OpDecorate %gl_TessLevelOuter Patch
// SPIRV: OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
// SPIRV: OpDecorate %gl_TessLevelInner Patch

// Hull Shader (HS)
[shader("hull")]
[domain("quad")]
[partitioning("fractional_odd")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("constants")]
PerVertex hullMain(InputPatch<PerVertex, 4> patch, uint i : SV_OutputControlPointID)
{
    return patch[i];
}

struct PatchTessFactors
{
    // This is the problematic case: vector types on tessellation-factor builtins.
    float4 EdgeTessFactor : SV_TessFactor;
    float2 InsideTessFactor : SV_InsideTessFactor;
};

PatchTessFactors constants(InputPatch<PerVertex, 4> patch)
{
    PatchTessFactors o;
    o.EdgeTessFactor = float4(1.0, 2.0, 3.0, 4.0);
    o.InsideTessFactor = float2(0.5, 0.5);
    return o;
}


