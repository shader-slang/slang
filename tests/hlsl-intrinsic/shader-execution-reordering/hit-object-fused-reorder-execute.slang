// #13774: the documented SER fused functions `TraceReorderExecute`, `ReorderExecute` and
// `TraceMotionReorderExecute` lower to one SPV_EXT_shader_invocation_reorder fused instruction on
// SPIR-V and to the GLSL `hitObject*ReorderExecuteEXT` builtins on GLSL EXT. Targets without a
// fused form use the equivalent separate calls:
//   TraceReorderExecute        TraceRay -> ReorderThread -> Invoke  (SPIR-V/GLSL NV, DXR, NVAPI, CUDA)
//   ReorderExecute             ReorderThread -> Invoke              (SPIR-V/GLSL NV, DXR, CUDA)
//   TraceMotionReorderExecute  TraceMotionRay -> ReorderThread -> Invoke  (SPIR-V/GLSL NV, CUDA)
// `ReorderExecute` on NVAPI and `TraceMotionReorderExecute` on D3D12 are diagnosed errors; see
// hit-object-fused-reorder-execute-diagnostics.slang.
//
// Each overload has its own entry point, so each module has exactly one RayPayloadKHR variable;
// the `%[[P]]` captures below rely on that to check that the fused instruction and the copy-back
// load use the same variable.

// SPIR-V, default profile and explicit EXT capability: one fused instruction, no sequence.
//TEST:SIMPLE(filecheck=TRACE): -target spirv -entry traceReorderExecute -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=TRACE): -target spirv -entry traceReorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderEXT
//TEST:SIMPLE(filecheck=FUSED): -target spirv -entry traceReorderExecute -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=TRACEHINT): -target spirv -entry traceReorderExecuteHint -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=FUSED): -target spirv -entry traceReorderExecuteHint -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=REORDER): -target spirv -entry reorderExecute -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=FUSED): -target spirv -entry reorderExecute -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=REORDERHINT): -target spirv -entry reorderExecuteHint -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=FUSED): -target spirv -entry reorderExecuteHint -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=MOTION): -target spirv -entry traceMotionReorderExecute -stage raygeneration -emit-spirv-directly
//TEST:SIMPLE(filecheck=FUSED): -target spirv -entry traceMotionReorderExecute -stage raygeneration -emit-spirv-directly

// SPIR-V NV has no fused instruction. `spvShaderInvocationReorderNV` implies the EXT capability, so
// these runs check that the more specific NV sequence case wins over the EXT case.
//TEST:SIMPLE(filecheck=NVTRACE0): -target spirv -entry traceReorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=NVREORDER0): -target spirv -entry reorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=NVTRACE): -target spirv -entry traceReorderExecuteHint -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=NOEXT): -target spirv -entry traceReorderExecuteHint -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=NVREORDER): -target spirv -entry reorderExecuteHint -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=NOEXT): -target spirv -entry reorderExecuteHint -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=NVMOTION): -target spirv -entry traceMotionReorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderMotionNV
//TEST:SIMPLE(filecheck=NOEXT): -target spirv -entry traceMotionReorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderMotionNV
//TEST:SIMPLE(filecheck=NVMOTION): -target spirv -entry traceMotionReorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=NOEXT): -target spirv -entry traceMotionReorderExecute -stage raygeneration -emit-spirv-directly -capability spvShaderInvocationReorderNV
//TEST:SIMPLE(filecheck=GLSLNVMOTION): -target glsl -entry traceMotionReorderExecute -stage raygeneration -capability GL_NV_shader_invocation_reorder -capability GL_NV_ray_tracing_motion_blur

