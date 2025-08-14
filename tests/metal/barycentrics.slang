//TEST:SIMPLE(filecheck=CHECK): -target metal
//TEST:SIMPLE(filecheck=CHECK-ASM): -target metallib

// CHECK: {{\[\[}}barycentric_coord{{\]\]}}
// CHECK-ASM: !"air.barycentric_coord"

[shader("fragment")]
float4 fragment_main(float4 pos: SV_Position, float3 barycentrics: SV_Barycentrics)
    : SV_Target
{
    return float4(barycentrics, 1.f);
}
