//TEST:SIMPLE: -profile ps_4_0 -entry main -target hlsl
//TEST:SIMPLE: -profile ps_4_0 -entry main -target glsl

[earlydepthstencil]
float4 main(): SV_Target
{
    return float4(1, 0, 0, 1); 
}

