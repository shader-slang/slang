//TEST:SIMPLE(filecheck=POINT):-target spirv-asm -entry pointMain -stage geometry
//TEST:SIMPLE(filecheck=LINE):-target spirv-asm -entry lineMain -stage geometry
//TEST:SIMPLE(filecheck=LINEADJ):-target spirv-asm -entry lineAdjMain -stage geometry
//TEST:SIMPLE(filecheck=TRIADJ):-target spirv-asm -entry triangleAdjMain -stage geometry
//TEST:SIMPLE(filecheck=TRI):-target spirv-asm -entry triangleMain -stage geometry
//TEST:SIMPLE(filecheck=NONEMPTY):-target spirv-asm -entry nonEmptyMain -stage geometry
//TEST:SIMPLE(filecheck=POINTGLSL):-target glsl -entry pointMain -stage geometry
//TEST:SIMPLE(filecheck=LINEGLSL):-target glsl -entry lineMain -stage geometry
//TEST:SIMPLE(filecheck=LINEADJGLSL):-target glsl -entry lineAdjMain -stage geometry
//TEST:SIMPLE(filecheck=TRIADJGLSL):-target glsl -entry triangleAdjMain -stage geometry

// A geometry shader's declared input primitive topology must survive an empty input element
// struct. Regression test for https://github.com/shader-slang/slang/issues/13193.

struct Empty {};
struct Vertex { float4 position : SV_Position; };

// POINT: OpExecutionMode {{.*}} InputPoints
// POINT-NOT: OpExecutionMode {{.*}} Triangles
// POINTGLSL: layout(points) in;
// POINTGLSL-NOT: layout(triangles) in;
[maxvertexcount(1)]
void pointMain(point Empty input[1], inout PointStream<Vertex> output)
{
    Vertex v;
    v.position = float4(0, 0, 0, 1);
    output.Append(v);
}

// LINE: OpExecutionMode {{.*}} InputLines
// LINE-NOT: OpExecutionMode {{.*}} Triangles
// LINEGLSL: layout(lines) in;
// LINEGLSL-NOT: layout(triangles) in;
[maxvertexcount(2)]
void lineMain(line Empty input[2], inout PointStream<Vertex> output)
{
    Vertex v;
    v.position = float4(0, 0, 0, 1);
    output.Append(v);
}

// LINEADJ: OpExecutionMode {{.*}} InputLinesAdjacency
// LINEADJ-NOT: OpExecutionMode {{.*}} Triangles
// LINEADJGLSL: layout(lines_adjacency) in;
// LINEADJGLSL-NOT: layout(triangles) in;
[maxvertexcount(2)]
void lineAdjMain(lineadj Empty input[4], inout PointStream<Vertex> output)
{
    Vertex v;
    v.position = float4(0, 0, 0, 1);
    output.Append(v);
}

// No `-NOT: Triangles` here: the correct `InputTrianglesAdjacency` output contains that substring.
// TRIADJ: OpExecutionMode {{.*}} InputTrianglesAdjacency
// TRIADJGLSL: layout(triangles_adjacency) in;
[maxvertexcount(3)]
void triangleAdjMain(triangleadj Empty input[6], inout PointStream<Vertex> output)
{
    Vertex v;
    v.position = float4(0, 0, 0, 1);
    output.Append(v);
}

// `triangle` is also the fallback topology, so this only checks that explicit triangle input is
// retained.
// TRI: OpExecutionMode {{.*}} Triangles
[maxvertexcount(3)]
void triangleMain(triangle Empty input[3], inout PointStream<Vertex> output)
{
    Vertex v;
    v.position = float4(0, 0, 0, 1);
    output.Append(v);
}

// Non-empty input control: the parameter survives to ir-glsl-legalize.
struct NonEmpty { float4 pos : Position; };

// NONEMPTY: OpExecutionMode {{.*}} InputPoints
// NONEMPTY-NOT: OpExecutionMode {{.*}} Triangles
[maxvertexcount(1)]
void nonEmptyMain(point NonEmpty input[1], inout PointStream<Vertex> output)
{
    Vertex v;
    v.position = float4(0, 0, 0, 1);
    output.Append(v);
}
