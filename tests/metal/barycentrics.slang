//TEST:SIMPLE(filecheck=METAL): -target metal
//TEST:SIMPLE(filecheck=METALLIB): -target metallib

// METAL:    {{\[\[}}barycentric_coord{{\]\]}}
// METALLIB: !"air.barycentric_coord"

[shader("fragment")]
float4 fragment_main(float4 pos: SV_Position, float3 barycentrics: SV_Barycentrics)
    : SV_Target
{
    return float4(barycentrics, 1.f);
}
