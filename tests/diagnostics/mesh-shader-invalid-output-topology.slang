//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -entry main1 -stage mesh -target hlsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -entry main1 -stage mesh -target spirv
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -entry main1 -stage mesh -target glsl
//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK): -entry main1 -stage mesh -target metal

//DIAGNOSTIC_TEST:SIMPLE(diag=CHECK2): -entry main2 -stage mesh -target hlsl
//TEST:SIMPLE(filecheck=CHECK_OK): -entry main2 -stage mesh -target spirv
//TEST:SIMPLE(filecheck=CHECK_OK): -entry main2 -stage mesh -target glsl
//TEST:SIMPLE(filecheck=CHECK_OK): -entry main2 -stage mesh -target metal


struct TaskPayload {
    uint offset;
};

struct Output {
    float4 position : SV_POSITION;
};


[numthreads(32, 1, 1)]
[outputtopology("asdqwe")]
/*CHECK:
 ^^^^^^^^^^^^^^ invalid mesh output topology
 ^^^^^^^^^^^^^^ Invalid mesh stage output topology
*/
[shader("mesh")]
void main1(
    uint ThreadIndex: SV_GroupIndex,
    uint GroupID: SV_GroupID,
    out vertices Output Vertices[64],
    out indices uint3 Triangles[124],
    in payload TaskPayload Payload
) {
}

// 'point' is not valid for HLSL only, other targets must compile successfully.
// CHECK_OK: main
[numthreads(32, 1, 1)]
[outputtopology("point")]
/*CHECK2:
 ^^^^^^^^^^^^^^ invalid mesh output topology
 ^^^^^^^^^^^^^^ Invalid mesh stage output topology
*/
[shader("mesh")]
void main2(
    uint ThreadIndex: SV_GroupIndex,
    uint GroupID: SV_GroupID,
    out vertices Output Vertices[64],
    out indices uint Points[124],
    in payload TaskPayload Payload
) {
}
