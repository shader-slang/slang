// get-attribute-at-vertex.slang

// Basic test for `GetAttributeAtVertex` function

//TEST:SIMPLE(filecheck=CHECK-SPIRV):-target spirv -entry main -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric
//TEST:SIMPLE(filecheck=CHECK-GLSL):-target glsl -entry main -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric

// CHECK-SPIRV: OpCapability FragmentBarycentricKHR
// CHECK-SPIRV: OpExtension "SPV_KHR_fragment_shader_barycentric"

// CHECK-SPIRV: OpEntryPoint Fragment %main "main"

// CHECK-SPIRV-DAG: OpDecorate %{{.*}} BuiltIn BaryCoordKHR

// CHECK-SPIRV-DAG: OpDecorate %{{.*}} BuiltIn BaryCoordNoPerspKHR

// CHECK-SPIRV: %{{.*}} = OpAccessChain %_ptr_Input_{{.*}} %{{.*}} %uint_0
// CHECK-SPIRV: %{{.*}} = OpAccessChain %_ptr_Input_{{.*}} %{{.*}} %uint_1
// CHECK-SPIRV: %{{.*}} = OpAccessChain %_ptr_Input_{{.*}} %{{.*}} %uint_2

// CHECK-GLSL: #extension GL_EXT_fragment_shader_barycentric : require
// CHECK-GLSL-DAG: gl_BaryCoordEXT
// CHECK-GLSL-DAG: gl_BaryCoordNoPerspEXT

[shader("fragment")]
void main(
    pervertex float4 color : COLOR,
    float3 bary : SV_Barycentrics,
    noperspective float3 baryNoPerspective : SV_Barycentrics,
    out float4 result : SV_Target)
{
    result = bary.x * GetAttributeAtVertex(color, 0)
           + bary.y * GetAttributeAtVertex(color, 1)
           + bary.z * GetAttributeAtVertex(color, 2)
           + baryNoPerspective.x * 0.1f;
}
