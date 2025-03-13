//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target spirv
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target glsl
//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -target metal

struct TaskPayload {
    uint offset;
};

struct Output {
    float4 position : SV_POSITION;
};


// CHECK: 50060: Invalid output topology{{.*}}not_valid
[numthreads(32, 1, 1)]
[outputtopology("not_valid")]
[shader("mesh")]
void RenderClusters(
    uint ThreadIndex: SV_GroupIndex,
    uint GroupID: SV_GroupID,
    out vertices Output Vertices[64],
    out indices uint3 Triangles[124],
    in payload TaskPayload Payload
) {
}
