// SV_ClipDistance on a vertex or mesh output maps to MSL's [[clip_distance]] attribute, whose type is
// `float` or `float[n]` (MSL spec, Tables 5.4 and 2.11). Slang otherwise spells arrays as
// `metal::array<T, N>`, so an array clip distance is declared as a native array with the attribute
// between the name and the bound, and is written one element at a time because a native array is
// not assignable. The METALLIB runs compile the output with the Metal compiler where one is
// available (macOS).

//TEST:SIMPLE(filecheck=SCALAR): -target metal -stage vertex -entry vsScalar
//TEST:SIMPLE(filecheck=ARRAY): -target metal -stage vertex -entry vsArray
//TEST:SIMPLE(filecheck=OUTARRAY): -target metal -stage vertex -entry vsOutArray
//TEST:SIMPLE(filecheck=MESH): -target metal -stage mesh -entry msArray
//TEST:SIMPLE(filecheck=MESH2): -target metal -entry msArray -stage mesh -entry msArray2 -stage mesh
//TEST:SIMPLE(filecheck=MESHSCALAR): -target metal -stage mesh -entry msScalar
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage vertex -entry vsScalar
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage vertex -entry vsArray
//TEST:SIMPLE(filecheck=METALLIB): -target metallib -stage vertex -entry vsOutArray
//TEST:SIMPLE(filecheck=METALLIB_MESH): -target metallib -stage mesh -entry msArray
//TEST:SIMPLE(filecheck=METALLIB_MESH2): -target metallib -entry msArray -stage mesh -entry msArray2 -stage mesh
//TEST:SIMPLE(filecheck=METALLIB_MESHSCALAR): -target metallib -stage mesh -entry msScalar

// METALLIB: @vs
// METALLIB_MESH: @msArray
// METALLIB_MESH2: @msArray
// METALLIB_MESHSCALAR: @msScalar

struct ScalarOut
{
    float4 pos : SV_Position;
    float clip : SV_ClipDistance0;
};

// SCALAR: float clip{{[_0-9]*}} {{\[\[}}clip_distance{{\]\]}};
// SCALAR-NOT: {{\[\[}}{{\]\]}}
[shader("vertex")]
ScalarOut vsScalar(uint vid : SV_VertexID)
{
    ScalarOut o;
    o.pos = float4(float(vid), 0, 0, 1);
    o.clip = float(vid) - 1.0;
    return o;
}

struct ArrayOut
{
    float4 pos : SV_Position;
    float clip[3] : SV_ClipDistance;
};

// ARRAY: float [[CLIP:clip_[0-9]+]] {{\[\[}}clip_distance{{\]\]}} [3];
// ARRAY-NOT: {{\[\[}}{{\]\]}}
// ARRAY-NOT: ->[[CLIP]] =
// ARRAY: ->[[CLIP]][int(0)] =
// ARRAY: ->[[CLIP]][int(1)] =
// ARRAY: ->[[CLIP]][int(2)] =
// ARRAY-NOT: ->[[CLIP]] =
[shader("vertex")]
ArrayOut vsArray(uint vid : SV_VertexID)
{
    ArrayOut o;
    o.pos = float4(float(vid), 0, 0, 1);
    o.clip[0] = 1.0;
    o.clip[1] = float(vid);
    o.clip[2] = -1.0;
    return o;
}

// An `out` parameter is moved into the result struct, which is built in one aggregate here, so
// this checks the aggregate case as well as the field-store case above.
// OUTARRAY: float [[CLIP:c_[0-9]+]] {{\[\[}}clip_distance{{\]\]}} [2];
// OUTARRAY-NOT: {{\[\[}}{{\]\]}}
// OUTARRAY-NOT: ->[[CLIP]] =
// OUTARRAY: ->[[CLIP]][int(0)] =
// OUTARRAY: ->[[CLIP]][int(1)] =
// OUTARRAY-NOT: ->[[CLIP]] =
[shader("vertex")]
float4 vsOutArray(uint vid : SV_VertexID, out float c[2] : SV_ClipDistance) : SV_Position
{
    c[0] = 1.0;
    c[1] = float(vid);
    return float4(float(vid), 0, 0, 1);
}

