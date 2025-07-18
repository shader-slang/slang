// get-attribute-at-vertex.slang

// Basic test for `GetAttributeAtVertex` function

//TEST:SIMPLE(filecheck=CHECK):-emit-spirv-directly -target spirv -entry main -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric

// CHECK: OpCapability FragmentBarycentricKHR
// CHECK: OpExtension "SPV_KHR_fragment_shader_barycentric"
// CHECK: OpDecorate %{{.*}} BuiltIn BaryCoordKHR
// CHECK: %{{.*}} = OpAccessChain %_ptr_Input_{{.*}} %{{.*}} %uint_0
// CHECK: %{{.*}} = OpAccessChain %_ptr_Input_{{.*}} %{{.*}} %uint_1
// CHECK: %{{.*}} = OpAccessChain %_ptr_Input_{{.*}} %{{.*}} %uint_2

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

// Test for noperspective barycentric coordinates

//TEST:SIMPLE(filecheck=CHECK_NOPERSP):-emit-spirv-directly -target spirv -entry mainNoPerspective -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric

// CHECK_NOPERSP: OpCapability FragmentBarycentricKHR
// CHECK_NOPERSP: OpExtension "SPV_KHR_fragment_shader_barycentric"
// CHECK_NOPERSP: OpDecorate %{{.*}} BuiltIn BaryCoordNoPerspKHR

[shader("fragment")]
void mainNoPerspective(
    pervertex float4 color : COLOR,
    noperspective float3 bary : SV_Barycentrics,
    out float4 result : SV_Target)
{
    result = bary.x * GetAttributeAtVertex(color, 0)
           + bary.y * GetAttributeAtVertex(color, 1)
           + bary.z * GetAttributeAtVertex(color, 2);
}
