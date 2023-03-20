//TEST:CROSS_COMPILE: -target spirv-assembly -capability GL_NV_fragment_shader_barycentric -entry main -stage fragment

float4 main(float3 bary : SV_Barycentrics) : SV_Target
{
    return float4(bary, 0);
}
