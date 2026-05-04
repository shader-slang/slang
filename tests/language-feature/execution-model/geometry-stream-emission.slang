// Geometry shader stream output emission across HLSL, GLSL, and SPIRV.
// Verifies that TriangleStream / Append / RestartStrip lower to the
// stage's stream-output ABI.

//TEST:SIMPLE(filecheck=CHECK_HLSL): -target hlsl -stage geometry -entry geometryMain
//TEST:SIMPLE(filecheck=CHECK_GLSL): -target glsl -stage geometry -entry geometryMain
//TEST:SIMPLE(filecheck=CHECK_SPIRV): -target spirv-asm -stage geometry -entry geometryMain -emit-spirv-directly

struct GSIn
{
    float4 pos : SV_Position;
};

struct GSOut
{
    float4 pos : SV_Position;
    float3 color : COLOR;
};

[shader("geometry")]
[maxvertexcount(3)]
void geometryMain(triangle GSIn input[3], inout TriangleStream<GSOut> stream)
{
    [unroll]
    for (int i = 0; i < 3; ++i)
    {
        GSOut o;
        o.pos = input[i].pos;
        o.color = float3(float(i), 0.0, 1.0);
        stream.Append(o);
    }
    stream.RestartStrip();
}

// HLSL keeps the explicit attributes and stream API.
// CHECK_HLSL-DAG: maxvertexcount
// CHECK_HLSL-DAG: TriangleStream
// CHECK_HLSL-DAG: .Append(

// GLSL emits the equivalent layout / EmitVertex / EndPrimitive.
// CHECK_GLSL-DAG: layout(triangles)
// CHECK_GLSL-DAG: layout(triangle_strip
// CHECK_GLSL-DAG: max_vertices = 3
// CHECK_GLSL-DAG: EmitVertex
// CHECK_GLSL-DAG: EndPrimitive

// SPIRV emits ExecutionMode declarations + the OpEmitVertex /
// OpEndPrimitive opcodes.
// CHECK_SPIRV-DAG: ExecutionMode {{.*}} Triangles
// CHECK_SPIRV-DAG: ExecutionMode {{.*}} OutputTriangleStrip
// CHECK_SPIRV-DAG: ExecutionMode {{.*}} OutputVertices 3
// CHECK_SPIRV-DAG: OpEmitVertex
// CHECK_SPIRV-DAG: OpEndPrimitive
