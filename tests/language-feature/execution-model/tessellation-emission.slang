// Tessellation hull + domain shader emission.
//
// A hull shader has two functions:
//   - the patch-constant function, which produces tessellation factors
//     (`SV_TessFactor`, `SV_InsideTessFactor`)
//   - the per-control-point function decorated with the partitioning,
//     output topology, output control point count, and patch-constant
//     function.
//
// A domain shader consumes the tessellated `SV_DomainLocation`.
//
// This test pins HLSL, GLSL, and SPIRV emission of the full pipeline.

//TEST:SIMPLE(filecheck=CHECK_HULL_HLSL): -target hlsl -stage hull -entry hullMain
//TEST:SIMPLE(filecheck=CHECK_HULL_GLSL): -target glsl -stage hull -entry hullMain
//TEST:SIMPLE(filecheck=CHECK_HULL_SPV): -target spirv-asm -stage hull -entry hullMain -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK_DOMAIN_HLSL): -target hlsl -stage domain -entry domainMain
//TEST:SIMPLE(filecheck=CHECK_DOMAIN_GLSL): -target glsl -stage domain -entry domainMain
//TEST:SIMPLE(filecheck=CHECK_DOMAIN_SPV): -target spirv-asm -stage domain -entry domainMain -emit-spirv-directly

struct HSPerVertex
{
    float3 pos : POSITION;
};

struct HSPatchConstant
{
    float edges[3] : SV_TessFactor;
    float inside : SV_InsideTessFactor;
};

HSPatchConstant patchConstantFunc(InputPatch<HSPerVertex, 3> patch)
{
    HSPatchConstant o;
    o.edges[0] = 1.0;
    o.edges[1] = 1.0;
    o.edges[2] = 1.0;
    o.inside = 1.0;
    return o;
}

[shader("hull")]
[domain("tri")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(3)]
[patchconstantfunc("patchConstantFunc")]
HSPerVertex hullMain(InputPatch<HSPerVertex, 3> patch, uint cpId : SV_OutputControlPointID)
{
    return patch[cpId];
}

[shader("domain")]
[domain("tri")]
float4 domainMain(
    HSPatchConstant pc,
    OutputPatch<HSPerVertex, 3> patch,
    float3 bary : SV_DomainLocation) : SV_Position
{
    float3 p = patch[0].pos * bary.x + patch[1].pos * bary.y + patch[2].pos * bary.z;
    return float4(p, 1.0);
}

// Hull / HLSL — Slang appends a uniqueness suffix to the patch-
// constant function name; match the prefix.
// CHECK_HULL_HLSL-DAG: domain("tri")
// CHECK_HULL_HLSL-DAG: partitioning("integer")
// CHECK_HULL_HLSL-DAG: outputtopology("triangle_cw")
// CHECK_HULL_HLSL-DAG: outputcontrolpoints(3)
// CHECK_HULL_HLSL-DAG: patchconstantfunc("patchConstantFunc
// CHECK_HULL_HLSL-DAG: SV_TessFactor
// CHECK_HULL_HLSL-DAG: SV_InsideTessFactor

// Hull / GLSL — the Slang GLSL emitter writes tess factors via
// gl_TessLevelOuter / gl_TessLevelInner from the patch-constant
// function rather than declaring an explicit `layout(vertices = N)`.
// CHECK_HULL_GLSL-DAG: gl_TessLevelOuter
// CHECK_HULL_GLSL-DAG: gl_TessLevelInner

// Hull / SPIRV.
// CHECK_HULL_SPV-DAG: ExecutionMode {{.*}} OutputVertices 3
// CHECK_HULL_SPV-DAG: ExecutionMode {{.*}} Triangles
// CHECK_HULL_SPV-DAG: BuiltIn TessLevelOuter
// CHECK_HULL_SPV-DAG: BuiltIn TessLevelInner

// Domain / HLSL.
// CHECK_DOMAIN_HLSL-DAG: domain("tri")
// CHECK_DOMAIN_HLSL-DAG: SV_DomainLocation

// Domain / GLSL — Slang uses gl_TessCoord for SV_DomainLocation.
// CHECK_DOMAIN_GLSL-DAG: gl_TessCoord

// Domain / SPIRV.
// CHECK_DOMAIN_SPV-DAG: ExecutionMode {{.*}} Triangles
// CHECK_DOMAIN_SPV-DAG: BuiltIn TessCoord
