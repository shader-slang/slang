//DIAGNOSTIC_TEST:SIMPLE(filecheck=CHECK): -entry main -stage mesh -target spirv

struct msOutput {
    float4 position: SV_Position;
    float3 color: COLOR0;
};

uniform uint iTime;

[outputtopology("triangle")]
[numthreads(1, 1, 1)]
void main(out indices uint3 triangleBox[1], out vertices msOutput triangleVertex[3]) {
    double finalColor;
    uint    uiColor;

    SetMeshOutputCounts(3, 1);
    triangleBox[0] = uint3(0, 1, 1);
    // CHECK: {{.*}}error 57005{{.*}}mesh shader output
    uiColor = triangleBox[0].x + iTime;

    // CHECK: {{.*}}error 57005{{.*}}mesh shader output
    finalColor = triangleVertex[0].position.x;

    triangleVertex[0].position = float4(1, 0.5, 0.0, 1.0);
    triangleVertex[0].color = float3(1, 0.0, 0.0);

    triangleVertex[1].position = float4(1, 0.5, 0.0, 1.0);
    triangleVertex[1].color = float3(1.0, 1.0, 0.0);

    triangleVertex[2].position = float4(1.0, -0.5, 0.0, 1.0);
    triangleVertex[2].color = float3(1.0, 0.0, 1.0);
}
