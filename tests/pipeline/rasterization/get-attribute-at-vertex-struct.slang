// get-attribute-at-vertex-struct.slang

// Test GetAttributeAtVertex and SV_Barycentrics on struct members

//TEST:SIMPLE(filecheck=CHECK-SPIRV):-target spirv -entry main -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric
//TEST:SIMPLE(filecheck=CHECK-GLSL):-target glsl -entry main -stage fragment -profile glsl_450+GL_EXT_fragment_shader_barycentric

// CHECK-SPIRV: OpCapability FragmentBarycentricKHR
// CHECK-SPIRV: OpExtension "SPV_KHR_fragment_shader_barycentric"

// CHECK-SPIRV: OpEntryPoint Fragment %main "main"
// CHECK-SPIRV: OpDecorate %{{.*}} BuiltIn BaryCoordKHR
// CHECK-SPIRV: OpDecorate %{{.*}} BuiltIn BaryCoordNoPerspKHR
// CHECK-SPIRV: %{{.*}} = OpAccessChain %_ptr_Input_{{.*}} %{{.*}} %uint_0
// CHECK-SPIRV: %{{.*}} = OpAccessChain %_ptr_Input_{{.*}} %{{.*}} %uint_1
// CHECK-SPIRV: %{{.*}} = OpAccessChain %_ptr_Input_{{.*}} %{{.*}} %uint_2

// CHECK-GLSL: #extension GL_EXT_fragment_shader_barycentric : require
// CHECK-GLSL-DAG: gl_BaryCoordEXT
// CHECK-GLSL-DAG: gl_BaryCoordNoPerspEXT

struct Input
{
    pervertex float4 color : COLOR;
    float3 bary : SV_Barycentrics;
    noperspective float3 baryNoPerspective : SV_Barycentrics;
}

[shader("fragment")]
void main(
    Input input,
    out float4 result : SV_Target)
{
    result = input.bary.x * GetAttributeAtVertex(input.color, 0)
        + input.bary.y * GetAttributeAtVertex(input.color, 1)
        + input.bary.z * GetAttributeAtVertex(input.color, 2)
        + input.baryNoPerspective.x * 0.1f;
}