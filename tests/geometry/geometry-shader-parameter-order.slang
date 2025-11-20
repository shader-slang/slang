//TEST:SIMPLE(filecheck=CHECK):-target spirv-asm -stage geometry -entry main

// Test for issue #9058: Geometry shader compilation error with reordered input declaration

struct CoarseVertex
{
    float4 position : POSITION;
}

struct RasterVertex
{
    float4 position : POSITION;
}

// Test case: coarseVertices input after outputStream (should compile without internal error)
[maxvertexcount(3)]
void main(
    inout TriangleStream<RasterVertex> outputStream,
    triangle CoarseVertex coarseVertices[3]
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