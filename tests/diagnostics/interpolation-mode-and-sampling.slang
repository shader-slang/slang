// An interpolation mode (linear, noperspective, nointerpolation) combines with a sampling location
// (centroid, sample), as in HLSL (`noperspective centroid` is accepted by DXC). Two sampling
// locations still conflict: interpolation-sampling-conflict.slang.
//TEST:SIMPLE(filecheck=HLSL): -target hlsl -stage fragment -entry fsMain
//TEST:SIMPLE(filecheck=SPV): -target spirv-asm -stage fragment -entry fsMain
//TEST:SIMPLE(filecheck=GLSL): -target glsl -stage fragment -entry fsMain

struct PSIn
{
    float4 pos : SV_Position;
    noperspective centroid float2 uv : TEXCOORD0;
    noperspective sample float w : TEXCOORD1;
    linear centroid float z : TEXCOORD2;
};

// HLSL-DAG: centroid noperspective float2 uv
// HLSL-DAG: sample noperspective float w
// HLSL-DAG: centroid linear float z
// SPV-DAG: OpDecorate %{{.*}}uv NoPerspective
// SPV-DAG: OpDecorate %{{.*}}uv Centroid
// SPV-DAG: OpDecorate %{{.*}}w NoPerspective
// SPV-DAG: OpDecorate %{{.*}}w Sample
// SPV-DAG: OpDecorate %{{.*}}z Centroid
// GLSL-DAG: noperspective centroid layout(location = 0)
// GLSL-DAG: noperspective sample layout(location = 1)
float4 fsMain(PSIn i) : SV_Target
{
    return float4(i.uv, i.w, i.z);
}
