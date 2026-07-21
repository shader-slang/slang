// A GLSL-style global-scope geometry-shader input declared as an array of
// structs (`in triangle CoarseVertex coarseVertices[3];`) must legalize and
// compile for both SPIR-V and GLSL.
// See https://github.com/shader-slang/slang/issues/12169

//TEST:SIMPLE(filecheck=SPIRV): -profile sm_5_0 -stage geometry -entry main -target spirv
//TEST:SIMPLE(filecheck=GLSL): -profile sm_5_0 -stage geometry -entry main -target glsl

struct CoarseVertex
{
    float4 position : POSITION;
}

struct RasterVertex
{
    float4 position : POSITION;
}

in triangle CoarseVertex coarseVertices[3];

// SPIRV: OpExecutionMode %main Triangles
// GLSL: layout(triangles) in;

[shader("geometry")]
[maxvertexcount(3)]
void main(inout TriangleStream<RasterVertex> outputStream)
{
    for (int ii = 0; ii < 3; ++ii)
    {
        RasterVertex v;
        v.position = coarseVertices[ii].position;
        outputStream.Append(v);
    }
}