// GLSL EXT uses the fused builtins; GLSL NV uses the sequence.
//TEST:SIMPLE(filecheck=GLSLTRACE): -target glsl -entry traceReorderExecute -stage raygeneration
//TEST:SIMPLE(filecheck=GLSLTRACEHINT): -target glsl -entry traceReorderExecuteHint -stage raygeneration
//TEST:SIMPLE(filecheck=GLSLREORDER): -target glsl -entry reorderExecute -stage raygeneration
//TEST:SIMPLE(filecheck=GLSLREORDERHINT): -target glsl -entry reorderExecuteHint -stage raygeneration
//TEST:SIMPLE(filecheck=GLSLMOTION): -target glsl -entry traceMotionReorderExecute -stage raygeneration -capability GL_EXT_shader_invocation_reorder_motion
//TEST:SIMPLE(filecheck=GLSLNV): -target glsl -entry traceReorderExecuteHint -stage raygeneration -capability GL_NV_shader_invocation_reorder
//TEST:SIMPLE(filecheck=GLSLNVREORDER): -target glsl -entry reorderExecuteHint -stage raygeneration -capability GL_NV_shader_invocation_reorder
//TEST:SIMPLE(filecheck=GLSLNVTRACE0): -target glsl -entry traceReorderExecute -stage raygeneration -capability GL_NV_shader_invocation_reorder
//TEST:SIMPLE(filecheck=GLSLNVREORDER0): -target glsl -entry reorderExecute -stage raygeneration -capability GL_NV_shader_invocation_reorder
//TEST:SIMPLE(filecheck=GLSLEXTONLY): -target glsl -entry traceReorderExecute -stage raygeneration
//TEST:SIMPLE(filecheck=GLSLEXTONLY): -target glsl -entry traceReorderExecuteHint -stage raygeneration
//TEST:SIMPLE(filecheck=GLSLEXTONLY): -target glsl -entry reorderExecute -stage raygeneration
//TEST:SIMPLE(filecheck=GLSLEXTONLY): -target glsl -entry reorderExecuteHint -stage raygeneration
//TEST:SIMPLE(filecheck=GLSLEXTONLY): -target glsl -entry traceMotionReorderExecute -stage raygeneration -capability GL_EXT_shader_invocation_reorder_motion
//TEST:SIMPLE(filecheck=GLSLNVONLY): -target glsl -entry traceReorderExecute -stage raygeneration -capability GL_NV_shader_invocation_reorder
//TEST:SIMPLE(filecheck=GLSLNVONLY): -target glsl -entry traceReorderExecuteHint -stage raygeneration -capability GL_NV_shader_invocation_reorder
//TEST:SIMPLE(filecheck=GLSLNVONLY): -target glsl -entry reorderExecute -stage raygeneration -capability GL_NV_shader_invocation_reorder
//TEST:SIMPLE(filecheck=GLSLNVONLY): -target glsl -entry reorderExecuteHint -stage raygeneration -capability GL_NV_shader_invocation_reorder
//TEST:SIMPLE(filecheck=GLSLNVONLY): -target glsl -entry traceMotionReorderExecute -stage raygeneration -capability GL_NV_shader_invocation_reorder -capability GL_NV_ray_tracing_motion_blur

// D3D12 (DXR 1.3 and NVAPI) and CUDA use the sequence.
//TEST:SIMPLE(filecheck=DXRTRACE0): -target hlsl -entry traceReorderExecute -stage raygeneration -profile sm_6_9 -capability ser_dxr
//TEST:SIMPLE(filecheck=DXRREORDER0): -target hlsl -entry reorderExecute -stage raygeneration -profile sm_6_9 -capability ser_dxr
//TEST:SIMPLE(filecheck=CUDATRACE0): -target cuda -entry traceReorderExecute -stage raygeneration
//TEST:SIMPLE(filecheck=CUDAREORDER0): -target cuda -entry reorderExecute -stage raygeneration
//TEST:SIMPLE(filecheck=DXRTRACE): -target hlsl -entry traceReorderExecuteHint -stage raygeneration -profile sm_6_9 -capability ser_dxr
//TEST:SIMPLE(filecheck=DXRREORDER): -target hlsl -entry reorderExecuteHint -stage raygeneration -profile sm_6_9 -capability ser_dxr
//TEST:SIMPLE(filecheck=NVAPI): -target hlsl -entry traceReorderExecuteHint -stage raygeneration -profile sm_6_5 -capability ser_nvapi
//TEST:SIMPLE(filecheck=NVAPI0): -target hlsl -entry traceReorderExecute -stage raygeneration -profile sm_6_5 -capability ser_nvapi
//TEST:SIMPLE(filecheck=CUDATRACE): -target cuda -entry traceReorderExecuteHint -stage raygeneration
//TEST:SIMPLE(filecheck=CUDAREORDER): -target cuda -entry reorderExecuteHint -stage raygeneration
//TEST:SIMPLE(filecheck=CUDAMOTION): -target cuda -entry traceMotionReorderExecute -stage raygeneration
//TEST:SIMPLE(filecheck=PTXTRACE): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry traceReorderExecuteHint -stage raygeneration
//TEST:SIMPLE(filecheck=PTXREORDER): -target ptx -Xnvrtc -I"./external/optix-dev/include/" -entry reorderExecuteHint -stage raygeneration

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

// GLSL EXT output calls only the fused builtins, and GLSL NV output never calls them. These
// prefixes have only `-NOT` lines, so each check covers the whole output.
// GLSLEXTONLY-NOT: reorderThreadEXT
// GLSLEXTONLY-NOT: hitObjectExecuteShaderEXT
// GLSLEXTONLY-NOT: hitObjectTraceRayEXT
// GLSLNVONLY-NOT: ReorderExecuteEXT

