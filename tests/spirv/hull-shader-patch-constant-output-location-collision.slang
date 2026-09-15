//TEST:SIMPLE(filecheck=SPIRV):-target spirv -entry main -stage hull -allow-glsl

// Per-vertex outputs:
//SPIRV: OpDecorate %entryPointParam_main_pos Location 0

// Patch constant func output with user-semantics, should NOT be Location 0,
// and each user semantic should get its own distinct location:
//SPIRV: OpDecorate %entryPointParam_constants_center Location 2
//SPIRV: OpDecorate %entryPointParam_constants_center Patch
//SPIRV: OpDecorate %entryPointParam_constants_tangent Location 3
//SPIRV: OpDecorate %entryPointParam_constants_tangent Patch

struct VOut { float3 pos : POSITION; }
struct HsOut
{
  float3 pos : POSITION0;
  float2 uv : TEXCOORD0;
}
struct HscOut
{
    float edges[3] : SV_TessFactor;
    float inner : SV_InsideTessFactor;
    float3 center : CENTER;
    float3 tangent : TANGENT;
}

HscOut constants(InputPatch<VOut, 3> ip, OutputPatch<HsOut, 3> op)
{
    HscOut o;
    o.edges[0] = op[0].pos.x;
    o.edges[1] = 2;
    o.edges[2] = 3;
    o.inner = ip[0].pos.y;
    o.center = float3(42, 99, 7);
    o.tangent = float3(1, 0, 0);
    return o;
}

[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("constants")]
[shader("hull")]
HsOut main(InputPatch<VOut, 3> ip, uint cpid : SV_OutputControlPointID)
{
    HsOut o;
    o.pos = ip[cpid].pos;
    o.uv = float2(0.0, 1.0);
    return o;
}
