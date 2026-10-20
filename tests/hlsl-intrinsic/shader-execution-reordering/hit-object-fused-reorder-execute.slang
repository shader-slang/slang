// #13774: the documented SER fused functions `TraceReorderExecute`, `ReorderExecute` and
// `TraceMotionReorderExecute` lower to one SPV_EXT_shader_invocation_reorder fused instruction on
// SPIR-V and to the GLSL `hitObject*ReorderExecuteEXT` builtins on GLSL. Targets without a fused
// form (SPIR-V/GLSL NV, DXR 1.3, NVAPI, CUDA) get the equivalent TraceRay -> ReorderThread ->
// Invoke sequence.

//TEST:SIMPLE(filecheck=SPV): -target spirv -entry traceReorderExecute -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPV): -target spirv -entry traceReorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderEXT
//TEST:SIMPLE(filecheck=SPVSEQ): -target spirv -entry traceReorderExecute -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=REORDER): -target spirv -entry reorderExecute -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVSEQ): -target spirv -entry reorderExecute -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=MOTION): -target spirv -entry traceMotionReorderExecute -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=SPVSEQ): -target spirv -entry traceMotionReorderExecute -stage raygeneration -emit-spirv-directly

// SPIR-V NV has no fused instruction, and `spvShaderInvocationReorderNV` implies the EXT capability,
// so the NV sequence must be selected before the EXT case.
//TEST:SIMPLE(filecheck=NV): -target spirv -entry traceReorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=NOEXT): -target spirv -entry traceReorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=NVREORDER): -target spirv -entry reorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=NOEXT): -target spirv -entry reorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=NVMOTION): -target spirv -entry traceMotionReorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderMotionNV
//TEST:SIMPLE(filecheck=NOEXT): -target spirv -entry traceMotionReorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderMotionNV

//TEST:SIMPLE(filecheck=GLSL): -target glsl -entry traceReorderExecute -stage raygeneration
//TEST:SIMPLE(filecheck=GLSLREORDER): -target glsl -entry reorderExecute -stage raygeneration
//TEST:SIMPLE(filecheck=GLSLMOTION): -target glsl -entry traceMotionReorderExecute -stage raygeneration -capability GL_EXT_shader_invocation_reorder_motion
//TEST:SIMPLE(filecheck=GLSLNV): -target glsl -entry traceReorderExecute -stage raygeneration -capability GL_NV_shader_invocation_reorder

//TEST:SIMPLE(filecheck=DXR): -target hlsl -entry traceReorderExecute -stage raygeneration -profile sm_6_9 -capability ser_dxr
//TEST:SIMPLE(filecheck=DXRREORDER): -target hlsl -entry reorderExecute -stage raygeneration -profile sm_6_9 -capability ser_dxr
//TEST:SIMPLE(filecheck=NVAPI): -target hlsl -entry traceReorderExecute -stage raygeneration -profile sm_6_5 -capability ser_nvapi
//TEST:SIMPLE(filecheck=CUDA): -target cuda -entry traceReorderExecute -stage raygeneration
//TEST:SIMPLE(filecheck=CUDAREORDER): -target cuda -entry reorderExecute -stage raygeneration

//TEST_INPUT: set scene = AccelerationStructure
uniform RaytracingAccelerationStructure scene;

//TEST_INPUT:set outputBuffer = out ubuffer(data=[0 0 0 0], stride=4)
RWStructuredBuffer<float4> outputBuffer;

struct MyPayload { float4 color; };

RayDesc makeRay()
{
    RayDesc ray;
    ray.Origin = float3(DispatchRaysIndex().x, 0, 0);
    ray.TMin = 0.01f;
    ray.Direction = float3(0, 1, 0);
    ray.TMax = 1e4f;
    return ray;
}

// A sequence spelling must not survive anywhere in a fused SPIR-V module.
// SPVSEQ-NOT: OpHitObjectTraceRayEXT
// SPVSEQ-NOT: OpHitObjectTraceRayMotionEXT
// SPVSEQ-NOT: OpReorderThreadWithHitObjectEXT
// SPVSEQ-NOT: OpHitObjectExecuteShaderEXT

// No EXT instruction or capability may appear on an NV target.
// NOEXT-NOT: ReorderExecute
// NOEXT-NOT: ShaderInvocationReorderEXT

// The payload operand of every fused instruction is a RayPayloadKHR variable.
// SPV: %[[PAYLOAD:[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} RayPayloadKHR
// REORDER: %[[RPAYLOAD:[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} RayPayloadKHR
// MOTION: %[[MPAYLOAD:[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} RayPayloadKHR