// No instruction of the separate sequence survives in a fused SPIR-V module.
// FUSED: OpEntryPoint RayGenerationKHR
// FUSED-NOT: OpHitObjectTraceRayEXT
// FUSED-NOT: OpHitObjectTraceRayMotionEXT
// FUSED-NOT: OpReorderThreadWithHitObjectEXT
// FUSED-NOT: OpHitObjectExecuteShaderEXT

// No EXT instruction or capability appears on an NV target.
// NOEXT-NOT: OpCapability ShaderInvocationReorderEXT
// NOEXT: OpEntryPoint RayGenerationKHR
// NOEXT-NOT: OpHitObject{{.*}}ReorderExecute{{.*}}EXT

// The payload operand of each fused instruction is a RayPayloadKHR variable, and the result the
// invoked shader writes there is read back for the caller's `payload`.
// TRACE: %[[P:[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} RayPayloadKHR
// TRACEHINT: %[[P:[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} RayPayloadKHR
// REORDER: %[[P:[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} RayPayloadKHR
// REORDERHINT: %[[P:[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} RayPayloadKHR
// MOTION: %[[P:[A-Za-z0-9_]+]] = OpVariable %{{[A-Za-z0-9_]+}} RayPayloadKHR

[shader("raygeneration")]
void traceReorderExecute()
{
    MyPayload payload = { float4(1) };

    // TRACE: OpHitObjectTraceReorderExecuteEXT %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %[[P]]{{$}}
    // TRACE: OpLoad %{{[A-Za-z0-9_]+}} %[[P]]
    // GLSLTRACE: hitObjectTraceReorderExecuteEXT({{.*}}, {{[0-9]+}});
    // NVTRACE0: OpHitObjectTraceRayNV
    // NVTRACE0: OpReorderThreadWithHitObjectNV %{{[A-Za-z0-9_]+}}{{$}}
    // NVTRACE0: OpHitObjectExecuteShaderNV
    // DXRTRACE0: dx::HitObject::TraceRay(
    // DXRTRACE0: dx::MaybeReorderThread({{[^,]*}});
    // DXRTRACE0: dx::HitObject::Invoke({{[^,]*}}, {{[^,]*}});
    // CUDATRACE0: __raygen__traceReorderExecute
    // CUDATRACE0: optixTraverse(
    // CUDATRACE0: optixReorder();
    // CUDATRACE0: optixInvoke(
    // GLSLNVTRACE0: hitObjectTraceRayNV(
    // GLSLNVTRACE0: reorderThreadNV({{[^,]*}});
    // GLSLNVTRACE0: hitObjectExecuteShaderNV(
    // NVAPI0: NvTraceRayHitObject(
    // NVAPI0: NvReorderThread({{[^,]*}});
    // NVAPI0: NvInvokeHitObject({{[^,]*}}, {{[^,]*}}, {{[^,]*}});
    TraceReorderExecute(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, makeRay(), payload);

    outputBuffer[DispatchRaysIndex().x] = payload.color;
}

[shader("raygeneration")]
void traceReorderExecuteHint()
{
    MyPayload payload = { float4(1) };

    // The coherence hint and bit count follow the payload.
    // TRACEHINT: OpHitObjectTraceReorderExecuteEXT %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %[[P]] %uint_5 %uint_3{{$}}
    // TRACEHINT: OpLoad %{{[A-Za-z0-9_]+}} %[[P]]
    // NVTRACE: OpHitObjectTraceRayNV
    // NVTRACE: OpReorderThreadWithHitObjectNV %{{[A-Za-z0-9_]+}} %uint_5 %uint_3
    // NVTRACE: OpHitObjectExecuteShaderNV
    // GLSLTRACEHINT: hitObjectTraceReorderExecuteEXT({{.*}}, 5U, 3U, {{[0-9]+}});
    // GLSLNV: hitObjectTraceRayNV(
    // GLSLNV: reorderThreadNV({{.*}}, 5U, 3U);
    // GLSLNV: hitObjectExecuteShaderNV(
    // DXRTRACE: dx::HitObject::TraceRay(
    // DXRTRACE: dx::MaybeReorderThread({{.*}}, 5U, 3U);
    // DXRTRACE: dx::HitObject::Invoke({{[^,]*}}, {{[^,]*}});
    // NVAPI: NvTraceRayHitObject(
    // NVAPI: NvReorderThread({{.*}}, 5U, 3U);
    // NVAPI: NvInvokeHitObject({{[^,]*}}, {{[^,]*}}, {{[^,]*}});
    // CUDATRACE: __raygen__traceReorderExecuteHint
    // CUDATRACE: optixTraverse(
    // CUDATRACE: optixReorder({{.*}}5U{{.*}}3U{{.*}});
    // CUDATRACE: optixInvoke(
    // PTXTRACE: _optix_hitobject_traverse
    // PTXTRACE: _optix_hitobject_reorder
    // PTXTRACE: _optix_hitobject_invoke
    TraceReorderExecute(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, makeRay(), 5, 3, payload);

    outputBuffer[DispatchRaysIndex().x] = payload.color;
}

