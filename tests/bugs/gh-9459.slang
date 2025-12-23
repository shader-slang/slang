//TEST:SIMPLE(filecheck=CHECK): -target spirv -profile spirv_1_4

// CHECK: OpEntryPoint

struct MeshOutput<let hasA : bool, let hasB : bool> {
    float4 position : SV_Position;
    Conditional<uint, hasA> fieldA : FIELD_A;
    Conditional<uint, hasB> fieldB : FIELD_B;
};

typealias Mixed = MeshOutput<true, false>;

[shader("mesh")]
[outputtopology("triangle")]
[numthreads(32, 1, 1)]
void meshMixed(
    uint gtid : SV_GroupThreadID,
    out vertices Mixed verts[64],
    out indices uint3 tris[64]
) {
    SetMeshOutputCounts(3, 1);
    if (gtid < 3) {
        verts[gtid].position = float4(0, 0, 0, 1);
        verts[gtid].fieldA = gtid;
    }
    if (gtid == 0) tris[0] = uint3(0, 1, 2);
}