[shader("raygeneration")]
void traceReorderExecute()
{
    MyPayload payload = { float4(1) };

    // SPV: OpHitObjectTraceReorderExecuteEXT %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %[[PAYLOAD]]{{$}}
    // NV: OpHitObjectTraceRayNV
    // NV: OpReorderThreadWithHitObjectNV
    // NV: OpHitObjectExecuteShaderNV
    // GLSL: hitObjectTraceReorderExecuteEXT(
    // GLSLNV: hitObjectTraceRayNV(
    // GLSLNV: reorderThreadNV(
    // GLSLNV: hitObjectExecuteShaderNV(
    // DXR: dx::HitObject::TraceRay(
    // DXR: dx::MaybeReorderThread(
    // DXR: dx::HitObject::Invoke(
    // NVAPI: NvTraceRayHitObject(
    // NVAPI: NvReorderThread(
    // NVAPI: NvInvokeHitObject(
    // CUDA: __raygen__traceReorderExecute
    // CUDA: optixTraverse(
    // CUDA: optixReorder(
    // CUDA: optixInvoke(
    TraceReorderExecute(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, makeRay(), payload);

    // The coherence hint and bit count follow the payload.
    // SPV: OpHitObjectTraceReorderExecuteEXT %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %[[PAYLOAD]] %uint_5 %uint_3{{$}}
    // NV: OpHitObjectTraceRayNV
    // NV: OpReorderThreadWithHitObjectNV %{{[A-Za-z0-9_]+}} %uint_5 %uint_3
    // NV: OpHitObjectExecuteShaderNV
    // GLSL: hitObjectTraceReorderExecuteEXT({{.*}}, 5U, 3U, {{[0-9]+}});
    // GLSLNV: reorderThreadNV({{.*}}, 5U, 3U);
    // DXR: dx::MaybeReorderThread({{.*}}, 5U, 3U);
    // NVAPI: NvReorderThread({{.*}}, 5U, 3U);
    // CUDA: optixReorder({{.*}}5U{{.*}}3U{{.*}});
    TraceReorderExecute(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, makeRay(), 5, 3, payload);

    // The payload written by the invoked shader is copied back to the caller's variable.
    // SPV: OpLoad %{{[A-Za-z0-9_]+}} %[[PAYLOAD]]
    outputBuffer[DispatchRaysIndex().x] = payload.color;
}

[shader("raygeneration")]
void reorderExecute()
{
    MyPayload payload = { float4(1) };
    HitObject hit = HitObject::MakeMiss(RAY_FLAG_NONE, 0, makeRay());

    // REORDER: OpHitObjectReorderExecuteShaderEXT %{{[A-Za-z0-9_]+}} %[[RPAYLOAD]]{{$}}
    // NVREORDER: OpReorderThreadWithHitObjectNV %{{[A-Za-z0-9_]+}}{{$}}
    // NVREORDER: OpHitObjectExecuteShaderNV
    // GLSLREORDER: hitObjectReorderExecuteEXT(
    // DXRREORDER: dx::MaybeReorderThread(
    // DXRREORDER: dx::HitObject::Invoke(
    // CUDAREORDER: __raygen__reorderExecute
    // CUDAREORDER: optixReorder(
    // CUDAREORDER: optixInvoke(
    ReorderExecute(hit, payload);

    // REORDER: OpHitObjectReorderExecuteShaderEXT %{{[A-Za-z0-9_]+}} %[[RPAYLOAD]] %uint_7 %uint_2{{$}}
    // NVREORDER: OpReorderThreadWithHitObjectNV %{{[A-Za-z0-9_]+}} %uint_7 %uint_2
    // NVREORDER: OpHitObjectExecuteShaderNV
    // GLSLREORDER: hitObjectReorderExecuteEXT({{.*}}, 7U, 2U, {{[0-9]+}});
    // DXRREORDER: dx::MaybeReorderThread({{.*}}, 7U, 2U);
    // DXRREORDER: dx::HitObject::Invoke(
    // CUDAREORDER: optixReorder({{.*}}7U{{.*}}2U{{.*}});
    // CUDAREORDER: optixInvoke(
    ReorderExecute(hit, 7, 2, payload);

    outputBuffer[DispatchRaysIndex().x] = payload.color;
}

[shader("raygeneration")]
void traceMotionReorderExecute()
{
    MyPayload payload = { float4(1) };

    // MOTION: OpCapability RayTracingMotionBlurNV
    // MOTION: OpHitObjectTraceMotionReorderExecuteEXT %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %float_0_5 %[[MPAYLOAD]] %uint_4 %uint_2{{$}}
    // NVMOTION: OpHitObjectTraceRayMotionNV
    // NVMOTION: OpReorderThreadWithHitObjectNV %{{[A-Za-z0-9_]+}} %uint_4 %uint_2
    // NVMOTION: OpHitObjectExecuteShaderNV
    // GLSLMOTION: hitObjectTraceMotionReorderExecuteEXT({{.*}}, 0.5, 4U, 2U, {{[0-9]+}});
    TraceMotionReorderExecute(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, makeRay(), 0.5, 4, 2, payload);

    outputBuffer[DispatchRaysIndex().x] = payload.color;
}
