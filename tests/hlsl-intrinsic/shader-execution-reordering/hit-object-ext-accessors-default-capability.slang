// The HitObject accessors whose SPIR-V EXT branch was keyed on spvShaderInvocationReorderEXT
// instead of on `spirv` (SetShaderTableIndex, GetRayFlags, GetRayTMin, GetWorldRayOrigin,
// GetWorldRayDirection and GetTriangleVertexPositions) must compile for SPIR-V without an explicit
// SER capability. Without one, HitObject::TraceRay and the other accessors already select the EXT
// path; these used to fail with E41011 ("__target_switch has no compatible target") unless
// -capability spvShaderInvocationReorderEXT was passed.
//
// Under spvShaderInvocationReorderNV, the accessors with no NV opcode must still be rejected
// (an NV build emits OpTypeHitObjectNV, which the EXT opcodes cannot take): their NV case must
// keep winning over `case spirv:` (NV run; GetTriangleVertexPositions is covered by
// hit-object-get-triangle-vertex-positions.slang).

//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=CHECK): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderEXT
//TEST:SIMPLE(filecheck=POS): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvRayTracingPositionFetchKHR -DPOSITION_FETCH
//TEST:SIMPLE(filecheck=NONV): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=NONV): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderEXT
//TEST:SIMPLE(filecheck=NONV): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvRayTracingPositionFetchKHR -DPOSITION_FETCH
//DIAGNOSTIC_TEST:SIMPLE(filecheck=NV): -target spirv-asm -entry rayGenerationMain -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV

// CHECK-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
// CHECK-DAG: OpCapability ShaderInvocationReorderEXT
// CHECK-DAG: OpHitObjectSetShaderBindingTableRecordIndexEXT
// CHECK-DAG: OpHitObjectGetRayFlagsEXT
// CHECK-DAG: OpHitObjectGetRayTMinEXT
// CHECK-DAG: OpHitObjectGetWorldRayOriginEXT
// CHECK-DAG: OpHitObjectGetWorldRayDirectionEXT

// POS-DAG: OpExtension "SPV_EXT_shader_invocation_reorder"
// POS-DAG: OpCapability ShaderInvocationReorderEXT
// POS-DAG: OpCapability RayTracingPositionFetchKHR
// POS-DAG: OpHitObjectGetIntersectionTriangleVertexPositionsEXT

// No run uses the NV extension anywhere in the module.
// NONV-NOT: ShaderInvocationReorderNV

// NV-DAG: SetShaderTableIndex() has no SPIRV NV equivalent
// NV-DAG: GetRayFlags() has no SPIRV NV equivalent

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float4> outputBuffer;

struct MyPayload
{
    float4 color;
};

[shader("raygeneration")]
void rayGenerationMain()
{
    RayDesc ray;
    ray.Origin = float3(0, 0, 0);
    ray.Direction = float3(0, 0, 1);
    ray.TMin = 0.01;
    ray.TMax = 1e4;

    MyPayload payload = { float4(0) };
    HitObject hit = HitObject::TraceRay(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, ray, payload);

    hit.SetShaderTableIndex(1);
    float4 result = float4(hit.GetWorldRayOrigin() + hit.GetWorldRayDirection(), hit.GetRayTMin());
    result.w += float(hit.GetRayFlags());
#ifdef POSITION_FETCH
    float3 positions[3] = hit.GetTriangleVertexPositions();
    result.xyz += positions[0];
#endif
    outputBuffer[0] = result;
}
