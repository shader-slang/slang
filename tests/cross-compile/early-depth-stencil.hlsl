//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage fragment
//TEST:CROSS_COMPILE:-profile ps_6_0 -target dxil-assembly -entry main 

[earlydepthstencil]
float4 main(): SV_Target
{
    return float4(1, 0, 0, 1); 
}

