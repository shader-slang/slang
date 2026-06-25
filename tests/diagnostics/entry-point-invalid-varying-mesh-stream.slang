// Verify the entry-point varying type walker descends through geometry-shader
// stream output wrappers and mesh-shader output wrappers, rather than treating
// them as opaque zero-field structs.

//DIAGNOSTIC_TEST:SIMPLE(diag=STREAM,non-exhaustive):-target spirv -entry geomMain -stage geometry
//DIAGNOSTIC_TEST:SIMPLE(diag=VERTICES,non-exhaustive):-target spirv -entry meshMain -stage mesh

// A struct containing a type banned as a varying, reached through a
// `TriangleStream<...>` parameter.
struct BadStream
{
    float4 pos : SV_Position;
    DifferentialPair<float> dp : A;
}

[shader("geometry")]
[maxvertexcount(3)]
void geomMain(triangle float4 input[3] : SV_Position, inout TriangleStream<BadStream> outStream)
//STREAM: DifferentialPair/DifferentialPtrPair is not a valid varying type
{
    BadStream v = {};
    outStream.Append(v);
}

// And through a mesh-shader `OutputVertices<...>` parameter.
struct BadVertex
{
    float4 pos : SV_Position;
    DifferentialPair<float> dp : B;
}

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void meshMain(OutputVertices<BadVertex, 1> verts, OutputIndices<uint3, 1> tris)
//VERTICES: DifferentialPair/DifferentialPtrPair is not a valid varying type
{
    SetMeshOutputCounts(1, 1);
}
