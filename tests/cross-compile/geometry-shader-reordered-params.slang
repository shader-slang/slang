// geometry-shader-reordered-params.slang

// Regression test for https://github.com/shader-slang/slang/issues/9058
// The input primitive parameter (`triangle ...`, `point ...`, `line ...`,
// `lineadj ...`, `triangleadj ...`) appears *after* the output stream
// parameter. This used to trigger an internal compiler error:
//   "Only expected a single parameter to have
//    IRGeometryInputPrimitiveTypeDecoration decoration"
// because the default `triangle` decoration was applied to the function while
// iterating over the output-stream parameter, before the real input parameter
// was processed.
//
// As well as compiling, the emitted input primitive topology must match the
// real qualifier rather than the `triangle` default, so each case checks the
// emitted GLSL `layout(...) in;` and SPIR-V execution mode, and uses CHECK-NOT
// to assert no *other* input topology leaks in.

//TEST:SIMPLE(filecheck=TRI_GLSL): -profile sm_5_0 -stage geometry -entry mainTriangle -target glsl
//TEST:SIMPLE(filecheck=TRI_SPIRV): -profile sm_5_0 -stage geometry -entry mainTriangle -target spirv
//TEST:SIMPLE(filecheck=PNT_GLSL): -profile sm_5_0 -stage geometry -entry mainPoint -target glsl
//TEST:SIMPLE(filecheck=PNT_SPIRV): -profile sm_5_0 -stage geometry -entry mainPoint -target spirv
//TEST:SIMPLE(filecheck=LINE_GLSL): -profile sm_5_0 -stage geometry -entry mainLine -target glsl
//TEST:SIMPLE(filecheck=LINE_SPIRV): -profile sm_5_0 -stage geometry -entry mainLine -target spirv
//TEST:SIMPLE(filecheck=LINEADJ_GLSL): -profile sm_5_0 -stage geometry -entry mainLineAdj -target glsl
//TEST:SIMPLE(filecheck=LINEADJ_SPIRV): -profile sm_5_0 -stage geometry -entry mainLineAdj -target spirv
//TEST:SIMPLE(filecheck=TRIADJ_GLSL): -profile sm_5_0 -stage geometry -entry mainTriangleAdj -target glsl
//TEST:SIMPLE(filecheck=TRIADJ_SPIRV): -profile sm_5_0 -stage geometry -entry mainTriangleAdj -target spirv

struct CoarseVertex
{
    float4 position : POSITION;
}

struct RasterVertex
{
    float4 position : POSITION;
}

// TRI_GLSL: layout(triangles) in;
// TRI_GLSL-NOT: layout(points) in;
// TRI_GLSL-NOT: layout(lines{{(_adjacency)?}}) in;
// TRI_GLSL-NOT: layout(triangles_adjacency) in;
// TRI_SPIRV: OpExecutionMode %mainTriangle Triangles

[shader("geometry")]
[maxvertexcount(3)]
void mainTriangle(
    inout TriangleStream<RasterVertex> outputStream,
    triangle CoarseVertex coarseVertices[3])
{
    RasterVertex rasterVertex;
    rasterVertex.position = coarseVertices[0].position;
    outputStream.Append(rasterVertex);
}

// PNT_GLSL: layout(points) in;
// PNT_GLSL-NOT: layout(triangles) in;
// PNT_GLSL-NOT: layout(lines{{(_adjacency)?}}) in;
// PNT_SPIRV: OpExecutionMode %mainPoint InputPoints

[shader("geometry")]
[maxvertexcount(1)]
void mainPoint(
    inout PointStream<RasterVertex> outputStream,
    point CoarseVertex coarseVertices[1])
{
    RasterVertex rasterVertex;
    rasterVertex.position = coarseVertices[0].position;
    outputStream.Append(rasterVertex);
}

// LINE_GLSL: layout(lines) in;
// LINE_GLSL-NOT: layout(triangles) in;
// LINE_GLSL-NOT: layout(points) in;
// LINE_GLSL-NOT: layout(lines_adjacency) in;
// LINE_GLSL-NOT: layout(triangles_adjacency) in;
// LINE_SPIRV: OpExecutionMode %mainLine InputLines

[shader("geometry")]
[maxvertexcount(2)]
void mainLine(
    inout LineStream<RasterVertex> outputStream,
    line CoarseVertex coarseVertices[2])
{
    RasterVertex rasterVertex;
    rasterVertex.position = coarseVertices[0].position;
    outputStream.Append(rasterVertex);
}

// LINEADJ_GLSL: layout(lines_adjacency) in;
// LINEADJ_GLSL-NOT: layout(triangles) in;
// LINEADJ_GLSL-NOT: layout(triangles_adjacency) in;
// LINEADJ_GLSL-NOT: layout(points) in;
// LINEADJ_GLSL-NOT: layout(lines) in;
// LINEADJ_SPIRV: OpExecutionMode %mainLineAdj InputLinesAdjacency

[shader("geometry")]
[maxvertexcount(2)]
void mainLineAdj(
    inout LineStream<RasterVertex> outputStream,
    lineadj CoarseVertex coarseVertices[4])
{
    RasterVertex rasterVertex;
    rasterVertex.position = coarseVertices[0].position;
    outputStream.Append(rasterVertex);
}

// TRIADJ_GLSL: layout(triangles_adjacency) in;
// TRIADJ_GLSL-NOT: layout(points) in;
// TRIADJ_GLSL-NOT: layout(lines{{(_adjacency)?}}) in;
// TRIADJ_GLSL-NOT: layout(triangles) in;
// TRIADJ_SPIRV: OpExecutionMode %mainTriangleAdj InputTrianglesAdjacency

[shader("geometry")]
[maxvertexcount(3)]
void mainTriangleAdj(
    inout TriangleStream<RasterVertex> outputStream,
    triangleadj CoarseVertex coarseVertices[6])
{
    RasterVertex rasterVertex;
    rasterVertex.position = coarseVertices[0].position;
    outputStream.Append(rasterVertex);
}
