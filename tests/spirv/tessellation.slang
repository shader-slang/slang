//TEST:SIMPLE(filecheck=CHECK): -target spirv

// CHECK-DAG: OpExecutionMode %main SpacingEqual

// CHECK-DAG: OpExecutionMode %main OutputVertices 4

// CHECK-DAG: OpExecutionMode %main VertexOrderCw

// CHECK-DAG: OpExecutionMode %main Quads

// CHECK: OpDecorate %gl_TessLevelOuter BuiltIn TessLevelOuter
// CHECK: OpDecorate %gl_TessLevelOuter Patch
// CHECK: OpDecorate %gl_TessLevelInner BuiltIn TessLevelInner
// CHECK: OpDecorate %gl_TessLevelInner Patch

// CHECK: OpControlBarrier %uint_2 %uint_4 %uint_0

// CHECK: OpStore %gl_TessLevelOuter
// CHECK: OpStore %gl_TessLevelInner

struct VS_OUT
{
    float3 position : POSITION;
};

struct HS_OUT
{
    float3 position : POSITION;
};

struct HSC_OUT
{
    float EdgeTessFactor[4] : SV_TessFactor;
    float InsideTessFactor[2] : SV_InsideTessFactor;
};

// Hull Shader (HS)
[domain("quad")]
[partitioning("integer")]
[outputtopology("triangle_cw")]
[outputcontrolpoints(4)]
[patchconstantfunc("constants")]
HS_OUT main(InputPatch<VS_OUT, 4> patch, uint i : SV_OutputControlPointID)
{
    HS_OUT o;
    o.position = patch[i].position;
    return o;
}

HSC_OUT constants(InputPatch<VS_OUT, 4> patch)
{
    float3 p0 = patch[0].position;
    float3 p1 = patch[1].position;
    float3 p2 = patch[2].position;
    float3 p3 = patch[3].position;

    HSC_OUT o;
    o.EdgeTessFactor[0] = dot(p0, p1); 
    o.EdgeTessFactor[1] = dot(p0, p3);
    o.EdgeTessFactor[2] = dot(p2, p3);
    o.EdgeTessFactor[3] = dot(p1, p2);
    o.InsideTessFactor[0] = lerp(o.EdgeTessFactor[1], o.EdgeTessFactor[3], 0.5);
    o.InsideTessFactor[1] = lerp(o.EdgeTessFactor[0], o.EdgeTessFactor[2], 0.5);
    return o;
}