struct MeshVertex
{
    float4 position : SV_Position;
    float clip[2] : SV_ClipDistance;
};

// The user's struct keeps its `metal::array` field; the mesh is declared over a separate vertex
// struct that carries the attribute.
// MESH: array<float, int(2)> clip
// MESH: float4 position{{[_0-9]*}} {{\[\[}}position{{\]\]}};
// MESH-NEXT: float [[CLIP:clip_[0-9]+]] {{\[\[}}clip_distance{{\]\]}} [2];
// MESH: metal::mesh<
// MESH-NOT: ->[[CLIP]] =
// MESH: ->[[CLIP]][int(0)] =
// MESH: ->[[CLIP]][int(1)] =
// MESH-NOT: ->[[CLIP]] =
// MESH: set_vertex(
[shader("mesh")]
[numthreads(3, 1, 1)]
[outputtopology("triangle")]
void msArray(
    uint tid : SV_GroupThreadID,
    OutputVertices<MeshVertex, 3> verts,
    OutputIndices<uint3, 1> indices)
{
    SetMeshOutputCounts(3, 1);
    MeshVertex v;
    v.position = float4(float(tid), 0, 0, 1);
    v.clip[0] = 1.0;
    v.clip[1] = -float(tid);
    verts[tid] = v;
    if (tid == 0)
        indices[0] = uint3(0, 1, 2);
}

// A second mesh entry point that shares the vertex struct declares its mesh over the same output
// struct, and its `set_vertex` call receives a value of that struct.
// MESH2: metal::mesh<MeshVertexOutput_0,
// MESH2: thread MeshVertexOutput_0 [[V1:_S[0-9]+]];
// MESH2: set_vertex({{.*}},[[V1]])
// MESH2: metal::mesh<MeshVertexOutput_0,
// MESH2: thread MeshVertexOutput_0 [[V2:_S[0-9]+]];
// MESH2: set_vertex({{.*}},[[V2]])
// MESH2-NOT: MeshVertexOutput_1
[shader("mesh")]
[numthreads(3, 1, 1)]
[outputtopology("triangle")]
void msArray2(
    uint tid : SV_GroupThreadID,
    OutputVertices<MeshVertex, 3> verts,
    OutputIndices<uint3, 1> indices)
{
    SetMeshOutputCounts(3, 1);
    MeshVertex v;
    v.position = float4(0, float(tid), 0, 1);
    v.clip[0] = -1.0;
    v.clip[1] = float(tid);
    verts[tid] = v;
    if (tid == 0)
        indices[0] = uint3(0, 1, 2);
}

struct ScalarMeshVertex
{
    float4 position : SV_Position;
    float clip : SV_ClipDistance0;
};

// A scalar clip distance is copied with an ordinary store.
// MESHSCALAR: float clip{{[_0-9]*}} {{\[\[}}clip_distance{{\]\]}};
// MESHSCALAR: metal::mesh<ScalarMeshVertexOutput_0,
// MESHSCALAR: thread ScalarMeshVertexOutput_0 [[V:_S[0-9]+]];
// MESHSCALAR: ->clip{{[_0-9]*}} =
// MESHSCALAR: set_vertex({{.*}},[[V]])
[shader("mesh")]
[numthreads(3, 1, 1)]
[outputtopology("triangle")]
void msScalar(
    uint tid : SV_GroupThreadID,
    OutputVertices<ScalarMeshVertex, 3> verts,
    OutputIndices<uint3, 1> indices)
{
    SetMeshOutputCounts(3, 1);
    ScalarMeshVertex v;
    v.position = float4(float(tid), 0, 0, 1);
    v.clip = 1.0 - float(tid);
    verts[tid] = v;
    if (tid == 0)
        indices[0] = uint3(0, 1, 2);
}
