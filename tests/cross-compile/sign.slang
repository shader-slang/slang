// sign.slang

//TEST:CROSS_COMPILE:-target spirv-assembly -entry main -stage fragment
//TEST:CROSS_COMPILE:-target dxil-assembly  -entry main -stage fragment -profile sm_6_0

// Test cross compilation of the sign function

float4 main() : SV_Target
{
    float4 s = sign(float4(1.5, 1.0, -1.5, -1.0));
    return s;
}

