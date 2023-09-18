// entry-point-params.slang

// Test that entry-point `uniform` parameters on ray-tracing
// shaders properly map to the "shader record" in SPIR-V output.

//TEST:SIMPLE(filecheck=CHECK): -profile glsl_460+spirv_1_4 -stage raygeneration -entry main -target spirv-assembly

RWStructuredBuffer<float> buffer;

void main(
	uniform float value) 
{
	buffer[DispatchRaysIndex().x] = value;
}

// CHECK: OpCapability RayTracingKHR
// CHECK: OpExtension "SPV_KHR_ray_tracing"
// CHECK: OpEntryPoint RayGenerationNV %main
// CHECK: OpDecorate %gl_LaunchIDEXT BuiltIn LaunchIdNV
// CHECK: %gl_LaunchIDEXT = OpVariable %_ptr_Input_v3uint Input
// CHECK: %{{.*}} = OpLoad %v3uint %gl_LaunchIDEXT
// CHECK: %{{.*}} = OpAccessChain %_ptr_ShaderRecordBufferNV_float %_S2 %int_0
