//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage geometry -entry main

// Test for issue #9058: Geometry shader compilation error with GLSL syntax

struct CoarseVertex
{
    float4 position : POSITION;
}

struct RasterVertex
{
    float4 position : POSITION;
}

// Test case: GLSL-style input declaration (should either compile or give a proper error, not internal error)
in triangle CoarseVertex coarseVertices[3];

[maxvertexcount(3)]
void main(
    inout TriangleStream<RasterVertex> outputStream
    )
{
    for(int ii = 0; ii < 3; ++ii)
    {
        RasterVertex rasterVertex;
        rasterVertex.position = coarseVertices[ii].position;
        outputStream.Append(rasterVertex);
    }
}

// CHECK: OpExecutionMode
// CHECK: Triangles