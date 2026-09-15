// Writing `tris[pbase + 1u]` must scale the whole index, `(pbase + 1U)*3`, not `pbase + 1U*3`.
// A scalar `out indices uint` (point topology) has one index per primitive and no component.

//TEST:SIMPLE(filecheck=TRI): -entry meshTriangles -stage mesh -target metal
//TEST:SIMPLE(filecheck=LINE): -entry meshLines -stage mesh -target metal
//TEST:SIMPLE(filecheck=POINT): -entry meshPoints -stage mesh -target metal
//TEST:SIMPLE(filecheck=VALUE): -entry meshVectorValue -stage mesh -target metal
//TEST:SIMPLE(filecheck=METALLIB): -entry meshTriangles -stage mesh -target metallib
//TEST:SIMPLE(filecheck=METALLIB): -entry meshLines -stage mesh -target metallib
//TEST:SIMPLE(filecheck=METALLIB): -entry meshPoints -stage mesh -target metallib
//TEST:SIMPLE(filecheck=METALLIB): -entry meshVectorValue -stage mesh -target metallib

// METALLIB: result code = 0

struct V { float4 pos : SV_Position; };

[shader("mesh")]
[numthreads(32, 1, 1)]
[outputtopology("triangle")]
void meshTriangles(uint lane : SV_GroupIndex, out vertices V verts[128], out indices uint3 tris[64])
{
    SetMeshOutputCounts(128, 64);
    uint vbase = lane * 4u;
    uint pbase = lane * 2u;
    for (uint i = 0u; i < 4u; ++i)
    {
        V v;
        v.pos = float4(float(i), 0.0, 0.0, 1.0);
        verts[vbase + i] = v;
    }

    // TRI: _slang_mesh.set_index(pbase{{.*}}*3+0,
    // TRI: _slang_mesh.set_index(pbase{{.*}}*3+1,
    // TRI: _slang_mesh.set_index(pbase{{.*}}*3+2,
    tris[pbase] = uint3(vbase, vbase + 1u, vbase + 2u);

    // TRI: _slang_mesh.set_index((pbase{{.*}} + 1U)*3+0,
    // TRI: _slang_mesh.set_index((pbase{{.*}} + 1U)*3+1,
    // TRI: _slang_mesh.set_index((pbase{{.*}} + 1U)*3+2,
    tris[pbase + 1u] = uint3(vbase, vbase + 2u, vbase + 3u);
}

[shader("mesh")]
[numthreads(32, 1, 1)]
[outputtopology("line")]
void meshLines(uint lane : SV_GroupIndex, out vertices V verts[128], out indices uint2 lines[64])
{
    SetMeshOutputCounts(128, 64);
    uint vbase = lane * 2u;
    for (uint i = 0u; i < 2u; ++i)
    {
        V v;
        v.pos = float4(float(i), 0.0, 0.0, 1.0);
        verts[vbase + i] = v;
    }

    // LINE: _slang_mesh.set_index(({{.*}} + 1U)*2+0,
    // LINE: _slang_mesh.set_index(({{.*}} + 1U)*2+1,
    lines[lane + 1u] = uint2(vbase, vbase + 1u);
}

[shader("mesh")]
[numthreads(32, 1, 1)]
[outputtopology("point")]
void meshPoints(uint lane : SV_GroupIndex, out vertices V verts[128], out indices uint points[64])
{
    SetMeshOutputCounts(128, 64);
    V v;
    v.pos = float4(float(lane), 0.0, 0.0, 1.0);
    verts[lane] = v;

    // POINT: _slang_mesh.set_index(({{.*}} + 1U)*1+0,{{[a-z_0-9]+}});
    points[lane + 1u] = lane;
}

[shader("mesh")]
[numthreads(32, 1, 1)]
[outputtopology("triangle")]
void meshVectorValue(
    uint lane : SV_GroupIndex,
    uniform uint3 offset,
    out vertices V verts[128],
    out indices uint3 tris[64])
{
    SetMeshOutputCounts(128, 64);
    V v;
    v.pos = float4(float(lane), 0.0, 0.0, 1.0);
    verts[lane] = v;

    // A value below postfix precedence is parenthesized before it is subscripted.
    // VALUE: _slang_mesh.set_index({{.*}}*3+0,({{.*}} + {{.*}})[0]);
    // VALUE: _slang_mesh.set_index({{.*}}*3+1,({{.*}} + {{.*}})[1]);
    // VALUE: _slang_mesh.set_index({{.*}}*3+2,({{.*}} + {{.*}})[2]);
    tris[lane] = uint3(lane, lane + 1u, lane + 2u) + offset;
}
