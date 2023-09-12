// callable.slang

//TEST:SIMPLE(filecheck=CHECK): -profile glsl_460+spirv_1_4 -stage callable -entry main -target spirv-assembly

import callable_shared;

Texture2D gAlbedoMap;
SamplerState gSampler;

void main(in out MaterialPayload ioPayload)
{
	ioPayload.albedo = gAlbedoMap.SampleLevel(
		gSampler,
		ioPayload.uv,
		0);
}

// CHECK: OpCapability RayTracingKHR
// CHECK: OpExtension "SPV_KHR_ray_tracing"
// CHECK: OpEntryPoint CallableNV %main
// CHECK: %_ptr_IncomingCallableDataNV_MaterialPayload_0 = OpTypePointer IncomingCallableDataNV %MaterialPayload_0
// CHECK: %_ptr_IncomingCallableDataNV_v2float = OpTypePointer IncomingCallableDataNV %v2float
// CHECK: %_ptr_IncomingCallableDataNV_v4float = OpTypePointer IncomingCallableDataNV %v4float
// CHECK: %{{.*}} = OpAccessChain %_ptr_IncomingCallableDataNV_v2float %_S1 %int_1
// CHECK: %{{.*}} = OpAccessChain %_ptr_IncomingCallableDataNV_v4float %_S1 %int_0