[shader("raygeneration")]
void reorderExecute()
{
    MyPayload payload = { float4(1) };
    HitObject hit = HitObject::MakeNop();

    // REORDER: OpHitObjectReorderExecuteShaderEXT %{{[A-Za-z0-9_]+}} %[[P]]{{$}}
    // REORDER: OpLoad %{{[A-Za-z0-9_]+}} %[[P]]
    // GLSLREORDER: hitObjectReorderExecuteEXT({{[^,]*}}, {{[0-9]+}});
    // NVREORDER0: OpReorderThreadWithHitObjectNV %{{[A-Za-z0-9_]+}}{{$}}
    // NVREORDER0: OpHitObjectExecuteShaderNV
    // DXRREORDER0: dx::MaybeReorderThread({{[^,]*}});
    // DXRREORDER0: dx::HitObject::Invoke({{[^,]*}}, {{[^,]*}});
    // CUDAREORDER0: __raygen__reorderExecute
    // CUDAREORDER0: optixReorder();
    // CUDAREORDER0: optixInvoke(
    // GLSLNVREORDER0: reorderThreadNV({{[^,]*}});
    // GLSLNVREORDER0: hitObjectExecuteShaderNV(
    ReorderExecute(hit, payload);

    outputBuffer[DispatchRaysIndex().x] = payload.color;
}

[shader("raygeneration")]
void reorderExecuteHint()
{
    MyPayload payload = { float4(1) };
    HitObject hit = HitObject::MakeNop();

    // REORDERHINT: OpHitObjectReorderExecuteShaderEXT %{{[A-Za-z0-9_]+}} %[[P]] %uint_7 %uint_2{{$}}
    // REORDERHINT: OpLoad %{{[A-Za-z0-9_]+}} %[[P]]
    // NVREORDER: OpReorderThreadWithHitObjectNV %{{[A-Za-z0-9_]+}} %uint_7 %uint_2
    // NVREORDER: OpHitObjectExecuteShaderNV
    // GLSLREORDERHINT: hitObjectReorderExecuteEXT({{.*}}, 7U, 2U, {{[0-9]+}});
    // GLSLNVREORDER: reorderThreadNV({{.*}}, 7U, 2U);
    // GLSLNVREORDER: hitObjectExecuteShaderNV(
    // DXRREORDER: dx::MaybeReorderThread({{.*}}, 7U, 2U);
    // DXRREORDER: dx::HitObject::Invoke({{[^,]*}}, {{[^,]*}});
    // CUDAREORDER: __raygen__reorderExecuteHint
    // CUDAREORDER: optixReorder({{.*}}7U{{.*}}2U{{.*}});
    // CUDAREORDER: optixInvoke(
    // PTXREORDER: _optix_hitobject_reorder
    // PTXREORDER: _optix_hitobject_invoke
    ReorderExecute(hit, 7, 2, payload);

    outputBuffer[DispatchRaysIndex().x] = payload.color;
}

[shader("raygeneration")]
void traceMotionReorderExecute()
{
    MyPayload payload = { float4(1) };

    // MOTION: OpHitObjectTraceMotionReorderExecuteEXT %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %{{[A-Za-z0-9_]+}} %float_0_5 %[[P]] %uint_4 %uint_2{{$}}
    // MOTION: OpLoad %{{[A-Za-z0-9_]+}} %[[P]]
    // NVMOTION: OpHitObjectTraceRayMotionNV
    // NVMOTION: OpReorderThreadWithHitObjectNV %{{[A-Za-z0-9_]+}} %uint_4 %uint_2
    // NVMOTION: OpHitObjectExecuteShaderNV
    // GLSLNVMOTION: hitObjectTraceRayMotionNV(
    // GLSLNVMOTION: reorderThreadNV({{.*}}, 4U, 2U);
    // GLSLNVMOTION: hitObjectExecuteShaderNV(
    // GLSLMOTION: hitObjectTraceMotionReorderExecuteEXT({{.*}}, 0.5, 4U, 2U, {{[0-9]+}});
    // CUDAMOTION: __raygen__traceMotionReorderExecute
    // CUDAMOTION: optixTraverse(
    // CUDAMOTION: optixReorder({{.*}}4U{{.*}}2U{{.*}});
    // CUDAMOTION: optixInvoke(
    TraceMotionReorderExecute(scene, RAY_FLAG_NONE, 0xFF, 0, 1, 0, makeRay(), 0.5, 4, 2, payload);

    outputBuffer[DispatchRaysIndex().x] = payload.color;
}
