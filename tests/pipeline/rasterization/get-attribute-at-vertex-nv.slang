// get-attribute-at-vertex.slang

// Basic test for `GetAttributeAtVertex` function

//TEST:CROSS_COMPILE:-target dxil -capability GL_NV_fragment_shader_barycentric -entry main -stage fragment -profile sm_6_1
//TEST:CROSS_COMPILE:-target spirv -capability GL_NV_fragment_shader_barycentric -entry main -stage fragment -profile glsl_450

[shader("fragment")]
void main(
    pervertex float4 color : COLOR,
    float3 bary : SV_Barycentrics,
    out float4 result : SV_Target)
{
    result = bary.x * GetAttributeAtVertex(color, 0)
           + bary.y * GetAttributeAtVertex(color, 1)
           + bary.z * GetAttributeAtVertex(color, 2);
}
